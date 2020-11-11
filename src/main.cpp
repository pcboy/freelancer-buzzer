/*
 *          DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                  Version 2, December 2004
 *
 *  Copyright (C) 2004 Sam Hocevar
 *  14 rue de Plaisance, 75014 Paris, France
 *  Everyone is permitted to copy and distribute verbatim or modified
 *  copies of this license document, and changing it is allowed as long
 *  as the name is changed.
 *  DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 *
 *
 *  David Hagege <david.hagege@gmail.com>
 */
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

#define WIFI_TIMEOUT_MS 20000 // 20 second WiFi connection timeout
#define WIFI_RECOVER_TIME_MS 5000

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include "configuration.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

bool isLedON = false;

// setting PWM properties
const int freq = 10000;
const int ledChannel = 0;
const int resolution = 8;
const int ledPin = 19;
const int buttonPin = 12;

void ledON(void) {
  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    // changing the LED brightness with PWM
    ledcWrite(ledChannel, dutyCycle);
    delay(1);
  }
  isLedON = true;
}

void ledOFF(void) {
  for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    // changing the LED brightness with PWM
    ledcWrite(ledChannel, dutyCycle);
    delay(1);
  }
  isLedON = false;
}

void setLedInitialState() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(STATUS_URL);
    // http.setAuthorization(USER, PASSWORD);
    int code = http.GET();
    http.end();
    if (code == 200) {
      ledON();
    } else {
      ledOFF();
    }
  }
}

void ledError(int cycles = 10) {
  for (int count = 0; count < cycles; count++) {
    ledcWrite(ledChannel, 255);
    delay(250);
    ledcWrite(ledChannel, 0);
    delay(250);
  }
}

void offCall(void) {
  // Make HTTP request for the off call

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(OFF_TRIGGER_URL);
    // http.setAuthorization(USER, PASSWORD);
    int code = http.POST("");
    if (code != 200) {
      ledError();
      setLedInitialState();
    }

    http.end();
  }
}

void onCall(void) {
  // Make HTTP request for the on call

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(ON_TRIGGER_URL);
    // http.setAuthorization(USER, PASSWORD);
    int code = http.POST("");
    http.end();
    if (code != 200) {
      ledError();
      setLedInitialState();
    }
  }
}

void loopWifiKeepAlive(void *pvParameters) {
  // esp_task_wdt_add(NULL); // Attach task to watchdog
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  delay(2000);
  while (42) {
    // esp_task_wdt_reset(); // reset watchdog
    if (WiFi.status() == WL_CONNECTED) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      // delay(1000);
      continue;
    } else {
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WIFI] Connecting");

        ledError(1);
      }
      Serial.println("[WIFI] Connected: ");
      setLedInitialState();
    }
  }
}

void loopButton(void *pvParameters) {
  while (42) {
    int reading = digitalRead(buttonPin);

    if (reading == LOW) {
      Serial.println("Button pushed");

      Serial.println(reading);
      if (isLedON) {
        ledOFF();
        offCall();
      } else {
        ledON();
        onCall();
      }
    }
  }
}

void setup(void) {
  Serial.begin(9600);

  Serial.printf("Connecting to %s ", ssid);
  // setCpuFrequencyMhz(80);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  pinMode(buttonPin, INPUT_PULLUP);

  // OTASetup();

  /* Init watchdog timer */
  // esp_task_wdt_init(3, false);

  xTaskCreatePinnedToCore(loopWifiKeepAlive, "loopWifiKeepAlive", 4096, NULL, 3,
                          NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(loopButton, "loopButton", 4096, NULL, 2, NULL,
                          ARDUINO_RUNNING_CORE);
}

void loop(void) {}

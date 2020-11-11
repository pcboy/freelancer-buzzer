# Freelancer Buzzer

The Freelancer Buzzer is a very simple device, one button.  
When you push it, it lights up and makes a call to the url of your choice (ON_TRIGGER_URL in [include/configuration.h](include/configuration.h)), and when you push it again it calls another url of your choice (OFF_TRIGGER_URL in [include/configuration.h](include/configuration.h)) and the light is turned off.  
That means you can easily use it as a timer while you are working. I'm personally connecting it to https://github.com/pcboy/timetr4cker .

<a href="assets/button.jpg"><img src="assets/button.jpg" width="60%"/></a>

# BOM

ESP32 and an arcade button, that's it!

ESP32: https://www.aliexpress.com/item/4000340140634.html  

5 Colors LED Light 60MM Arcade Video Game Player Push Button Switch:  
https://www.aliexpress.com/item/32764952742.html


The button being directly alimented from one of the ESP32 pins you are not gonna get the maximum brightness but it's more than enough. Feel free to provide proper current otherwise.

# Configuration

Check [include/configuration.h](include/configuration.h) 

STATUS_URL is another endpoint I use to set the current button status on boot up. So you can unplug the button and plug it again and keep your status. If it returns 200, the status of the button will be marked as ON (light ON), otherwise OFF. Feel free to change the code if that doesn't suit you.

# Compilation

You need platformIO, then simply do platformio run -t upload.  
Check the platformio.ini, you may need to change the serial port depending on your config.

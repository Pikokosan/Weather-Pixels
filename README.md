# Weather-Pixels Tank light
This code is for a fish tank lighting system with live weather.
This is a modified version of the Adafruit Weather Lamp which used Yahoo Weather API before (Weather data and timestamp)
Yahoo weather API does not work anymore. This version works with Openweathermap instead.
info: https://openweathermap.org/current
Openweathermap does not parse the currenttime. (only the moment where a weather change occurred.) this version uses pool.ntp.org
Also changed the weather code id for each scenario to match Openweathermap codes: https://openweathermap.org/weather-conditions
Dont forget to set the configuration in definitions.h.

This system uses the Neopixel library and makes use of RGBW led's to give a better looking light.

# setup
In the Tankweathe.ino set APIkey to your openweathermap APIkey. Also put your zip code in the location. look over the definition.h as well. If you are not using RGBW led's comment out "#define ENABLE_RGBW" as normal RGB led's will not work properly. Take note that the color values will need to be changed to work with your setup. I highly recommend getting RGBW led's Amazon sells 144 led strips for $28 at the time of this project. 


# What does it do
Currently the project will display the current weather. it polls openweathermap and parses
the json information. It creates clouds based on the precentage returned by openweathermap.
it also moves the clouds across the strip using the wind speed that polled from openweathermap.
Sunrise and sunset are currently set to 6am and 6pm respectivly. This will change in a later version to be more accurate.

As of now if you go to the home page of the ESP8266 it just displays the last requested json information received from openweathermap.org.

# Things to do
  - [ ] Use parsed sunrise and sunset
  - [ ] Wifi manager custom arg "timezone"
  - [ ] Wifi manager custom arg "Daylight savings"
  - [ ] Wifi manager custom arg "zip"
  - [ ] Wifi manager custom arg "API id"
  - [ ] Wifi manager custom arg "Pixel pin"
  - [ ] Wifi manager custom arg "Number of leds"
  - [ ] custom webpage with above setting.
  - [ ] clean up code
  - [ ] store above args in eeprom.
  - [ ] eeprom setting to json for webpage.
  - [x] live cloud coverage.
  - [x] Live wind speed.
  - [x] Wifimanager.
  - [x] OTA update.

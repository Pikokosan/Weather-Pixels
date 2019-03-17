// 2019 Bas Emmen
// Release notes
// This is a modified version of the Adafruit Weather Lamp which used Yahoo Weather API before (Weather data and timestamp)
// Yahoo weather API does not work anymore. This version works with Openweathermap instead.
// info: https://openweathermap.org/current
// Openweathermap does not parse the currenttime. (only the moment where a weather change occurred.) this version uses pool.ntp.org
// Also changed the weather code id for each scenario to match Openweathermap codes: https://openweathermap.org/weather-conditions
// Dont forget to set Neopixel PIN and amount of Leds in animation.cpp

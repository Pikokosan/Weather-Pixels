#include "definitions.h"
#include "webpage.h"
//#include "Timingcode.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <time.h>
//#include <TimeLib.h>
#include <WiFiManager.h>


void animSetup(void);
void animConfig(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void waitForFrame(void);
void renderFrame(void);
void handleroot(void);
time_t getNtpTime();

// YOUR VARIABLES ARE BELOW
const char* APIkey = "YOUR API KEY"; // Your Openweather API key. Subscribe at https://openweathermap.org/appid


const char* location = "YOU ZIP CODE"; // Your city,countrycode , e.g. Amsterdam,NL
// YOUR VARIABLES ARE ABOVE. Dont forget edit the animation.cpp file

const char* path_prefix = "http://api.openweathermap.org/data/2.5/weather?zip=";
const char* path_postfix1 = "&APPID=";
const char* path_postfix2 = "&mode=json&units=imperial&cnt=0";
const char* host = "api.openweathermap.org";
const int httpPort = 80;

bool testtime = false;
uint16_t timeofday = 0;




unsigned long lastConnectionTime = 10 * 60 * 1000;     // last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 10* 1000;//10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)

int16_t createhour, createmin;

String result;
const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 270;
DynamicJsonDocument doc(capacity);


int weathercode = 0; //Auto set to clear

int wind_speed = (int)doc["wind"]["speed"]; // 8.05

int clouds_all = doc["clouds"]["all"]; // 1

long dt = doc["dt"]; // 1554952404

JsonObject sys = doc["sys"];
long sys_sunrise = sys["sunrise"]; // 1554898169
long sys_sunset = sys["sunset"]; // 1554944080

//Web page setup.
ESP8266WebServer server(80);



void setup() {
  Serial.begin(9600);
  delay(10);

  // We start by connecting to a WiFi network

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();



  configTime(TIMEZONE * 3600, DST * 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  delay(1000); //Needs a short delay to allow time to sync.
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }

  //Setup up web pages

  server.on("/", handleroot);

  server.begin();
  Serial.println("HTTP server started");

  animConfig(0, 0, 0, 0, 0, 0);
  animSetup();
}



void loop() {
  //OWM requires 10mins between request intervals
  //check if 10mins has passed then conect again and pull
  if (millis() - lastConnectionTime > postInterval){
    lastConnectionTime = millis();
    if(updateWeatherJson()){
      updateWeather();
    }

  }
  ArduinoOTA.handle();
  waitForFrame();
  renderFrame();
  server.handleClient();

}


bool updateWeatherJson() {

  Serial.print("Connecting to "); Serial.println(host);

  // Use WiFiClient class to create TCP connections
  HTTPClient client;




  // We now create a URI for the request

  String url = String(path_prefix) + String(location) + String(path_postfix1)+ String(APIkey) + String(path_postfix2);
  client.begin(url);
  Serial.print("Requesting URL: ");  Serial.println(url);
  int httpCode = client.GET();
  if(httpCode > 0) {
    //result = ""
    result = client.getString();
  }
  client.end();


  Serial.println("Closing connection");
  DeserializationError err = deserializeJson(doc, result);
  switch (err.code()) {
    case DeserializationError::Ok:
        Serial.println(F("Deserialization succeeded"));
        //serializeJsonPretty(doc, Serial);
        break;
    case DeserializationError::InvalidInput:
        Serial.println(F("Invalid input!"));
        break;
    case DeserializationError::NoMemory:
        Serial.println(F("Not enough memory"));
        return false;
        break;
    default:
        Serial.println(F("Deserialization failed"));
        break;
}
  //serializeJsonPretty(doc,Serial);
  return true;
}

void updateWeather()
{

// calculate time
  //time_t now = time(nullptr);
  //timeval tv = { rtc, 0 };
  //timezone tz = { TZ_MN + DST_MN, 0 };
  //settimeofday(&tv, &tz);
//pull time from weather info?
  //time_t now = doc["dt"];
  //dt = doc["dt"];

  time_t now;
  //
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  //Serial.println(timeinfo->tm_hour);
  createhour = timeinfo->tm_hour;
  createmin = timeinfo->tm_min;
  Serial.print("NTP Utc time: ");
  Serial.println(time(&now));
  JsonObject weather_0 = doc["weather"][0];
  weathercode = weather_0["id"]; // 800
  clouds_all = doc["clouds"]["all"];
  sys_sunrise = sys["sunrise"]; // 1554898169
  sys_sunset = sys["sunset"]; // 1554944080
  wind_speed = (int)doc["wind"]["speed"];
  Serial.print("Wind speed int: ");
  Serial.println("wind_speed");


  //result = "";

  Serial.print("\nWeather code: "); Serial.print(weathercode);
  Serial.print(" @ "); Serial.print(createhour); Serial.print(":"); Serial.println(createmin);

  // Get the current time of day, between 0 and 65535
  timeofday = map((createhour * 60) + createmin, 0, 1440, 0, 65535);


  Serial.print("Time of day = "); Serial.print(timeofday); Serial.println("/65535");

  /* void animConfig(
    uint16_t t,   // Time of day in fixed-point 16-bit units, where 0=midnight,
                // 32768=noon, 65536=midnight. THIS DOES NOT CORRESPOND TO
                // ANY SORT OF REAL-WORLD UNITS LIKE SECONDS, nor does it
                // handle things like seasons or Daylight Saving Time, it's
                // just an "ish" approximation to give the sky animation some
                // vague context. The time of day should be polled from the
                // same source that's providing the weather data, DO NOT use
                // millis() or micros() to attempt to follow real time, as
                // the NeoPixel library is known to mangle these interrupt-
                // based functions. TIME OF DAY IS "ISH!"
    uint8_t  c,   // Cloud cover, as a percentage (0-100).
    uint8_t  r,   // Rainfall as a "strength" value (0-255) that doesn't really
                // correspond to anything except "none" to "max."
    uint8_t  s,   // Snowfall, similar "strength" value (0-255).
    uint8_t  l,   // Lightning, ditto.
    uint8_t  w) { // Wind speed as a "strength" value (0-255) that also doesn't
                // correspond to anything real; this is the number of fixed-
                // point units that the clouds will move per frame. There are
                // 65536 units around the 'sky,' so a value of 255 will take
                // about 257 frames to make a full revolution of the LEDs,
                // which at 50 FPS would be a little over 5 seconds.
  **************************/

  //weathercode = 800; // hardcode weather animation test
  //timeofday = 32768;
  switch (weathercode) {
    case 999: // tornado! //not yet matched
      Serial.println("tornado");
      // lotsa cloud, no rain, and wind!
      animConfig(timeofday, 100, 0, 0, 0, wind_speed);
      break;
    case 221: // tropical storm
      Serial.println("tropical storm");
      // no cloud, a lot of rain, no snow, no thunder and lotsa wind!
      animConfig(timeofday, 0, 255, 0, 0, wind_speed);
      break;
    case 781: // hurricane
      Serial.println("hurricane");
      // some cloud, some rain, no snow, no thunder and lotsa wind!
      animConfig(timeofday, 50, 100, 0, 0, wind_speed);
      break;

    case 212: // severe thunder
      Serial.println("severe thunder");
      // some cloud, no rain, no snow, mega lightning, some wind!
      animConfig(timeofday, 100, 0, 0, 255, wind_speed);
      break;

    case 211: // thunder
      Serial.println("thunder");
      // some cloud, no rain, no snow, some lightning, some wind!
      animConfig(timeofday, 100, 0, 0, 100, wind_speed);
      break;

    case 615: // mixed rain + snow
    case 616: // mixed snow and sleet
    case 611: // sleet
      Serial.println("Rain/Snow/Sleet");
      // some cloud, some rain, some snow, no lightning, no wind!
      animConfig(timeofday, 10, 100, 100, 0, wind_speed);
      break;

    case 311: // freezing drizzle
    case 300: // light drizzle
    case 301: // drizzle
    case 310: // light drizzle rain
    case 500: // light rain
      Serial.println("Drizzle");
      // some cloud, a little rain, no snow, no lightning, no wind!
      animConfig(timeofday, 30, 70, 0, 0, wind_speed);
      break;

    case 511: // freezing rain
    case 521: // showers
    case 501: // showers
    case 502: // showers
    case 504: // showers
    case 522: // showers
    case 531: // showers
    case 302: // heavy drizzle
    case 312: // heavy drizzle rain
    case 313: // mixed rain and drizzle
    case 314: // heavy shower rain and drizzle
    case 315: // Shower drizzle
      Serial.println("Rain/Showers");
      // some cloud, lotsa rain, no snow, no lightning, no wind!
      animConfig(timeofday, 30, 250, 0, 0, wind_speed);
      break;

    case 620: // light snow showers
      Serial.println("flurries");
      // some cloud, no rain, some snow, no lightning, no wind!
      animConfig(timeofday, 30, 0, 100, 0, wind_speed);
      break;

    case 612: // blowing snow
      Serial.println("blowing snow");
      // some cloud, no rain, snow, no lightning, lotsa wind!
      animConfig(timeofday, 30, 0, 150, 0, wind_speed);
      break;

    case 601: // snow
    case 600: // light snow
      Serial.println("snow");
      // some cloud, no rain, snow, no lightning, no wind!
      animConfig(timeofday, 30, 0, 150, 0, wind_speed);
      break;

    case 602: // heavy snow
    case 622: // heavy snow
      Serial.println("heavy snow");
      // some cloud, no rain, lotsa snow, no lightning, no wind!
      animConfig(timeofday, 30, 0, 255, 0, wind_speed);
      break;

    case 800: // clear
      Serial.println("Clear/fair");
      // no cloud, no rain, no snow, no lightning, no wind!
      animConfig(timeofday, 0, 0, 0, 0, wind_speed);
      break;

    case 771: // squalls
      Serial.println("Windy");
      // no cloud, no rain, no snow, no lightning, lots wind
      animConfig(timeofday, 0, 0, 0, 0, wind_speed);
      break;

    case 802: // cloudy scattered
    case 803: // cloudy broken
    case 761: // dust
    case 701: // mist
    case 751: // sand
      Serial.println("Cloudy");
      // lotsa cloud, nothing else
      animConfig(timeofday, 255, 0, 0, 0, wind_speed);
      break;

    case 762: // ash
    case 804: // mostly cloudy
    case 741: // foggy
    case 711: // smoky
    case 721: // haze
    case 731: // dust whirls
      Serial.println("mostly Cloudy");
      // lotsa cloud, nothing else
      animConfig(timeofday, 150, 0, 0, 0, wind_speed);
      break;

    case 801: // partly cloudy
      Serial.println("Partly Cloudy");
      // lotsa cloud, nothing else
      animConfig(timeofday, 150, 0, 0, 0, wind_speed);
      break;

    case 200: // isolated thunderstorms
      Serial.println("isolated thunderstorms");
      // some cloud, some rain, no snow, some lite, no wind
      animConfig(timeofday, 30, 150, 0, 30, wind_speed);
      break;

    case 210: // scattered thunderstorms
    case 230: // scattered thundershowers
      Serial.println("scattered thundershowers");
      // some cloud, some rain, no snow, some lite, no wind
      animConfig(timeofday, 20, 150, 0, 60, wind_speed);
      break;

    case 201: // thundershowers
    case 202: // heavy thundershower
    case 231: // drizzle thunderstorm
    case 232: // drizzle thunderstorm
      Serial.println("thundershowers");
      // some cloud, rain, no snow, lite, no wind
      animConfig(timeofday, 20, 250, 0, 100, wind_speed);
      break;

    case 520: // scattered showers
      Serial.println("scattered showers");
      // some cloud, some rain, no snow, no lite, no wind
      animConfig(timeofday, 30, 50, 0, 0, wind_speed);
      break;

    case 621: // snow showers
      Serial.println("snow showers");
      // some cloud, some rain, some snow, no lite, no wind
      animConfig(timeofday, 30, 100, 100, 0, wind_speed);
      break;

    default:
      break;
    }
  }






void handleroot(){
  //String s = MAIN_page;
  String json_str;
  serializeJsonPretty(doc, json_str);
  //serializeJsonPretty(weather_0,json_str);
    server.send(200, "text/json", json_str);
  }

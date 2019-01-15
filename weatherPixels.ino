// 2019 Bas Emmen
// Release notes
// This is a modified version of the Adafruit Weather Lamp which used Yahoo Weather API before (Weather data and timestamp)
// Yahoo weather API does not work anymore. This version works with Openweathermap instead.
// info: https://openweathermap.org/current
// Openweathermap does not parse the currenttime. (only the moment where a weather change occurred.) this version uses pool.ntp.org
// Also changed the weather code id for each scenario to match Openweathermap codes: https://openweathermap.org/weather-conditions
// Dont forget to set Neopixel PIN and amount of Leds in animation.cpp

#include <ESP8266WiFi.h>
#include <time.h>

void animSetup(void);
void animConfig(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
void waitForFrame(void);
void renderFrame(void);

// YOUR VARIABLES ARE BELOW
const char* ssid     = "YOURSSID"; // Your Wifi SSID
const char* password = "YOURPASSWORD"; // Your Wifi password
const char *APIkey = "YOURAPIKEY"; // Your Openweather API key. Subscribe at https://openweathermap.org/appid
int8_t utc_offset = -2; // hours off of UTC, e.g. EST is -5
const char* location = "Amsterdam,NL"; // Your city,countrycode , e.g. Amsterdam,NL
// YOUR VARIABLES ARE ABOVE. Dont forget edit the animation.cpp file

const char* path_prefix = "http://api.openweathermap.org/data/2.5/weather?q=";
const char* path_postfix1 = "&APPID=";
const char* path_postfix2 = "&mode=json&units=metric&cnt=1";
const char* host = "api.openweathermap.org";
const int httpPort = 80;

int16_t weathercode = -1;
int16_t createhour, createmin;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  animConfig(0, 0, 0, 0, 0, 0);
  animSetup();
}

uint32_t timekeep = 0xFFFF;

void loop() {
  uint32_t currTime = millis();
  // every 240 seconds (or if there's a rollover/first time running, update the weather!
  if ((timekeep > currTime)  || (currTime > (timekeep + 240000))) {
    timekeep = currTime;
    updateWeather();
  }
  
  waitForFrame();
  renderFrame();

}


void updateWeather() {

  Serial.print("Connecting to "); Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // We now create a URI for the request

  String url = String(path_prefix) + String(location) + String(path_postfix1)+ String(APIkey) + String(path_postfix2);

  Serial.print("Requesting URL: ");  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);

  weathercode = -1;
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    int i = line.indexOf(String("weather"));
    if (i < 0) continue;
    Serial.println(line);
    weathercode = (line.substring(i + 16)).toInt();
  }

  Serial.println("Closing connection");

// calculate time
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);  
  Serial.println(timeinfo->tm_hour);
  createhour = timeinfo->tm_hour + utc_offset;
  createmin = timeinfo->tm_min;

  Serial.print("\nWeather code: "); Serial.print(weathercode);
  Serial.print(" @ "); Serial.print(createhour); Serial.print(":"); Serial.println(createmin);

  // Get the current time of day, between 0 and 65535
  uint16_t timeofday = map((createhour * 60) + createmin, 0, 1440, 0, 65535);

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

 // weathercode = 32; // hardcode weather animation test

  switch (weathercode) {
    case 999: // tornado! //not yet matched
      Serial.println("tornado");
      // lotsa cloud, no rain, and wind!
      animConfig(timeofday, 100, 0, 0, 0, 255);
      break;
    case 221: // tropical storm
      Serial.println("tropical storm");
      // no cloud, a lot of rain, no snow, no thunder and lotsa wind!
      animConfig(timeofday, 0, 255, 0, 0, 255);
      break;
    case 781: // hurricane
      Serial.println("hurricane");
      // some cloud, some rain, no snow, no thunder and lotsa wind!
      animConfig(timeofday, 50, 100, 0, 0, 255);
      break;

    case 212: // severe thunder
      Serial.println("severe thunder");
      // some cloud, no rain, no snow, mega lightning, some wind!
      animConfig(timeofday, 100, 0, 0, 255, 20);
      break;

    case 211: // thunder
      Serial.println("thunder");
      // some cloud, no rain, no snow, some lightning, some wind!
      animConfig(timeofday, 100, 0, 0, 100, 50);
      break;

    case 615: // mixed rain + snow
    case 616: // mixed snow and sleet
    case 611: // sleet
      Serial.println("Rain/Snow/Sleet");
      // some cloud, some rain, some snow, no lightning, no wind!
      animConfig(timeofday, 10, 100, 100, 0, 0);
      break;

    case 311: // freezing drizzle
    case 300: // light drizzle
    case 301: // drizzle
    case 310: // light drizzle rain
    case 500: // light rain
      Serial.println("Drizzle");
      // some cloud, a little rain, no snow, no lightning, no wind!
      animConfig(timeofday, 30, 70, 0, 0, 0);
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
      animConfig(timeofday, 30, 250, 0, 0, 0);
      break;

    case 620: // light snow showers
      Serial.println("flurries");
      // some cloud, no rain, some snow, no lightning, no wind!
      animConfig(timeofday, 30, 0, 100, 0, 0);
      break;

    case 612: // blowing snow
      Serial.println("blowing snow");
      // some cloud, no rain, snow, no lightning, lotsa wind!
      animConfig(timeofday, 30, 0, 150, 0, 200);
      break;

    case 601: // snow
    case 600: // light snow
      Serial.println("snow");
      // some cloud, no rain, snow, no lightning, no wind!
      animConfig(timeofday, 30, 0, 150, 0, 0);
      break;

    case 602: // heavy snow
    case 622: // heavy snow
      Serial.println("heavy snow");
      // some cloud, no rain, lotsa snow, no lightning, no wind!
      animConfig(timeofday, 30, 0, 255, 0, 0);
      break;

    case 800: // clear
      Serial.println("Clear/fair");
      // no cloud, no rain, no snow, no lightning, no wind!
      animConfig(timeofday, 0, 0, 0, 0, 0);
      break;

    case 771: // squalls
      Serial.println("Windy");
      // no cloud, no rain, no snow, no lightning, lots wind
      animConfig(timeofday, 0, 0, 0, 0, 200);
      break;

    case 802: // cloudy scattered
    case 803: // cloudy broken
    case 761: // dust
    case 701: // mist
    case 751: // sand
      Serial.println("Cloudy");
      // lotsa cloud, nothing else
      animConfig(timeofday, 255, 0, 0, 0, 0);
      break;

    case 762: // ash
    case 804: // mostly cloudy
    case 741: // foggy
    case 711: // smoky
    case 721: // haze
    case 731: // dust whirls
      Serial.println("mostly Cloudy");
      // lotsa cloud, nothing else
      animConfig(timeofday, 150, 0, 0, 0, 0);
      break;

    case 801: // partly cloudy
      Serial.println("Partly Cloudy");
      // lotsa cloud, nothing else
      animConfig(timeofday, 150, 0, 0, 0, 0);
      break;

    case 200: // isolated thunderstorms
      Serial.println("isolated thunderstorms");
      // some cloud, some rain, no snow, some lite, no wind
      animConfig(timeofday, 30, 150, 0, 30, 0);
      break;

    case 210: // scattered thunderstorms
    case 230: // scattered thundershowers
      Serial.println("scattered thundershowers");
      // some cloud, some rain, no snow, some lite, no wind
      animConfig(timeofday, 20, 150, 0, 60, 0);
      break;

    case 201: // thundershowers
    case 202: // heavy thundershower
    case 231: // drizzle thunderstorm
    case 232: // drizzle thunderstorm
      Serial.println("thundershowers");
      // some cloud, rain, no snow, lite, no wind
      animConfig(timeofday, 20, 250, 0, 100, 0);
      break;

    case 520: // scattered showers
      Serial.println("scattered showers");
      // some cloud, some rain, no snow, no lite, no wind
      animConfig(timeofday, 30, 50, 0, 0, 0);
      break;

    case 621: // snow showers
      Serial.println("snow showers");
      // some cloud, some rain, some snow, no lite, no wind
      animConfig(timeofday, 30, 100, 100, 0, 0);
      break;

    default:
      break;
  }
  /*

    25  cold
    36  hot
    3200  not available
  */
}

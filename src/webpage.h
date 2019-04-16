const char MAIN_page[] PROGMEM = R"=====(
<HTML>
  <HEAD>
      <TITLE>My first web page</TITLE>
  </HEAD>
<BODY>
  <CENTER>
      <B>Hello World.... </B>
  </CENTER>
</BODY>
</HTML>
)=====";






/*
void writeStripConfigFS(void){
  //save the strip config to FS JSON

  DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4)+300);
  JsonObject json = jsonBuffer.to<JsonObject>();
  json["pixel_pount"] = WS2812FXStripSettings.stripSize;
  json["rgb_order"] = WS2812FXStripSettings.RGBOrder;
  json["pin"] = WS2812FXStripSettings.pin;

  //SPIFFS.remove("/neoconfig.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
  File configFile = SPIFFS.open("/neoconfig.json", "w");
  if (!configFile) {
    DBG_OUTPUT_PORT.println("Failed!");
    updateFS = false;
  }
  serializeJson(json, DBG_OUTPUT_PORT);
  serializeJson(json, configFile);
  DBG_OUTPUT_PORT.println();
  configFile.close();
  updateFS = false;
  //end save
}
*/

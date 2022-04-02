/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Firmware for Legacy UDAWA Board
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#include "main.h"

Settings mySettings;

const size_t callbacksSize = 6;
GenericCallback callbacks[callbacksSize] = {
  { "sharedAttributesUpdate", processSharedAttributesUpdate },
  { "provisionResponse", processProvisionResponse },
  { "saveConfig", processSaveConfig },
  { "saveSettings", processSaveSettings },
  { "syncClientAttributes", processSyncClientAttributes },
  { "reboot", processReboot }
};

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound())
  {
    config.frame_size = FRAMESIZE_HQVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_96X96;
    config.jpeg_quality = 20;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
  }

  delay(1000);

  startup();
  loadSettings();

  if(mySettings.fTeleDev)
  {
    taskManager.scheduleFixedRate(1000, [] {
      publishDeviceTelemetry();
    });
  }

  taskManager.scheduleFixedRate(mySettings.myTaskInterval, [] {
    myTask();
  });
}

void loop()
{
  udawa();

  if(tb.connected() && FLAG_IOT_SUBSCRIBE)
  {
    if(tb.callbackSubscribe(callbacks, callbacksSize))
    {
      sprintf_P(logBuff, PSTR("Callbacks subscribed successfuly!"));
      recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
      FLAG_IOT_SUBSCRIBE = false;
    }
    if (tb.Firmware_Update(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION))
    {
      sprintf_P(logBuff, PSTR("OTA Update finished, rebooting..."));
      recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
      reboot();
    }
    else
    {
      sprintf_P(logBuff, PSTR("Firmware up-to-date."));
      recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
    }

    syncClientAttributes();
  }
}

void loadSettings()
{
  StaticJsonDocument<DOCSIZE> doc;
  readSettings(doc, settingsPath);

  if(doc["fTeleDev"] != nullptr)
  {
    mySettings.fTeleDev = doc["fTeleDev"].as<bool>();
  }
  else
  {
    mySettings.fTeleDev = 1;
  }

  if(doc["myTaskInterval"] != nullptr)
  {
    mySettings.myTaskInterval = doc["myTaskInterval"].as<unsigned long>();
  }
  else
  {
    mySettings.myTaskInterval = 30000;
  }

}

void saveSettings()
{
  StaticJsonDocument<DOCSIZE> doc;

  doc["fTeleDev"] = mySettings.fTeleDev;
  doc["myTaskInterval"] = mySettings.myTaskInterval;


  writeSettings(doc, settingsPath);
}

callbackResponse processSaveConfig(const callbackData &data)
{
  configSave();
  return callbackResponse("saveConfig", 1);
}

callbackResponse processSaveSettings(const callbackData &data)
{
  saveSettings();
  loadSettings();

  mySettings.lastUpdated = millis();
  return callbackResponse("saveSettings", 1);
}

callbackResponse processReboot(const callbackData &data)
{
  reboot();
  return callbackResponse("reboot", 1);
}

callbackResponse processSyncClientAttributes(const callbackData &data)
{
  syncClientAttributes();
  return callbackResponse("syncClientAttributes", 1);
}

callbackResponse processSharedAttributesUpdate(const callbackData &data)
{
  sprintf_P(logBuff, PSTR("Received shared attributes update:"));
  recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
  if(config.logLev >= 4){serializeJsonPretty(data, Serial);}

  if(data["model"] != nullptr){strlcpy(config.model, data["model"].as<const char*>(), sizeof(config.model));}
  if(data["group"] != nullptr){strlcpy(config.group, data["group"].as<const char*>(), sizeof(config.group));}
  if(data["broker"] != nullptr){strlcpy(config.broker, data["broker"].as<const char*>(), sizeof(config.broker));}
  if(data["port"] != nullptr){data["port"].as<uint16_t>();}
  if(data["wssid"] != nullptr){strlcpy(config.wssid, data["wssid"].as<const char*>(), sizeof(config.wssid));}
  if(data["wpass"] != nullptr){strlcpy(config.wpass, data["wpass"].as<const char*>(), sizeof(config.wpass));}
  if(data["dssid"] != nullptr){strlcpy(config.dssid, data["dssid"].as<const char*>(), sizeof(config.dssid));}
  if(data["dpass"] != nullptr){strlcpy(config.dpass, data["dpass"].as<const char*>(), sizeof(config.dpass));}
  if(data["upass"] != nullptr){strlcpy(config.upass, data["upass"].as<const char*>(), sizeof(config.upass));}
  if(data["provisionDeviceKey"] != nullptr){strlcpy(config.provisionDeviceKey, data["provisionDeviceKey"].as<const char*>(), sizeof(config.provisionDeviceKey));}
  if(data["provisionDeviceSecret"] != nullptr){strlcpy(config.provisionDeviceSecret, data["provisionDeviceSecret"].as<const char*>(), sizeof(config.provisionDeviceSecret));}
  if(data["logLev"] != nullptr){config.logLev = data["logLev"].as<uint8_t>();}

  if(data["fTeleDev"] != nullptr){mySettings.fTeleDev = data["fTeleDev"].as<bool>();}
  if(data["myTaskInterval"] != nullptr){mySettings.myTaskInterval = data["myTaskInterval"].as<unsigned long>();}

  mySettings.lastUpdated = millis();
  return callbackResponse("sharedAttributesUpdate", 1);
}

void syncClientAttributes()
{
  StaticJsonDocument<DOCSIZE> doc;

  IPAddress ip = WiFi.localIP();
  char ipa[25];
  sprintf(ipa, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  doc["ipad"] = ipa;
  doc["compdate"] = COMPILED;
  doc["fmTitle"] = CURRENT_FIRMWARE_TITLE;
  doc["fmVersion"] = CURRENT_FIRMWARE_VERSION;
  doc["stamac"] = WiFi.macAddress();
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["apmac"] = WiFi.softAPmacAddress();
  doc["flashFree"] = ESP.getFreeSketchSpace();
  doc["firmwareSize"] = ESP.getSketchSize();
  doc["flashSize"] = ESP.getFlashChipSize();
  doc["sdkVer"] = ESP.getSdkVersion();
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["model"] = config.model;
  doc["group"] = config.group;
  doc["broker"] = config.broker;
  doc["port"] = config.port;
  doc["wssid"] = config.wssid;
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["wpass"] = config.wpass;
  doc["dssid"] = config.dssid;
  doc["dpass"] = config.dpass;
  doc["upass"] = config.upass;
  doc["accessToken"] = config.accessToken;
  doc["provisionDeviceKey"] = config.provisionDeviceKey;
  doc["provisionDeviceSecret"] = config.provisionDeviceSecret;
  doc["logLev"] = config.logLev;
  doc["fTeleDev"] = mySettings.fTeleDev;
  doc["myTaskInterval"] = mySettings.myTaskInterval;

  tb.sendAttributeDoc(doc);
  doc.clear();
}

void publishDeviceTelemetry()
{
  StaticJsonDocument<DOCSIZE> doc;

  doc["heap"] = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis()/1000;
  tb.sendTelemetryDoc(doc);
  doc.clear();
}

void myTask()
{
  //capture gambar
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    sprintf_P(logBuff, PSTR("Camera capture failed."));
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  else
  {
    int freeHeapBefore = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    String imgDataB64 = base64::encode(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    int freeHeapAfter = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    sprintf_P(logBuff, PSTR("Free heap: %d vs %d - Image buffer length: %d - after base64: %d"), freeHeapBefore, freeHeapAfter, fb->len, imgDataB64.length());
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));

    DynamicJsonDocument doc(imgDataB64.length() + 128);
    doc["b64"] = imgDataB64;
    doc["w"] = fb->width;
    doc["h"] = fb->height;
    doc["f"] = fb->format;


    String data;
    serializeJson(doc, data);
    doc.clear();

    uint16_t maxSize = data.length() + (uint16_t)128;
    bool setBufferSizeStatus = tb.setBufferSize(maxSize);

    bool deliveryStatus = tb.sendTelemetryJson(data.c_str());

    sprintf_P(logBuff, PSTR("Final data length: %d - BufferSize: %d - targetBufferSize: %d - Set Buffer Status: %d, Delivery Status: %d"), data.length(), tb.getBufferSize(), maxSize, (int)setBufferSizeStatus, (int)deliveryStatus);
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));

    tb.setBufferSize(DOCSIZE);
  }
}
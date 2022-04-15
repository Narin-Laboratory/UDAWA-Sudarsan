/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Firmware for Legacy UDAWA Board
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#include "main.h"

Settings mySettings;

const size_t callbacksSize = 7;
GenericCallback callbacks[callbacksSize] = {
  { "sharedAttributesUpdate", processSharedAttributesUpdate },
  { "provisionResponse", processProvisionResponse },
  { "saveConfig", processSaveConfig },
  { "saveSettings", processSaveSettings },
  { "syncClientAttributes", processSyncClientAttributes },
  { "reboot", processReboot },
  { "snap", processSnap }
};

void setup()
{
  startup();
  loadSettings();

  networkInit();
  tb.setBufferSize(DOCSIZE);

  if(mySettings.fTeleDev)
  {
    taskManager.scheduleFixedRate(1000, [] {
      publishDeviceTelemetry();
    });
  }

  if(mySettings.myTaskInterval > 0){
    taskManager.scheduleFixedRate(mySettings.myTaskInterval, [] {
      myTask();
    });
  }
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

  if(doc["frameSize"] != nullptr)
  {
    mySettings.frameSize = doc["frameSize"].as<uint8_t>();
  }
  else
  {
    mySettings.frameSize = 1;
  }

}

void saveSettings()
{
  StaticJsonDocument<DOCSIZE> doc;

  doc["fTeleDev"] = mySettings.fTeleDev;
  doc["myTaskInterval"] = mySettings.myTaskInterval;
  doc["frameSize"] = mySettings.frameSize;

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

callbackResponse processSnap(const callbackData &data)
{
  myTask();
  return callbackResponse("snap", 1);
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
  if(data["frameSize"] != nullptr){mySettings.frameSize = data["frameSize"].as<uint8_t>();}

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
  doc["frameSize"] = mySettings.frameSize;

  tb.sendAttributeDoc(doc);
  doc.clear();
}

void publishDeviceTelemetry()
{
  StaticJsonDocument<DOCSIZE> doc;

  doc["heap"] = heap_caps_get_free_size(MALLOC_CAP_32BIT);
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis()/1000;
  tb.sendTelemetryDoc(doc);
  doc.clear();
}

void myTask()
{
  if(tb.connected())
  {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    camera_config_t cam;
    cam.ledc_channel = LEDC_CHANNEL_0;
    cam.ledc_timer = LEDC_TIMER_0;
    cam.pin_d0 = Y2_GPIO_NUM;
    cam.pin_d1 = Y3_GPIO_NUM;
    cam.pin_d2 = Y4_GPIO_NUM;
    cam.pin_d3 = Y5_GPIO_NUM;
    cam.pin_d4 = Y6_GPIO_NUM;
    cam.pin_d5 = Y7_GPIO_NUM;
    cam.pin_d6 = Y8_GPIO_NUM;
    cam.pin_d7 = Y9_GPIO_NUM;
    cam.pin_xclk = XCLK_GPIO_NUM;
    cam.pin_pclk = PCLK_GPIO_NUM;
    cam.pin_vsync = VSYNC_GPIO_NUM;
    cam.pin_href = HREF_GPIO_NUM;
    cam.pin_sscb_sda = SIOD_GPIO_NUM;
    cam.pin_sscb_scl = SIOC_GPIO_NUM;
    cam.pin_pwdn = PWDN_GPIO_NUM;
    cam.pin_reset = RESET_GPIO_NUM;
    cam.xclk_freq_hz = 20000000;
    cam.pixel_format = PIXFORMAT_JPEG;


    cam.frame_size = (framesize_t)mySettings.frameSize;
    cam.jpeg_quality = 10;
    cam.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&cam);
    if (err != ESP_OK) {
      sprintf_P(logBuff, PSTR("Camera init failed with error 0x%x"), err);
      recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
      reboot();
    }

    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();

    if (!fb) {
      sprintf_P(logBuff, PSTR("Camera capture failed."));
      recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
      reboot();
    }
    else
    {
      size_t size = base64_encode_expected_len(fb->len) + 1;
      char * buffer = (char *) ps_malloc(size);
      if(buffer) {
        base64_encodestate _state;
        base64_init_encodestate(&_state);
        int len = base64_encode_block((const char *) &fb->buf[0], fb->len, &buffer[0], &_state);
        len = base64_encode_blockend((buffer + len), &_state);
      }
      esp_camera_fb_return(fb);

      SpiRamJsonDocument doc(size + 64);
      doc["b64"] = buffer;
      free(buffer);
      doc["w"] = fb->width;
      doc["h"] = fb->height;
      doc["f"] = fb->format;

      size_t finalDataSize = measureJson(doc);
      char * data = (char *) ps_malloc(finalDataSize);
      serializeJson(doc, data, finalDataSize);
      doc.clear();

      tb.beginPublish("v1/devices/me/telemetry", finalDataSize, 0);
      for(int i=0;i<finalDataSize;i++){
        float progress = ((float)i+1.0) / (float)(finalDataSize) * 100.0;
        sprintf_P(logBuff, PSTR("Chunk %d/%d: (%.2f %%) - %d/%d - %d"), i+1, finalDataSize, progress, heap_caps_get_free_size(MALLOC_CAP_32BIT), ESP.getPsramSize(), ESP.getFreeHeap());
        Serial.print(data[i]);
        Serial.print(" - ");
        Serial.println(logBuff);
        tb.write(data[i]);
      }
      tb.endPublish();
    }
  }
}
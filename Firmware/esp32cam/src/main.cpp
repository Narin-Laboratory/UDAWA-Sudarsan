/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Firmware for Legacy UDAWA Board
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#include "main.h"

using namespace libudawa;
Settings mySettings;

const size_t callbacksSize = 9;
GenericCallback callbacks[callbacksSize] = {
  { "sharedAttributesUpdate", processSharedAttributesUpdate },
  { "provisionResponse", processProvisionResponse },
  { "saveConfig", processSaveConfig },
  { "saveSettings", processSaveSettings },
  { "syncClientAttributes", processSyncClientAttributes },
  { "reboot", processReboot },
  { "snap", processSnap },
  { "setFlash", processSetFlash },
  { "getFlash", processGetFlash }
};

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
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
    taskManager.scheduleFixedRate(mySettings.myTaskInterval * 1000, [] {
      myTask();
    });
  }

  config.logLev = 5;
}

void loop()
{
  udawa();

  if(tb.connected() && FLAG_IOT_SUBSCRIBE)
  {
    if(tb.callbackSubscribe(callbacks, callbacksSize))
    {
      log_manager->info(PSTR(__func__),PSTR("Callbacks subscribed successfuly!"));
      FLAG_IOT_SUBSCRIBE = false;
    }
    if (tb.Firmware_Update(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION))
    {
      log_manager->info(PSTR(__func__),PSTR("OTA Update finished, rebooting..."));
      reboot();
    }
    else
    {
      log_manager->info(PSTR(__func__),PSTR("Firmware up-to-date."));
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
    mySettings.myTaskInterval = 0;
  }

  if(doc["frameSize"] != nullptr)
  {
    mySettings.frameSize = doc["frameSize"].as<uint8_t>();
  }
  else
  {
    mySettings.frameSize = 17;
  }

  if(doc["jpegQuality"] != nullptr)
  {
    mySettings.jpegQuality = doc["jpegQuality"].as<int>();
  }
  else
  {
    mySettings.jpegQuality = 10;
  }

  if(doc["tempBuffSize"] != nullptr)
  {
    mySettings.tempBuffSize = doc["tempBuffSize"].as<int>();
  }
  else
  {
    mySettings.tempBuffSize = 16384;
  }

}

void saveSettings()
{
  StaticJsonDocument<DOCSIZE> doc;

  doc["fTeleDev"] = mySettings.fTeleDev;
  doc["myTaskInterval"] = mySettings.myTaskInterval;
  doc["frameSize"] = mySettings.frameSize;
  doc["jpegQuality"] = mySettings.jpegQuality;
  doc["tempBuffSize"] = mySettings.tempBuffSize;

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

callbackResponse processSetFlash(const callbackData &data)
{
  pinMode(4, OUTPUT);
  digitalWrite(4, data["params"].as<int>());
  return callbackResponse("setFlash", data["params"].as<int>());
}

callbackResponse processGetFlash(const callbackData &data)
{
  return callbackResponse("getFlash", (int)digitalRead(4));
}

callbackResponse processSyncClientAttributes(const callbackData &data)
{
  syncClientAttributes();
  return callbackResponse("syncClientAttributes", 1);
}

callbackResponse processSharedAttributesUpdate(const callbackData &data)
{
  //log_manager->debug(PSTR(__func__),PSTR("Received shared attributes update:"));
  //recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
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
  if(data["gmtOffset"] != nullptr){config.gmtOffset = data["gmtOffset"].as<int>();}

  if(data["fTeleDev"] != nullptr){mySettings.fTeleDev = data["fTeleDev"].as<bool>();}
  if(data["myTaskInterval"] != nullptr){mySettings.myTaskInterval = data["myTaskInterval"].as<unsigned long>();}
  if(data["frameSize"] != nullptr){mySettings.frameSize = data["frameSize"].as<uint8_t>();}
  if(data["jpegQuality"] != nullptr){mySettings.jpegQuality = data["jpegQuality"].as<int>();}
  if(data["tempBuffSize"] != nullptr){mySettings.tempBuffSize = data["tempBuffSize"].as<uint16_t>();}

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
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["logLev"] = config.logLev;
  doc["gmtOffset"] = config.gmtOffset;
  doc["fTeleDev"] = mySettings.fTeleDev;
  doc["myTaskInterval"] = mySettings.myTaskInterval;
  doc["frameSize"] = mySettings.frameSize;
  doc["jpegQuality"] = mySettings.jpegQuality;
  doc["tempBuffSize"] = mySettings.tempBuffSize;
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
    cam.pin_sccb_sda = SIOD_GPIO_NUM;
    cam.pin_sccb_scl = SIOC_GPIO_NUM;
    cam.pin_pwdn = PWDN_GPIO_NUM;
    cam.pin_reset = RESET_GPIO_NUM;
    cam.xclk_freq_hz = 20000000;
    cam.pixel_format = PIXFORMAT_JPEG;
    cam.fb_count = 2;
    cam.frame_size = (framesize_t)mySettings.frameSize;
    cam.jpeg_quality = mySettings.jpegQuality;

    log_manager->debug(PSTR(__func__),PSTR("Taking snap (jpegQuality: %d, frameSize: %d), please wait."), cam.jpeg_quality, (int)cam.frame_size);

    // camera init
    esp_err_t err = esp_camera_init(&cam);
    if (err != ESP_OK) {
      log_manager->error(PSTR(__func__),PSTR("Camera init failed with error 0x%x"), err);
      esp_camera_deinit();
    }
    delay(5000);
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();

    if (!fb) {
      log_manager->error(PSTR(__func__),PSTR("Camera capture failed."));
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
      size_t w = fb->width;
      size_t h = fb->height;
      pixformat_t f = fb->format;
      esp_camera_fb_return(fb);
      esp_camera_deinit();

      SpiRamJsonDocument doc(size + 64);
      doc["b64"] = buffer;
      free(buffer);
      doc["w"] = w;
      doc["h"] = h;
      doc["f"] = f;

      size_t finalDataSize = measureJson(doc);
      byte * data = (byte *) ps_malloc(finalDataSize);
      serializeJson(doc, data, finalDataSize);
      doc.clear();

      log_manager->debug(PSTR(__func__),PSTR("Snap size %d.  Memory: %d/%d - %d. Sending now..."), finalDataSize, heap_caps_get_free_size(MALLOC_CAP_32BIT), ESP.getPsramSize(), ESP.getFreeHeap());

      const char* topic = "v1/devices/me/attributes";
      publishMqtt(topic, data, finalDataSize);
      free(data);
    }
  }
}

void publishMqtt(const char* channel, uint8_t *data, uint32_t len) {
  unsigned long start_ts = millis();

  tb.beginPublish(channel, len, false);

  size_t res;
  uint32_t offset = 0;
  uint32_t to_write = len;
  uint32_t buf_len;
  do {
    buf_len = to_write;
    if (buf_len > mySettings.tempBuffSize){
      buf_len = mySettings.tempBuffSize;
    }
    res = tb.write(data+offset, buf_len);
    offset += buf_len;
    to_write -= buf_len;
  } while (res == buf_len && to_write > 0);

  tb.endPublish();

  log_manager->debug(PSTR(__func__),PSTR("Finished: (size %d bytes, %d bytes written in %ld ms)\n"), len, len-to_write, millis()-start_ts);
}
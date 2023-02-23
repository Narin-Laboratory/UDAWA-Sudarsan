/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Firmware for UDAWA Sudarsan (AI Camera)
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#include "main.h"

using namespace libudawa;
Settings mySettings;

const size_t callbacksSize = 10;
GenericCallback callbacks[callbacksSize] = {
  { "sharedAttributesUpdate", processSharedAttributesUpdate },
  { "provisionResponse", processProvisionResponse },
  { "saveConfig", processSaveConfig },
  { "saveSettings", processSaveSettings },
  { "syncClientAttributes", processSyncClientAttributes },
  { "reboot", processReboot },
  { "snap", processSnap },
  { "setFlash", processSetFlash },
  { "getFlash", processGetFlash },
  { "resetConfig",  processResetConfig}
};

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  startup();
  loadSettings();
  if(String(config.model) == String("Generic")){
    strlcpy(config.model, "Sudarsan", sizeof(config.model));
  }

  networkInit();
  tb.setBufferSize(DOCSIZE);

  if(mySettings.intvDevTel != 0){
    taskid_t taskPublishDeviceTelemetry = taskManager.scheduleFixedRate(mySettings.intvDevTel * 1000, [] {
      publishDeviceTelemetry();
    });
  }

  if(mySettings.intvSnap != 0){
    taskid_t tasksnap = taskManager.scheduleFixedRate(mySettings.intvSnap * 1000, [] {
      snap();
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
      log_manager->info(PSTR(__func__),PSTR("Callbacks subscribed successfuly!\n"));
      FLAG_IOT_SUBSCRIBE = false;
    }
    if (tb.Firmware_Update(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION))
    {
      log_manager->info(PSTR(__func__),PSTR("OTA Update finished, rebooting...\n"));
      reboot();
    }
    else
    {
      log_manager->info(PSTR(__func__),PSTR("Firmware up-to-date.\n"));
    }

    syncClientAttributes();
  }
}

void loadSettings()
{
  StaticJsonDocument<DOCSIZE> doc;
  readSettings(doc, settingsPath);

  if(doc["intvDevTel"] != nullptr)
  {
    mySettings.intvDevTel = doc["intvDevTel"].as<unsigned long>();
  }
  else
  {
    mySettings.intvDevTel = 1;
  }

  if(doc["intvSnap"] != nullptr)
  {
    mySettings.intvSnap = doc["intvSnap"].as<unsigned long>();
  }
  else
  {
    mySettings.intvSnap = 0;
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

if(doc["brightness"] != nullptr){mySettings.brightness = doc["brightness"].as<int>();}
  else{mySettings.brightness = 0;}
if(doc["contrast"] != nullptr){mySettings.contrast = doc["contrast"].as<int>();}
  else{mySettings.contrast = 0;}
if(doc["saturation"] != nullptr){mySettings.saturation = doc["saturation"].as<int>();}
  else{mySettings.saturation = 0;}
if(doc["special_effect"] != nullptr){mySettings.special_effect = doc["special_effect"].as<int>();}
  else{mySettings.special_effect = 0;}
if(doc["whitebal"] != nullptr){mySettings.whitebal = doc["whitebal"].as<int>();}
  else{mySettings.whitebal = 1;}
if(doc["awb_gain"] != nullptr){mySettings.awb_gain = doc["awb_gain"].as<int>();}
  else{mySettings.awb_gain = 1;}
if(doc["wb_mode"] != nullptr){mySettings.wb_mode = doc["wb_mode"].as<int>();}
  else{mySettings.wb_mode = 0;}
if(doc["exposure_ctrl"] != nullptr){mySettings.exposure_ctrl = doc["exposure_ctrl"].as<int>();}
  else{mySettings.exposure_ctrl = 0;}
if(doc["aec2"] != nullptr){mySettings.aec2 = doc["aec2"].as<int>();}
  else{mySettings.brightness = 0;}
if(doc["ae_level"] != nullptr){mySettings.ae_level = doc["ae_level"].as<int>();}
  else{mySettings.ae_level = 0;}
if(doc["aec_value"] != nullptr){mySettings.aec_value = doc["aec_value"].as<int>();}
  else{mySettings.aec_value = 300;}
if(doc["gain_ctrl"] != nullptr){mySettings.gain_ctrl = doc["gain_ctrl"].as<int>();}
  else{mySettings.gain_ctrl = 1;}
if(doc["agc_gain"] != nullptr){mySettings.agc_gain = doc["agc_gain"].as<int>();}
  else{mySettings.agc_gain = 0;}
if(doc["gainceiling"] != nullptr){mySettings.gainceiling = doc["gainceiling"].as<gainceiling_t>();}
  else{mySettings.gainceiling = (gainceiling_t)0;}
if(doc["bpc"] != nullptr){mySettings.bpc = doc["bpc"].as<int>();}
  else{mySettings.bpc = 0;}
if(doc["wpc"] != nullptr){mySettings.wpc = doc["wpc"].as<int>();}
  else{mySettings.wpc = 1;}
if(doc["raw_gma"] != nullptr){mySettings.raw_gma = doc["raw_gma"].as<int>();}
  else{mySettings.raw_gma = 1;}
if(doc["lenc"] != nullptr){mySettings.lenc = doc["lenc"].as<int>();}
  else{mySettings.lenc = 1;}
if(doc["hmirror"] != nullptr){mySettings.hmirror = doc["hmirror"].as<int>();}
  else{mySettings.hmirror = 1;}
if(doc["vflip"] != nullptr){mySettings.vflip = doc["vflip"].as<int>();}
  else{mySettings.vflip = 1;}
if(doc["lenc"] != nullptr){mySettings.lenc = doc["lenc"].as<int>();}
  else{mySettings.lenc = 1;}
if(doc["dcw"] != nullptr){mySettings.dcw = doc["dcw"].as<int>();}
  else{mySettings.dcw = 1;}
if(doc["colorbar"] != nullptr){mySettings.colorbar = doc["colorbar"].as<int>();}
  else{mySettings.colorbar = 0;}

}

void saveSettings()
{
  StaticJsonDocument<DOCSIZE> doc;

  doc["intvDevTel"] = mySettings.intvDevTel;
  doc["intvSnap"] = mySettings.intvSnap;
  doc["frameSize"] = mySettings.frameSize;
  doc["jpegQuality"] = mySettings.jpegQuality;
  doc["tempBuffSize"] = mySettings.tempBuffSize;

  doc["brightness"] = mySettings.brightness;
  doc["contrast"] = mySettings.contrast;
  doc["saturation"] = mySettings.saturation;
  doc["special_effect"] = mySettings.special_effect;
  doc["whitebal"] = mySettings.whitebal;
  doc["awb_gain"] = mySettings.awb_gain;
  doc["wb_mode"] = mySettings.wb_mode;
  doc["exposure_ctrl"] = mySettings.exposure_ctrl;
  doc["aec2"] = mySettings.aec2;
  doc["ae_level"] = mySettings.ae_level;
  doc["aec_value"] = mySettings.aec_value;
  doc["gain_ctrl"] = mySettings.gain_ctrl;
  doc["agc_gain"] = mySettings.agc_gain;
  doc["gainceiling"] = mySettings.gainceiling;
  doc["bpc"] = mySettings.bpc;
  doc["wpc"] = mySettings.wpc;
  doc["raw_gma"] = mySettings.raw_gma;
  doc["lenc"] = mySettings.lenc;
  doc["hmirror"] = mySettings.hmirror;
  doc["vflip"] = mySettings.vflip;
  doc["dcw"] = mySettings.dcw;
  doc["colorbar"] = mySettings.colorbar;

  writeSettings(doc, settingsPath);
}

callbackResponse processResetConfig(const callbackData &data){
  configReset();
  reboot();
  return callbackResponse("resetConfig", 1);
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
  snap();
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
  //log_manager->debug(PSTR(__func__),PSTR("Received shared attributes update:\n"));
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

  if(data["intvDevTel"] != nullptr){mySettings.intvDevTel = data["intvDevTel"].as<unsigned long>();}
  if(data["intvSnap"] != nullptr){mySettings.intvSnap = data["intvSnap"].as<unsigned long>();}
  if(data["frameSize"] != nullptr){mySettings.frameSize = data["frameSize"].as<uint8_t>();}
  if(data["jpegQuality"] != nullptr){mySettings.jpegQuality = data["jpegQuality"].as<int>();}
  if(data["tempBuffSize"] != nullptr){mySettings.tempBuffSize = data["tempBuffSize"].as<uint16_t>();}

  if(data["brightness"] != nullptr){mySettings.brightness = data["brightness"].as<int>();}
  if(data["contrast"] != nullptr){mySettings.contrast = data["contrast"].as<int>();}
  if(data["saturation"] != nullptr){mySettings.saturation = data["saturation"].as<int>();}
  if(data["special_effect"] != nullptr){mySettings.special_effect = data["special_effect"].as<int>();}
  if(data["whitebal"] != nullptr){mySettings.whitebal = data["whitebal"].as<int>();}
  if(data["awb_gain"] != nullptr){mySettings.awb_gain = data["awb_gain"].as<int>();}
  if(data["wb_mode"] != nullptr){mySettings.wb_mode = data["wb_mode"].as<int>();}
  if(data["exposure_ctrl"] != nullptr){mySettings.exposure_ctrl = data["exposure_ctrl"].as<int>();}
  if(data["aec2"] != nullptr){mySettings.aec2 = data["aec2"].as<int>();}
  if(data["ae_level"] != nullptr){mySettings.ae_level = data["ae_level"].as<int>();}
  if(data["aec_value"] != nullptr){mySettings.aec_value = data["aec_value"].as<int>();}
  if(data["gain_ctrl"] != nullptr){mySettings.gain_ctrl = data["gain_ctrl"].as<int>();}
  if(data["agc_gain"] != nullptr){mySettings.agc_gain = data["agc_gain"].as<int>();}
  if(data["gainceiling"] != nullptr){mySettings.gainceiling = data["gainceiling"].as<gainceiling_t>();}
  if(data["bpc"] != nullptr){mySettings.bpc = data["bpc"].as<int>();}
  if(data["wpc"] != nullptr){mySettings.wpc = data["wpc"].as<int>();}
  if(data["raw_gma"] != nullptr){mySettings.raw_gma = data["raw_gma"].as<int>();}
  if(data["lenc"] != nullptr){mySettings.lenc = data["lenc"].as<int>();}
  if(data["hmirror"] != nullptr){mySettings.hmirror = data["hmirror"].as<int>();}
  if(data["vflip"] != nullptr){mySettings.vflip = data["vflip"].as<int>();}
  if(data["lenc"] != nullptr){mySettings.lenc = data["lenc"].as<int>();}
  if(data["dcw"] != nullptr){mySettings.dcw = data["dcw"].as<int>();}
  if(data["colorbar"] != nullptr){mySettings.colorbar = data["colorbar"].as<int>();}

  mySettings.lastUpdated = millis();
  return callbackResponse("sharedAttributesUpdate", 1);
}

void syncClientAttributes()
{
  StaticJsonDocument<DOCSIZE_MIN> doc;

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
  doc["intvDevTel"] = mySettings.intvDevTel;
  doc["intvSnap"] = mySettings.intvSnap;
  doc["frameSize"] = mySettings.frameSize;
  doc["jpegQuality"] = mySettings.jpegQuality;
  doc["tempBuffSize"] = mySettings.tempBuffSize;
  tb.sendAttributeDoc(doc);
  doc.clear();

  doc["brightness"] = mySettings.brightness;
  doc["contrast"] = mySettings.contrast;
  doc["saturation"] = mySettings.saturation;
  doc["special_effect"] = mySettings.special_effect;
  doc["whitebal"] = mySettings.whitebal;
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["awb_gain"] = mySettings.awb_gain;
  doc["wb_mode"] = mySettings.wb_mode;
  doc["exposure_ctrl"] = mySettings.exposure_ctrl;
  doc["aec2"] = mySettings.aec2;
  doc["ae_level"] = mySettings.ae_level;
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["aec_value"] = mySettings.aec_value;
  doc["gain_ctrl"] = mySettings.gain_ctrl;
  doc["agc_gain"] = mySettings.agc_gain;
  doc["gainceiling"] = mySettings.gainceiling;
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["bpc"] = mySettings.bpc;
  doc["wpc"] = mySettings.wpc;
  doc["raw_gma"] = mySettings.raw_gma;
  doc["lenc"] = mySettings.lenc;
  tb.sendAttributeDoc(doc);
  doc.clear();
  doc["hmirror"] = mySettings.hmirror;
  doc["vflip"] = mySettings.vflip;
  doc["dcw"] = mySettings.dcw;
  doc["colorbar"] = mySettings.colorbar;
  tb.sendAttributeDoc(doc);
  doc.clear();
}

void publishDeviceTelemetry()
{
  StaticJsonDocument<DOCSIZE_MIN> doc;

  doc["heap"] = heap_caps_get_free_size(MALLOC_CAP_32BIT);
  doc["rssi"] = WiFi.RSSI();
  doc["uptime"] = millis()/1000;
  tb.sendTelemetryDoc(doc);
  doc.clear();
}

void snap()
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

    log_manager->debug(PSTR(__func__),PSTR("Taking snap (jpegQuality: %d, frameSize: %d), please wait.\n"), cam.jpeg_quality, (int)cam.frame_size);

    // camera init
    esp_err_t err = esp_camera_init(&cam);
    if (err != ESP_OK) {
      log_manager->error(PSTR(__func__),PSTR("Camera init failed with error 0x%x\n"), err);
      esp_camera_deinit();
    }
    sensor_t * s = esp_camera_sensor_get();

    s->set_brightness(s, mySettings.brightness);     // -2 to 2
    s->set_contrast(s, mySettings.contrast);       // -2 to 2
    s->set_saturation(s, mySettings.saturation);     // -2 to 2
    s->set_special_effect(s, mySettings.special_effect); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, mySettings.whitebal);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, mySettings.awb_gain);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, mySettings.wb_mode);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, mySettings.exposure_ctrl);  // 0 = disable , 1 = enable
    s->set_aec2(s, mySettings.aec2);           // 0 = disable , 1 = enable
    s->set_ae_level(s, mySettings.ae_level);       // -2 to 2
    s->set_aec_value(s, mySettings.aec_value);    // 0 to 1200
    s->set_gain_ctrl(s, mySettings.gain_ctrl);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, mySettings.agc_gain);       // 0 to 30
    s->set_gainceiling(s, mySettings.gainceiling);  // 0 to 6
    s->set_bpc(s, mySettings.bpc);            // 0 = disable , 1 = enable
    s->set_wpc(s, mySettings.wpc);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, mySettings.raw_gma);        // 0 = disable , 1 = enable
    s->set_lenc(s, mySettings.lenc);           // 0 = disable , 1 = enable
    s->set_hmirror(s, mySettings.hmirror);        // 0 = disable , 1 = enable
    s->set_vflip(s, mySettings.vflip);          // 0 = disable , 1 = enable
    s->set_dcw(s, mySettings.dcw);            // 0 = disable , 1 = enable
    s->set_colorbar(s, mySettings.colorbar);       // 0 = disable , 1 = enable

    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();

    if (!fb) {
      log_manager->error(PSTR(__func__),PSTR("Camera capture failed.\n"));
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

      log_manager->debug(PSTR(__func__),PSTR("Snap size %d.  Memory: %d/%d - %d. Sending now...\n"), finalDataSize, heap_caps_get_free_size(MALLOC_CAP_32BIT), ESP.getPsramSize(), ESP.getFreeHeap());

      const char* topic = "v1/devices/me/attributes";
      publishMqtt(topic, data, finalDataSize);
      free(data);
    }
    esp_camera_fb_return(fb);
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

  log_manager->debug(PSTR(__func__),PSTR("Finished: (size %d bytes, %d bytes written in %ld ms)\n\n"), len, len-to_write, millis()-start_ts);
}
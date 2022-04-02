/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Firmware for Vanilla UDAWA Board (starter firmware)
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#ifndef main_h
#define main_h

#define DOCSIZE 1500
#define MQTT_MAX_PACKET_SIZE 10000
#include <libudawa.h>
#include <TaskManagerIO.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <base64.h>
#include <libb64/cencode.h>

#define CURRENT_FIRMWARE_TITLE "UDAWA-Sudarsan"
#define CURRENT_FIRMWARE_VERSION "0.0.1"

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* settingsPath = "/settings.json";

struct Settings
{
    unsigned long lastUpdated;
    bool fTeleDev;
    unsigned long publishInterval;
    unsigned long myTaskInterval;
};

callbackResponse processSaveConfig(const callbackData &data);
callbackResponse processSaveSettings(const callbackData &data);
callbackResponse processSharedAttributesUpdate(const callbackData &data);
callbackResponse processSyncClientAttributes(const callbackData &data);
callbackResponse processReboot(const callbackData &data);

void loadSettings();
void saveSettings();
void syncClientAttributes();
void publishDeviceTelemetry();
void myTask();

#endif
#ifndef PARAMS_H
#define PARAMS_H

#define SERIAL_BAUD_RATE 115200
#define COMPILED __DATE__ " " __TIME__
#define CURRENT_FIRMWARE_TITLE "Generic"
#define CURRENT_FIRMWARE_VERSION "0.1.0"

#define USE_WIFI_OTA

#define USE_LOCAL_WEB_INTERFACE
#ifdef USE_LOCAL_WEB_INTERFACE
#define WS_BLOCKED_DURATION 60000U
#define WS_RATE_LIMIT_INTERVAL 1000U
#endif

#define USE_IOT
#ifdef USE_IOT
#define USE_IOT_SECURE
#define IOT_MAX_MESSAGE_SIZE 2048U
#define IOT_STACKSIZE_TB 6000U
#define THINGSBOARD_ENABLE_DYNAMIC true
#endif

#define USE_WIFI_LOGGER
#ifdef USE_WIFI_LOGGER
#define WIFI_LOGGER_BUFFER_SIZE 256U
#endif

const int tbPort = 8883;
constexpr char tbAddr[] PROGMEM = "prita.undiknas.ac.id";
constexpr char spiffsBinUrl[] PROGMEM = "http://prita.undiknas.ac.id/cdn/firmware/sudarsan.spiffs.bin";
constexpr char model[] PROGMEM = "Generic";
constexpr char group[] PROGMEM = "UDAWA";
constexpr char logIP[] PROGMEM = "255.255.255.255";
const uint8_t logLev = 5;
const bool fIoT = true;
const bool fWOTA = true;
const bool fWeb = true;
const int gmtOff = 28880;
const uint16_t logPort = 29514;


#endif
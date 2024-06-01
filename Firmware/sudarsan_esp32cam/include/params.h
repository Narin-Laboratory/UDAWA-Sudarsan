#ifndef PARAMS_H
#define PARAMS_H

#define SERIAL_BAUD_RATE 115200
#define COMPILED __DATE__ " " __TIME__
#define CURRENT_FIRMWARE_TITLE "Generic"
#define CURRENT_FIRMWARE_VERSION "0.1.0"

#define USE_WIFI_OTA

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
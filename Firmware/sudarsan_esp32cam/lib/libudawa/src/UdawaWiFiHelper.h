#ifndef UdawaWiFiHelper_h
#define UdawaWiFiHelper_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "UdawaLogger.h"
#include <vector>

typedef void (*WiFiConnectedCallback)(); 
typedef void (*WiFiDisconnectedCallback)(); 
typedef void (*WiFiGotIPCallback)(); 

class UdawaWiFiHelper{
    public:
        UdawaWiFiHelper(const char* wssid, const char* wpass,
            const char* dssid, const char* dpass, const char* hname);
        void addOnConnectedCallback(WiFiConnectedCallback callback);
        void addOnDisconnectedCallback(WiFiDisconnectedCallback callback);
        void addOnGotIPCallback(WiFiGotIPCallback callback);
        void begin();
        void run();
    private:
        WiFiMulti _wiFi;
        UdawaLogger *_logger = UdawaLogger::getInstance(LogLevel::VERBOSE);
        const char* _wssid;
        const char* _wpass;
        const char* _dssid;
        const char* _dpass;
        const char* _hname;
        void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
        std::vector<WiFiConnectedCallback> _onConnectedCallbacks;
        std::vector<WiFiDisconnectedCallback> _onDisconnectedCallbacks;
        std::vector<WiFiGotIPCallback> _onGotIPCallbacks;
};

#endif
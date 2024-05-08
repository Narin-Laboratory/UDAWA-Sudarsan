#ifndef Udawa_h
#define Udawa_h
#include "UdawaSerialLogger.h"
#include "UdawaLogger.h"
#include "UdawaWiFiHelper.h"

class Udawa {
    public:
        Udawa(uint32_t baudRate);
        void run();
    private:
        UdawaSerialLogger _serialLogger;
        UdawaLogger *_logger = UdawaLogger::getInstance(LogLevel::VERBOSE);
        UdawaWiFiHelper _wiFiHelper;
        void _onWiFiConnected();
        void _onWiFiDisconnected();
        void _onWiFiGotIP();
};

#endif
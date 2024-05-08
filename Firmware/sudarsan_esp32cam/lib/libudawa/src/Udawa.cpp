#include "Udawa.h"

Udawa::Udawa(uint32_t baudRate) : _serialLogger(baudRate), _wiFiHelper("Govinda", "goloka8881", "UDAWA", "defaultkey", "sudarsan") {
    _logger->addLogger(&_serialLogger);
    _logger->setLogLevel(LogLevel::VERBOSE);
    
}

void Udawa::run(){
    _wiFiHelper.begin();
    _wiFiHelper.addOnConnectedCallback([this](){
        this->_onWiFiConnected();
    });
    _wiFiHelper.run();
}

void Udawa::_onWiFiConnected(){}
void Udawa::_onWiFiDisconnected(){}
void Udawa::_onWiFiGotIP(){}
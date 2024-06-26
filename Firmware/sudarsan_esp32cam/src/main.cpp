#include "main.h"

Udawa udawa;

void setup() {
  #ifdef USE_LOCAL_WEB_INTERFACE
  udawa.addOnWsEvent(_onWsEvent);
  #endif
  #ifdef USE_IOT
  udawa.addOnThingsboardSharedAttributesReceived(_processThingsboardSharedAttributesUpdate);
  #endif
  udawa.begin();
}

unsigned long timer = millis();
void loop() {
  udawa.run();

  if(millis() - timer > 1000){
    udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), ESP.getFreeHeap());

    timer = millis();
  }
}

#ifdef USE_LOCAL_WEB_INTERFACE
void _onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
   
  }
  else if(type == WS_EVT_DATA){
 
  }
}
#endif

#ifdef USE_IOT
void _processThingsboardSharedAttributesUpdate(const JsonObjectConst &data){
  String _data;
  serializeJson(data, _data);
  udawa.logger->debug(PSTR(__func__), PSTR("From main.cpp %s\n"), _data.c_str());
}
#endif
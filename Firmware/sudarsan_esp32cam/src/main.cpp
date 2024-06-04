#include <Arduino.h>
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "Udawa.h"

Udawa udawa;

void _onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

void setup() {
  udawa.addOnWsEvent(_onWsEvent);
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

void _onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    udawa.logger->debug(PSTR(__func__),PSTR("ws[%s][%u] connect\n"), server->url(), client->id());
    client->printf("Hello from main.cpp, Client %u :)", client->id());
    client->ping();
  }
  else if(type == WS_EVT_DATA){
    JsonDocument doc;
    deserializeJson(doc, data);
    serializeJsonPretty(doc, Serial);
  }
}
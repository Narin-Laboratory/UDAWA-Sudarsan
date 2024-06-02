#include <Arduino.h>
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "Udawa.h"
#include <TaskScheduler.h>
#ifdef USE_LOCAL_WEB_INTERFACE
#include <Crypto.h>
#include <SHA256.h>
#include "mbedtls/md.h"
#include <map>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif

Udawa udawa;

void t1Callback();
void _onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

//Tasks
Task t1(2000, TASK_FOREVER, &t1Callback);


Scheduler runner;

void setup() {
  udawa.addOnWsEvent(_onWsEvent);
  udawa.begin();

  runner.addTask(t1);
  t1.enable();
}

void loop() {
  udawa.run();
  runner.execute();
}

void t1Callback(){
  udawa.logger->debug(PSTR(__func__), PSTR("%d\n"), ESP.getFreeHeap());
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
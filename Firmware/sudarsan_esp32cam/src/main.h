#include <Arduino.h>
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "Udawa.h"

void _onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

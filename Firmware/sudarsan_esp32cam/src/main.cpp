#include <Arduino.h>
#include <WiFi.h>
#include <UdawaLogger.h>
#include <secret.h>
#include <Udawa.h>

Udawa udawa(115200);
UdawaLogger *logger = UdawaLogger::getInstance(LogLevel::VERBOSE);


void setup() {
  
}

void loop() {
  logger->error("blah", "blah\n");
  delay(1000);
}
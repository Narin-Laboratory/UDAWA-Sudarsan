#include <Arduino.h>
#include "UdawaLogger.h"
#include "UdawaSerialLogger.h"
#include "Udawa.h"
#include <TaskScheduler.h>

Udawa udawa;

void t1Callback();

//Tasks
Task t1(2000, TASK_FOREVER, &t1Callback);


Scheduler runner;

void setup() {
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
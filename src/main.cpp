#include <Arduino.h>
#include "motor_driver.h"
#include "robot_controller.h"
#include "system_context.h"
#include "task_webapp.h"

#define LED_PIN 2

namespace
{

robot::L298NMotorDriver motorDriver;
robot::RobotController robotController(motorDriver);
robot::SystemContext systemContext{
  &robotController,
  &motorDriver,
  nullptr,
  nullptr};

} // namespace

void Task_BlinkingLed(void *)
{
  bool on = false;
  pinMode(LED_PIN, OUTPUT);
  while (true)
  {
    if (on)
    {
      digitalWrite(LED_PIN, HIGH);
      on = false;
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
      on = true;
    }
    vTaskDelay(1000);
  }
}

void setup()
{
  Serial.begin(115200);
  motorDriver.begin();
  if (xTaskCreate(Task_WebApp,
                  "Webapp",
                  8192,
                  &systemContext,
                  0,
                  nullptr) != pdPASS)
  {
    Serial.println("Startup error: Webapp task could not be created.");
  }

  if (xTaskCreate(Task_BlinkingLed,
                  "Blinking LED",
                  1024,
                  nullptr,
                  1,
                  nullptr) != pdPASS)
  {
    Serial.println("Startup error: LED task could not be created.");
  }
}

void loop()
{
}
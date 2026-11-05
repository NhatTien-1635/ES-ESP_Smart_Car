#include <Arduino.h>
#include "task_webapp.h"

#define LED_PIN 2

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
  xTaskCreate(Task_WebApp, "Webapp", 8192, nullptr, 0, nullptr);
  xTaskCreate(Task_BlinkingLed, "Blinking LED", 1024, nullptr, 1, nullptr);
}

void loop()
{
}
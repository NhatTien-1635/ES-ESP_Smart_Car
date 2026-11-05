#ifndef TASK_WEBAPP_H
#define TASK_WEBAPP_H

#include <WiFi.h>

#include <LittleFS.h>

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "system_context.h"

#define SSID "ESP32_WebModule"
#define PASSWORD "88888888"

void Task_WebApp(void*);

#endif
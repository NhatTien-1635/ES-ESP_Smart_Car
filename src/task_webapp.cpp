#include "task_webapp.h"

static WebServer web_server(80);
static DNSServer dns_server;

static void HandleIndex();
static void HandleNotFound();
static void HandleCmd();
static void HandleTraceLine();
static void HandleTelemetry();

void Task_WebApp(void *)
{
    Serial.print("\nStarting AP Mode!\n");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASSWORD);

    dns_server.start(53, "*", WiFi.softAPIP());

    web_server.on("/", HandleIndex);
    web_server.on("/api/cmd", HandleCmd);
    web_server.on("/api/telemetry", HandleTelemetry);
    web_server.on("/api/linetrace", HandleTraceLine);
    web_server.onNotFound(HandleNotFound);
    web_server.begin();

    LittleFS.begin(true);

    Serial.println("AP mode started: " + WiFi.softAPIP().toString());

    while (true)
    {
        dns_server.processNextRequest();

        web_server.handleClient();
        vTaskDelay(1);
    }
}

static void HandleIndex()
{
    if (LittleFS.exists("/index.html"))
    {
        File file = LittleFS.open("/index.html", "r");
        web_server.streamFile(file, "text/html");
        file.close();
    }
    else
    {
        web_server.send(404, "text/plain", "File not found");
    }
}

static void HandleNotFound()
{
    web_server.sendHeader("Location", "http://escp32.setup/", true);
    web_server.send(302, "text/plain", "");
}

static void HandleCmd()
{
    String d = web_server.arg("d");
    if (d.length() == 1)
    {
        Serial.println("D-Pad command: " + d);
    }
    web_server.send(200, "text/plain", "ok");
}

static void HandleTraceLine()
{
    String on = web_server.arg("on");

    Serial.print("Traceline: ");
    Serial.print(on == "1" ? "ON" : "OFF");
    web_server.send(200, "text/plain", "ok");
}

static void HandleTelemetry()
{
    // TODO: replace with real values
    String json = "{\"speed\":0,\"battery\":7.4,\"distance\":0,\"mode\":\"Manual\",\"line\":[0,0,1,0,0]}";
    web_server.send(200, "application/json", json);
}
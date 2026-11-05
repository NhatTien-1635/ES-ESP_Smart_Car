#include "task_webapp.h"

static WebServer web_server(80);
static DNSServer dns_server;
static robot::SystemContext* system_context = nullptr;

static void HandleIndex();
static void HandleNotFound();
static void HandleCmd();
static void HandleTraceLine();
static void HandleTelemetry();

void Task_WebApp(void* parameter)
{
    system_context = static_cast<robot::SystemContext*>(parameter);
    if (system_context == nullptr || system_context->controller == nullptr)
    {
        Serial.println("Webapp startup failed: controller context is missing.");
        vTaskDelete(nullptr);
        return;
    }

    Serial.print("\nStarting AP Mode!\n");

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(SSID, PASSWORD))
    {
        Serial.println("Webapp startup failed: SoftAP could not start.");
        vTaskDelete(nullptr);
        return;
    }

    dns_server.start(53, "*", WiFi.softAPIP());

    web_server.on("/", HandleIndex);
    web_server.on("/api/cmd", HandleCmd);
    web_server.on("/api/telemetry", HandleTelemetry);
    web_server.on("/api/linetrace", HandleTraceLine);
    web_server.onNotFound(HandleNotFound);
    web_server.begin();

    if (!LittleFS.begin(true))
    {
        Serial.println("Webapp startup error: LittleFS could not mount.");
    }

    Serial.println("AP mode started: " + WiFi.softAPIP().toString());

    while (true)
    {
        dns_server.processNextRequest();

        web_server.handleClient();
        system_context->controller->tick(millis());
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
    robot::ControlCommand command;
    command.kind = robot::CommandKind::STOP;

    if (d == "F" || d == "B" || d == "L" || d == "R")
    {
        command.kind = robot::CommandKind::MOVE;
        command.hasDirection = true;
        command.hasSpeed = true;
        command.speed = 80;
        command.direction = d == "F"
                                ? robot::MotionDirection::FORWARD
                                : d == "B"
                                      ? robot::MotionDirection::BACKWARD
                                      : d == "L"
                                            ? robot::MotionDirection::LEFT
                                            : robot::MotionDirection::RIGHT;
    }
    else if (d != "S")
    {
        web_server.send(400, "text/plain", "invalid command");
        return;
    }

    const robot::CommandResult result =
        system_context->controller->handleCommand(command, millis());
    if (result.status != robot::CommandStatus::ACCEPTED)
    {
        web_server.send(409, "text/plain", "command rejected");
        return;
    }

    Serial.println("D-Pad command accepted: " + d);
    web_server.send(200, "text/plain", "ok");
}

static void HandleTraceLine()
{
    String on = web_server.arg("on");
    if (on != "0" && on != "1")
    {
        web_server.send(400, "text/plain", "invalid line-trace value");
        return;
    }

    robot::ControlCommand command;
    command.kind = robot::CommandKind::SET_MODE;
    command.mode = on == "1" ? robot::RobotMode::LINE_TRACKING
                              : robot::RobotMode::MANUAL;
    command.hasMode = true;
    const robot::CommandResult result =
        system_context->controller->handleCommand(command, millis());
    if (result.status != robot::CommandStatus::ACCEPTED)
    {
        web_server.send(409, "text/plain", "mode change rejected");
        return;
    }

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
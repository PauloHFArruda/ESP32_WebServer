#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "utils.h"
#include "SerialComunication.h"
#include "views.h"

const char *ssid = "VISITANTE";
const char *password = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

float pidParams[9] = {0}, rot[3] = {0}, pwm[4] = {0};

SerialComunication arduino;

void setPIDParamOnArduino(int selector, float val) {
    Serial2.write(MsgType::SET_PHI_KP + selector);
    Serial2.print(val);
    Serial2.write(MSG_END_CHAR);
}

void handleArduinoMsg(char msgType, float data[], int len)
{
    // Serial.print("MsgType: ");
    // Serial.write(msgType);
    // Serial.print(" data: [");
    // for (int i = 0; i < len; i++) {
    //     Serial.print(data[i]);
    //     Serial.print(", ");
    // }
    // Serial.println(']');
    switch (msgType)
    {
    case MsgType::RESP_PID_PARAMS:
        copyFloatArray(data, pidParams, 9);
        for (int i = 0; i < 3; i++) {
            ws.printfAll("%d;%.3f", i, pidParams[i]);
            ws.printfAll("%d;%.3f", i+3, pidParams[i+6]);
        }
        break;
    case MsgType::RESP_ROT:
        copyFloatArray(data, rot, 3);
        break;
    case MsgType::RESP_PWM:
        copyFloatArray(data, pwm, 4);
        break;
    case MsgType::RESP_ROT_AND_PWM:
        copyFloatArray(data, rot, 3);
        copyFloatArray(data + 3 * sizeof(float), pwm, 4);
        break;

    default:
        break;
    }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        int selector = data[0] - '0';
        float val = strToFloat((char *)data+2);

        pidParams[selector + 3] = val;
        setPIDParamOnArduino(selector + 3, val);

        if (selector < 3) {
            pidParams[selector] = val;
            setPIDParamOnArduino(selector, val);
        }

        ws.textAll((char *)data);
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf(
            "WebSocket client #%u connected from %s\n",
            client->id(),
            client->remoteIP().toString().c_str());
        for (int i = 0; i < 3; i++) {
            client->printf("%d;%.3f", i, pidParams[i]);
            client->printf("%d;%.3f", i+3, pidParams[i+6]);
        }
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void connectToWifi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());
}

void initWebSocket()
{
    connectToWifi();

    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    arduino.begin(handleArduinoMsg);

    while (pidParams[0] == 0)
    {
        Serial2.write(MsgType::GET_PID_PARAMS);
        Serial2.write(MSG_END_CHAR);
        delay(50);
        while (Serial2.available()) {
            arduino.updateBuffer(Serial2.read());
        }
        delay(100);
    }

    for (int i = 0; i < 9; i++)
    {
        Serial.print(pidParams[i]);
        Serial.print(", ");
    }
    Serial.println();

    initWebSocket();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.begin();
}

void loop()
{
    ws.cleanupClients();
    while (Serial2.available()) {
        arduino.updateBuffer(Serial2.read());
    }
}
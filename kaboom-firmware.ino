#include "Arduino.h"
#include <WiFi.h>
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

// NOTE: Serial2 uses pins 17 (TX) and 16 (RX)

// FIXME super hacky
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_TEXT:
            switch(payload[0]) {
                case 's':   // speed
                    Serial.println("starting...");
                    Serial2.write('s');   // send S to reset
                    break;
                case 'u':   // update number of modules completed
                    Serial.println("got update");
                    Serial.println((const char*) payload);
                    Serial2.write((uint8_t) atoi((const char*) payload + 1)); // FIXME is this cast okay?
                    break;
                default:
                    break;
            }
        break;
    case WStype_BIN:
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_DISCONNECTED:
        break;
    case WStype_CONNECTED:
        Serial.println("connected to websocket");
        break;
  }
}

int lastSerialMessage;
void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial2.begin(81250);   // FIXME this is dependent on the frequency we use for the FPGA
                            // this is a nice value at 65.000 MHz, but not at any others...

    WiFi.mode(WIFI_STA);
    WiFi.begin("WIFI_NAME", "WIFI_PASSWORD");

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("trying to connect...");
        delay(5000);
    }

    Serial.println("Connected to WiFi");

    webSocket.begin("35.173.125.185", 80, "/client");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);

    lastSerialMessage = 0;
}

void loop() {
    webSocket.loop();
//    Serial1.println("HI");
    if (millis() > lastSerialMessage + 100) {
//      Serial2.write(0xAD);
      lastSerialMessage = millis();
    }

    // TODO wtf
    if (Serial2.available()) {
        String message = String(Serial2.read(), DEC);
        Serial.println(message);
        webSocket.sendTXT(message);
    }
}

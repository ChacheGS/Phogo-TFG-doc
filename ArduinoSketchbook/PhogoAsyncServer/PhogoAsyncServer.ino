#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <string.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

#define DEBUG true

#include "debug.h"

const char* ssid = "Low Signal";
const char* password = "sierragador15";
// const char* ssid = "Chache Hotspot";
// const char* password = "wificarlos";
const char* mdns_hostname = "phogo";

//Globals
AsyncWebServer server(80);

#define WIFI_CONNECTION_TIMEOUT 1000 * 10 // 10 s
// Wifi Connection
void WifiConnect() {
    int connection_time = millis();
    bool connection_success = true;
    WiFi.hostname(mdns_hostname);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED && connection_success) {
        if (millis() - connection_time > WIFI_CONNECTION_TIMEOUT) {
            // we leave the loop after the timeout
            connection_success = false;
        }
        delay(100);
    }
    if (connection_success) {
        DEBUGGING("WiFi Connected. Local IP: %u.%u.%u.%u\n", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
        return;
    } else {
        DEBUGGING("WiFi Connection to '%s' timed out after ", ssid);
        DEBUGGINGL((millis() - connection_time) / 1000.0));
        DEBUGGINGL(" s\n");
    }

    // TODO: maybe retry connection?

    char APName[20] = {0};
    sprintf(APName, "Phogo%08X", ESP.getFlashChipId());
    DEBUGGING("Starting AP mode as '%s'\n", APName);
    WiFi.mode(WIFI_AP);
    // ap config
    IPAddress ip(10, 0, 0, 2);
    IPAddress gateway(10, 0, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    Wifi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(APName);

    delay(500);
    DEBUGGING("Started AP mode as '%s' [ip=", APName);
    DEBUGGINGL(WiFi.softAPIP());
    DEBUGGINGC("]\n");
}

void stop_forever() {
    while (1) {
        delay(1000);
    }
}

// mDNS
void mDNSConnect() {
    if (!MDNS.begin(mdns_hostname)) {
        DEBUGGING("Error setting up mDNS!\n");
        stop_forever();
    }
    DEBUGGING("mDNS started: 'http://%s.local/'\n", mdns_hostname);
    MDNS.addService("http", "tcp", 80);
}

void setup() {
#ifdef DEBUG
    Serial.begin(115200);
    while (!Serial) {
        // wait for initialization
    }
    // Serial.println();
#endif

    SPIFFS.begin();
    server.begin();

    WifiConnect();
    mDNSConnect();
    HTTPServerSetup();
}

int lastTimeHost;
int lastTimeRefresh;

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        // TODO: ensure_connection()
        WifiConnect();
        mDNSConnect();
    }
    
    // give the ESP a chance to do its own stuff
    delay(1000);
}

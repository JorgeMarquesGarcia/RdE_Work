#include "WIFIF.h"
#include <WiFi.h>
#include <time.h>
#include <Arduino.h>

WIFIF::WIFIF(const char* ssid, const char* password) 
    : ssid(ssid), password(password) {}


void WIFIF::connect() {
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}

bool WIFIF::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}



void WIFIF::Start() {
    connect();
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("MAC address: ");
    Serial.println(WiFi.macAddress());
}





 
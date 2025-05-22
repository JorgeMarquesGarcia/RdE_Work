#ifndef MQTT_CLIENT_MANAGER_H
#define MQTT_CLIENT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include "Estructuras.h"
#include <WiFi.h>  // Usamos WiFi directamente

class MQTTClientManager {
public:
    MQTTClientManager(const char* ssid, const char* password, const char* mqttServer, uint16_t mqttPort, const char* mqttUser, const char* mqttPassword);
    void connectWiFi();
    void connectMQTT();
    void loop();
    void publishSensorData(SensorData_t& data);
    void setTopic(const String& topic);

private:
    const char* ssid;
    const char* password;
    const char* mqttServer;
    uint16_t mqttPort;
    const char* mqttUser;
    const char* mqttPassword;
    String mqtt_topic;
    WiFiClient wifiClient; // Usamos WiFiClient para conectar a MQTT
    PubSubClient client;   // Usamos PubSubClient con el WiFiClient

    void reconnectMQTT();
    String createJSON(SensorData_t sensorData);
};

#endif // MQTT_CLIENT_MANAGER_H

#include "MQTTClientManager.h"
#include <WiFi.h>
#include <ArduinoJson.h>

MQTTClientManager::MQTTClientManager(const char* ssid, const char* password, const char* mqttServer, uint16_t mqttPort, const char* mqttUser, const char* mqttPassword)
    : ssid(ssid), password(password), mqttServer(mqttServer), mqttPort(mqttPort), mqttUser(mqttUser), mqttPassword(mqttPassword), wifiClient(), client(wifiClient) { 
    // Inicializamos el cliente de MQTT con WiFiClient
}

void MQTTClientManager::connectWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void MQTTClientManager::connectMQTT() {
    client.setServer(mqttServer, mqttPort);
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
      
        if (client.connect("ArduinoClient")) {
            Serial.println("Connected to MQTT server");
        } else {
            Serial.print("Failed to connect to MQTT, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 3 seconds");
            delay(3000);
        }
    }
}



void MQTTClientManager::loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
}


String MQTTClientManager::createJSON(SensorData_t sensorData) {
    // Creamos un objeto JSON
    StaticJsonDocument<96> doc;

    // Agregamos los datos de la estructura al objeto JSON
    doc["ax"] = sensorData.acc_x;
    doc["ay"] = sensorData.acc_y;
    doc["az"] = sensorData.acc_z;
    doc["gx"] = sensorData.gyro_x;
    doc["gy"] = sensorData.gyro_y;
    doc["gz"] = sensorData.gyro_z;

    // Convertimos el objeto JSON a una cadena
    String jsonString;
    serializeJson(doc, jsonString);

    return jsonString;
}

void MQTTClientManager::publishSensorData(SensorData_t& data) {
    String payload = createJSON(data);

    if (!client.connected()) {
        Serial.println("[MQTT Error] Client not connected");
        return;
    }

    if (mqtt_topic.length() == 0) {
        Serial.println("[MQTT Error] Topic not set");
        return;
    }

    client.publish(mqtt_topic.c_str(), payload.c_str());
}
void MQTTClientManager::setTopic(const String& topic) {
    mqtt_topic = "cf_dataset/" + topic;
    Serial.print("MQTT topic set to: ");
    Serial.println(mqtt_topic);
}



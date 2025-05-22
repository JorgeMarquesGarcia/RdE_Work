#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H
#include <Arduino.h>
#include <WIFIF.h>
#include <time.h>

typedef struct{
  float acc_x, acc_y, acc_z;
  float gyro_x, gyro_y, gyro_z;
} SensorData_t;
/*HOME*/
/*
typedef struct{
  const char* ssid = "XXXXXXXX";
  const char* password = "XXXXXXXXXX";
} WIFIF_t;
 */
/*PHONE*/

typedef struct{
  const char* ssid = "XXXXXXXXXX";
  const char* password = "XXXXXXXXXXX";
} WIFIF_t;
 /*

/*HOME*/
/*
typedef struct{
  const char* mqtt_server = "192.168.1.39";
  const int mqtt_port = 1883;
  const char* mqtt_user = "";
  const char* mqtt_pass = "";
  const char* mqtt_client_id = "ArduinoClient";
  const char* mqtt_topic = "cf_dataset/prueba60HZ";
} MQTT_t;

/*PHONE*/
typedef struct{
  const char* mqtt_server = "172.20.10.3";
  const int mqtt_port = 1883;
  const char* mqtt_user = "";
  const char* mqtt_pass = "";
  const char* mqtt_client_id = "ArduinoClient";
  const char* mqtt_topic = "cf_dataset/prueba60HZ";
} MQTT_t;


typedef struct{
  const char* serviceUUID = "0dd54ba6-6456-4cf9-87d3-e2b3319cb5d1";
  const char* charUUID = "0dd54ba7-6456-4cf9-87d3-e2b3319cb5d1";
} BLE_t;


extern bool DataReady;
extern QueueHandle_t dataQueue;


#endif // ESTRUCTURAS_H
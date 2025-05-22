#include <Arduino.h>
#include "WIFIF.h"
#include "BLEread.h"
#include "Estructuras.h"
#include "MQTTClientManager.h"
bool isDataChanged(SensorData_t newData);

/// Inicializamos las variables de configuración de BLE y FTP
/// y las variables de datos de los sensores
BLE_t bleConfig;
SensorData_t sensorData;
WIFIF_t wifiConfig;
MQTT_t mqttConfig;
BLE esp32(bleConfig.serviceUUID, bleConfig.charUUID);
MQTTClientManager mqttClient(wifiConfig.ssid, wifiConfig.password, mqttConfig.mqtt_server, mqttConfig.mqtt_port, mqttConfig.mqtt_user, mqttConfig.mqtt_pass);
//WIFIF wifi(wifiConfig.ssid, wifiConfig.password);

bool DataReady = true; // Variable para indicar si hay datos listos
/// Inicializamos las tareas 
TaskHandle_t TaskBLE;
TaskHandle_t TaskMQTT;
QueueHandle_t dataQueue; //cola para pasar datos entre tareas

void BLEtask(void *pvParameters);
void MQTTtask(void *pvParameters);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Configurando Wifi...");
  mqttClient.connectWiFi();  
  //wifi.Start();
  Serial.println("MQTT setup started");
  mqttClient.connectMQTT();
  Serial.println("MQTT setup complete");
  delay(500);
  Serial.println("Enter MQTT topic:");
  while (!Serial.available()) {
      delay(100);
  }
  String userTopic = Serial.readStringUntil('\n');
  userTopic.trim();
  mqttClient.setTopic(userTopic);
  Serial.println("Starting BLE connection...");
  esp32.setup();
  Serial.println("Setup complete.");
  delay(5000);
  // Inicializar cola
  dataQueue = xQueueCreate(10, sizeof(SensorData_t)); //tamaño de cola aumentado para evitar overflow
  if (dataQueue == NULL) {
    Serial.println("[Setup] ¡Error al crear la cola!");
    while (true); // Detener ejecución
  } else {
    Serial.println("[Setup] Cola creada correctamente");
  }

  xTaskCreatePinnedToCore(BLEtask,   /* Task function. */
                      "BLE reader", /* name of task. */
                      4096,     /* Stack size of task */
                      NULL,      /* parameter of the task */
                      2,         /* priority of the task */
                      &TaskBLE,  /* Task handle to keep track of created task */
                      0);        /* pin task to core 0 */ 

  xTaskCreatePinnedToCore(
                      MQTTtask,   // Task function. 
                      "MQTT Sender", // name of task. 
                      4096,     // Stack size of task 
                      NULL,      // parameter of the task 
                      2,         // priority of the task 
                      &TaskMQTT,  // Task handle to keep track of created task 
                      1);        // pin task to core 1 


}
void loop() {
}

/*
void BLEtask(void *pvParameters) {
  while (true) {
    //if (DataReady) {
    //Serial.println("Entrando en tarea BLE");
      esp32.loop(); // Llamar a la función loop() de BLE
      sensorData = esp32.getSensorData();
    if (xQueueSend(dataQueue, &sensorData, portMAX_DELAY) == pdTRUE) {
      //Serial.println("Datos enviados a la cola.");
      DataReady = true;  // Se indica que hay datos nuevos
    }
   // }
    vTaskDelay(pdMS_TO_TICKS(10));  // Más rápido para reaccionar en cuanto haya datos
  }
}

//Con esto la conexion me echa porque el data rate es demasiado alto, adafruit limita a 1 mensaje por segundo
void MQTTtask(void *pvParameters) {
  SensorData_t received;
  while (true) {
    mqttClient.loop(); // Llamar a la función loop() de MQTT
    if(xQueueReceive(dataQueue, &received, portMAX_DELAY) == pdTRUE){
      //TickType_t start = xTaskGetTickCount();
      Serial.println(received.acc_x);
      mqttClient.publishSensorData(received);
      //TickType_t end = xTaskGetTickCount();
      //Serial.printf("MQTT send took %lu ms\n", (end - start) * portTICK_PERIOD_MS);
    }
  vTaskDelay(pdMS_TO_TICKS(50));  // Más rápido para reaccionar en cuanto haya datos

  }
}*/


void BLEtask(void *pvParameters) {
  while (true) {
    esp32.loop(); // Llamar a la función loop() de BLE
    sensorData = esp32.getSensorData();  // Obtener datos del sensor
    
    // Aquí se podría hacer una comparación para evitar enviar datos repetidos
    if (isDataChanged(sensorData)) {
      if (xQueueSend(dataQueue, &sensorData, portMAX_DELAY) == pdTRUE) {
        //Serial.println("Datos nuevos enviados a la cola.");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));  // Esperar un poco para no sobrecargar la CPU
  }
}

void MQTTtask(void *pvParameters) {
  SensorData_t received;
  static SensorData_t lastPublishedData = {0};  // Últimos datos enviados a MQTT

  while (true) {
    mqttClient.loop();  // Llamar a la función loop() de MQTT
    TickType_t start = xTaskGetTickCount();

    // Verificar si hay nuevos datos en la cola
    if (xQueueReceive(dataQueue, &received, portMAX_DELAY) == pdTRUE) {
      // Solo publicamos si los datos son diferentes a los últimos publicados
      if (memcmp(&received, &lastPublishedData, sizeof(SensorData_t)) != 0) {
        //Serial.println("Publicando nuevos datos en MQTT...");
        mqttClient.publishSensorData(received);  // Publicar los datos en MQTT
        lastPublishedData = received;  // Actualizamos los últimos datos enviados
      } else {
        //Serial.println("Datos ya publicados, no enviando.");
      }
      TickType_t end = xTaskGetTickCount();
      Serial.printf("MQTT send took %lu ms\n", (end - start) * portTICK_PERIOD_MS);
    }
    vTaskDelay(pdMS_TO_TICKS(5));  // Como es MQTT podemos ir a todo trapo de hecho a 60 Hz vamos bastante justos
  }
}

bool isDataChanged(SensorData_t newData) {
  static SensorData_t lastData = {0};  // Guardamos los últimos datos enviados
  if (memcmp(&newData, &lastData, sizeof(SensorData_t)) != 0) {
    lastData = newData;  // Actualizamos los datos previos si son diferentes
    return true;
  }
  return false;  // Si los datos no han cambiado, no se envían a la cola
}
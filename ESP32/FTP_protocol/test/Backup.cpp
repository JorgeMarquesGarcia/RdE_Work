#include <Arduino.h>
#include "BLEread.h"
#include "Estructuras.h"
/*Checked, queue shows the previuous sensordata send*/


// Configuración BLE
BLE_t bleConfig;
BLE esp32(bleConfig.serviceUUID, bleConfig.charUUID);

// Cola y estructura de datos
QueueHandle_t dataQueue;
SensorData_t sensorData;

// Tareas y handles
TaskHandle_t TaskBLE;
TaskHandle_t TaskDebug;

// Prototipos
void bleTask(void *pvParameters);
void debugTask(void *pvParameters);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n[Setup] Iniciando sistema...");
  Serial.println("Starting BLE connection...");
  esp32.setup();
  Serial.println("Setup complete.");
  // Inicializar cola
  dataQueue = xQueueCreate(10, sizeof(SensorData_t));
  if (dataQueue == NULL) {
    Serial.println("[Setup] ¡Error al crear la cola!");
    while (true); // Detener ejecución
  } else {
    Serial.println("[Setup] Cola creada correctamente");
  }

  // Crear tarea BLE (Core 0)
  xTaskCreatePinnedToCore(
    bleTask,
    "BLE Reader Task",
    10000,
    NULL,
    1,
    &TaskBLE,
    0
  );

  // Crear tarea Debug (Core 1)
  xTaskCreatePinnedToCore(
    debugTask,
    "Debug Reader Task",
    10000,
    NULL,
    1,
    &TaskDebug,
    1
  );
}

void loop() {

  
}

void bleTask(void *pvParameters) {
  while (true) {
    Serial.println("[BLEtask] Esperando datos...");
    esp32.loop();  // Procesa BLE
    sensorData = esp32.getSensorData();  // Leer datos

    // Verificamos si recibimos algo válido (puedes personalizar esto)
    
    Serial.println("[BLEtask] Enviando datos a la cola...");
    xQueueSend(dataQueue, &sensorData, portMAX_DELAY);
  

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void debugTask(void *pvParameters) {
  SensorData_t received;
  while (true) {
    if (xQueueReceive(dataQueue, &received, portMAX_DELAY) == pdTRUE) {
      Serial.println("[DebugTask] Datos recibidos desde la cola:");
      Serial.printf("  ACC: %.2f %.2f %.2f\n", received.acc_x, received.acc_y, received.acc_z);
      Serial.printf("  GYRO: %.2f %.2f %.2f\n", received.gyro_x, received.gyro_y, received.gyro_z);
      Serial.println("--------------------------------------------------");
    }
  }
}

#include <Arduino.h>
#include "WIFIF.h"
#include "BLEread.h"
#include "Estructuras.h"
#include "FTPsend.h"

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

/// Inicializamos las variables de configuración de BLE y FTP
/// y las variables de datos de los sensores
BLE_t bleConfig;
FTP_t ftpConfig;
SensorData_t sensorData;
WIFIF_t wifiConfig;
BLE esp32(bleConfig.serviceUUID, bleConfig.charUUID);
WIFIF wifi(wifiConfig.ssid, wifiConfig.password);
FTPsend FTP(ftpConfig.ftp_server, ftpConfig.port, ftpConfig.ftp_user, ftpConfig.ftp_pass);

bool DataReady = true; // Variable para indicar si hay datos listos
/// Inicializamos las tareas 
TaskHandle_t TaskBLE;
TaskHandle_t TaskFTP;
QueueHandle_t dataQueue; //cola para pasar datos entre tareas

void BLEtask(void *pvParameters);
void FTPtask(void *pvParameters);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Configurando Wifi...");
  wifi.Start();
  Serial.println("FTP setup started");
  FTP.setup();
  Serial.println("FTP setup complete");
  delay(500);
  Serial.println("Starting BLE connection...");
  esp32.setup();
  Serial.println("Setup complete.");
  delay(5000);
  // Inicializar cola
  dataQueue = xQueueCreate(100, sizeof(SensorData_t)); //tamaño de cola aumentado para evitar overflow
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
                      FTPtask,   // Task function. 
                      "FTP Sender", // name of task. 
                      4096,     // Stack size of task 
                      NULL,      // parameter of the task 
                      1,         // priority of the task 
                      &TaskFTP,  // Task handle to keep track of created task 
                      1);        // pin task to core 1 


}
void loop() {
}


void BLEtask(void *pvParameters) {
  
  while (true) {
    if (DataReady) {
    //Serial.println("Entrando en tarea BLE");
      esp32.loop(); // Llamar a la función loop() de BLE
      sensorData = esp32.getSensorData();
      xQueueSend(dataQueue, &sensorData, portMAX_DELAY);
      DataReady = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));  // Más rápido para reaccionar en cuanto haya datos
  }
}

/****
[FTPsend]Data out
FTP send took 1054 ms
[FTPsend]Data out                   Por lo tanto ftp no me da para mandar los datos en tiempo real
FTP send took 1143 ms
****/
void FTPtask(void *pvParameters) {
  SensorData_t received;
  while (true) {
    if (xQueueReceive(dataQueue, &received, portMAX_DELAY) == pdTRUE) {
      TickType_t start = xTaskGetTickCount();
      //Serial.println("Entrando en tarea FTP");

      FTP.sendData(received); // Enviamos los datos a la FTP
      TickType_t end = xTaskGetTickCount();
      Serial.printf("FTP send took %lu ms\n", (end - start) * portTICK_PERIOD_MS);

    }
  }
  vTaskDelay(pdMS_TO_TICKS(10)); 
}


/*
#define BATCH_SIZE 10

void FTPtask(void *pvParameters) {
  SensorData_t batch[BATCH_SIZE];
  int index = 0;

  while (true) {
    // Recibe un dato con timeout de 100ms, así evitamos WDT
    if (xQueueReceive(dataQueue, &batch[index], pdMS_TO_TICKS(100)) == pdTRUE) {
      index++;
    }

    if (index >= BATCH_SIZE) {
      TickType_t start = xTaskGetTickCount();

      // Envía el batch por FTP (debes implementar bien esta función)
      FTP.sendDataBatch(batch, BATCH_SIZE);

      TickType_t end = xTaskGetTickCount();
      Serial.printf("FTP batch took %lu ms\n", (end - start) * portTICK_PERIOD_MS);

      index = 0;  // Reiniciamos el índice
    }

    // Ceder un poco de tiempo al sistema
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

*/


#include "BLEread.h"
#include "Estructuras.h"
#include <Arduino.h>
#include <BLEDevice.h>

SensorData_t BLE::sensorData;  // Definir la variable estática fuera de la clase

BLE::BLE(const char* serviceUUID, const char* charUUID)
    : serviceUUID(serviceUUID), charUUID(charUUID), pClient(nullptr), connected(false), doConnect(false), doScan(false) {}


void BLE::notifyCallback(
    BLERemoteCharacteristic* pBLERemoteCharacteristic,
    uint8_t* pData,
    size_t length,
    bool isNotify) {
   // SensorData_t SensorData;  // Crear una instancia de SensorData_t para almacenar los datos recibidos
    if (length == sizeof(SensorData_t)) {
        memcpy(&sensorData, pData, sizeof(SensorData_t));  // Asumiendo que SensorData_t es la estructura de datos
        //DataReady = true;  // Datos listos
       // Serial.print("[notifyCallback]"); Serial.println("Data in");

    } else {
        Serial.print("Tamaño inesperado: ");
        Serial.println(length);
    }
}


void BLE::MyClientCallback::onConnect(BLEClient* pclient) {
    Serial.println("Conectado al servidor BLE.");
    pclient->setMTU(36);
}

void BLE::MyClientCallback::onDisconnect(BLEClient* pclient) {
    Serial.println("Desconectado del servidor BLE.");
    parent->connected = false;
    parent->doScan = true;
}

void BLE::MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Dispositivo BLE encontrado: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(parent->serviceUUID)) {
        BLEDevice::getScan()->stop();
        parent->myDevice = new BLEAdvertisedDevice(advertisedDevice);
        parent->doConnect = true;
        parent->doScan = true;
    }
}

bool BLE::connectToServer() {
    if (pClient == nullptr) {
        pClient = BLEDevice::createClient();
        pClient->setClientCallbacks(new MyClientCallback(this));
    }

    if (pClient->isConnected()) {
        pClient->disconnect();
        delay(100);
    }

    pClient->connect(myDevice);
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        pClient->disconnect();
        return false;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        pClient->disconnect();
        return false;
    }

    if (pRemoteCharacteristic->canNotify()) {
        pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    connected = true;
    return true;
}

void BLE::setup() {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void BLE::loop() {
    if (doConnect) {
        if (connectToServer()) {
            Serial.println("Conectado al servidor BLE.");
        } else {
            Serial.println("Error al conectar al servidor BLE.");
        }
        doConnect = false;
    }

    if (!connected && doScan) {
        Serial.println("Iniciando escaneo BLE...");
        BLEDevice::getScan()->start(5, false);  // escaneo no bloqueante por 5s
        doScan = false;
    }

    vTaskDelay(pdMS_TO_TICKS(10));  // reemplazo de delay()
}


SensorData_t BLE::getSensorData() {
    return sensorData;
}

void BLE::startNotifications() {
    if (connected && pRemoteCharacteristic) {
        pRemoteCharacteristic->registerForNotify(notifyCallback);
        Serial.println("Notificaciones activadas.");
    }
}

void BLE::stopNotifications() {
    if (connected && pRemoteCharacteristic) {
        pRemoteCharacteristic->registerForNotify(nullptr);
        Serial.println("Notificaciones canceladas.");
    }
}
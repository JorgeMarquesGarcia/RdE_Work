#ifndef BLUETOOTHREAD_H
#define BLUETOOTHREAD_H

#include "BLEDevice.h"
#include "Estructuras.h"
#include <Arduino.h>
#include <BLEClient.h>
#include <time.h>
#include <WIFIF.h>

// Clase BLE para manejar la conexión y lectura de datos por Bluetooth
class BLE {
private:
    BLEUUID serviceUUID;
    BLEUUID charUUID;
    static SensorData_t sensorData;
    BLERemoteCharacteristic* pRemoteCharacteristic;
    BLEAdvertisedDevice* myDevice;
    BLEClient* pClient;
    bool connected;
    bool doConnect;
    bool doScan;

    // Callback para manejar notificaciones
    static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

    // Callback para manejar eventos de cliente
    class MyClientCallback : public BLEClientCallbacks {
        private:
            BLE* parent; // Puntero a la clase BLE
        public:
            MyClientCallback(BLE* parentInstance) : parent(parentInstance) {}
        
            void onConnect(BLEClient* pclient) override;
            void onDisconnect(BLEClient* pclient) override;
        };

    // Callback para manejar dispositivos anunciados
    class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    public:
        BLE* parent;
        MyAdvertisedDeviceCallbacks(BLE* parentInstance) : parent(parentInstance) {}
        void onResult(BLEAdvertisedDevice advertisedDevice);
    };

    bool connectToServer();

public:
    BLE(const char* serviceUUID, const char* charUUID);
    void setup();
    void loop();
    SensorData_t getSensorData();
    void startNotifications();
    void stopNotifications();

};

#endif // BLUETOOTHREAD_H
#ifndef FTPSEND_H
#define FTPSEND_H
#include <Arduino.h>
#include <WiFi.h>
#include <ESP32_FTPClient.h>
#include "WIFIF.h"  
#include "Estructuras.h"  

class FTPsend {
public:
    FTPsend(const char* ftp_server, int port, const char* ftp_user, const char* ftp_pass);
    void setup();
    void sendData(SensorData_t sensorData);
    void closeConnection();    
    void sendDataBatch(SensorData_t* data, size_t size);
private:
    ESP32_FTPClient ftp;  
    String filename; 

};



#endif
// FTPSEND_H
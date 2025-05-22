#ifndef WIFIF_H
#define WIFIF_H
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

class WIFIF {
public:
    WIFIF(const char* ssid, const char* password);
    void Start();

private:
    const char* ssid;
    const char* password;
    void connect();
    bool isConnected();
    
};

#endif // WIFI12F_H
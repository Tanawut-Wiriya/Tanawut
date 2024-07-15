#ifndef MODULE_H
#define MODULE_H

/*
WiFi 3 Mode
1 -> AP (Access Point)
2 -> STA (Station)
3 -> APSTA (Access Point and Station)
*/

#include <Arduino.h>

#include <WiFi.h>

extern const char* ssid;
extern const char* password;

void setup_wifi();
void setup_wifiAP();
#endif
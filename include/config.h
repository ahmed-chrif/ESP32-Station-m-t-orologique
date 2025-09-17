#ifndef CONFIG_H
#define CONFIG_H

// Configuration des broches
#define DHT_PIN 15
#define LED_PIN 2

// Configuration WiFi
const char *AP_SSID = "Mon_ESP32";
const char *AP_PASSWORD = "12345678";

// Configuration serveur web
#define WEB_SERVER_PORT 80

// Configuration NTP
#define NTP_SERVER "pool.ntp.org"
#define TIME_OFFSET 3600 // UTC+1 (Paris)
#define UPDATE_INTERVAL 60000

#endif
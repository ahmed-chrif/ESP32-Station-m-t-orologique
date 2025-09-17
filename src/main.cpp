#include <WiFi.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHTesp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);
const int LED_PIN = 2;
const char *ap_ssid = "Mon_ESP32";
const char *ap_password = "12345678";
WebServer server(80);

// Capteur DHT
const int DHT_PIN = 15;
DHTesp dhtSensor;

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables pour les données
float temperature = 0;
float humidity = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Initialisation DHT
    dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

    // Initialisation LCD
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    lcd.print("Initialisation...");
    delay(1000);
    lcd.clear();

    // Initialisation LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println("Démarrage...");

    // Démarrage du mode AP
    startAPMode();

    // Configuration du serveur web
    setupWebServer();

    Serial.println("Prêt! Connectez-vous au WiFi: " + String(ap_ssid));
    timeClient.begin();

    // Définir le décalage horaire une seule fois dans le setup
    timeClient.setTimeOffset(3600); // UTC+1 (Paris, Madrid)
}

void loop()
{
    server.handleClient();

    // Lecture des données DHT
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    temperature = data.temperature;
    humidity = data.humidity;

    // Affichage LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + String(temperature, 1) + "C ");
    lcd.setCursor(0, 1);
    lcd.print("Hum:  " + String(humidity, 1) + "%   ");

    // Mise à jour NTP
    timeClient.update();

    Serial.print("Temp: ");
    Serial.print(temperature, 1);
    Serial.println("°C");
    Serial.print("Hum: ");
    Serial.print(humidity, 1);
    Serial.println("%");
    Serial.print("Heure: ");
    Serial.println(timeClient.getFormattedTime());
    Serial.println("---");

    delay(2000);
}

void startAPMode()
{
    Serial.println("Création du point d'accès...");
    WiFi.softAP(ap_ssid, ap_password);

    Serial.println("=== MODE AP ACTIVÉ ===");
    Serial.print("SSID: ");
    Serial.println(ap_ssid);
    Serial.print("Mot de passe: ");
    Serial.println(ap_password);
    Serial.print("IP d'accès: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("======================");
}

void setupWebServer()
{
    // Page d'accueil
    server.on("/", []()
              {
    String page = R"rawliteral(
<!DOCTYPE html><html lang="fr"><head>
    <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Station Météo ESP32</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1a2980 0%, #26d0ce 100%);
            color: #333; min-height: 100vh; padding: 20px;
            display: flex; flex-direction: column; align-items: center; justify-content: center;
        }
        .container {
            width: 100%; max-width: 1200px; margin: 0 auto;
            background: rgba(255, 255, 255, 0.95); border-radius: 20px;
            padding: 30px; box-shadow: 0 15px 30px rgba(0, 0, 0, 0.25);
        }
        header { text-align: center; margin-bottom: 30px; padding-bottom: 20px; border-bottom: 2px solid #eaeaea; }
        h1 { font-size: 2.5rem; color: #2d3748; margin-bottom: 10px; }
        .dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin-bottom: 30px; }
        .sensor-card { background: white; padding: 20px; border-radius: 15px; text-align: center; box-shadow: 0 5px 15px rgba(0,0,0,0.1); }
        .value { font-size: 2.5rem; font-weight: bold; color: #2d3748; }
        .unit { color: #718096; }
        .controls { text-align: center; margin: 20px 0; }
        button { background: #48bb78; color: white; border: none; padding: 15px 30px; margin: 5px; 
                border-radius: 10px; cursor: pointer; font-size: 1.1rem; }
        button.off { background: #e53e3e; }
        .footer { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; 
                 text-align: center; color: #718096; padding-top: 20px; border-top: 1px solid #eaeaea; }
    </style>
</head><body>
    <div class="container">
        <header>
            <h1><i class="fas fa-cloud-sun"></i> Station Météo ESP32</h1>
            <p>Données en temps réel</p>
        </header>
        
        <div class="dashboard">
            <div class="sensor-card">
                <i class="fas fa-temperature-high" style="color:#e53e3e;font-size:2.5rem;"></i>
                <h2>Température</h2>
                <div class="value">)rawliteral" + String(temperature, 1) + R"rawliteral(°C</div>
            </div>
            
            <div class="sensor-card">
                <i class="fas fa-tint" style="color:#3182ce;font-size:2.5rem;"></i>
                <h2>Humidité</h2>
                <div class="value">)rawliteral" + String(humidity, 1) + R"rawliteral(%</div>
            </div>
            
            <div class="sensor-card">
                <i class="fas fa-clock" style="color:#d69e2e;font-size:2.5rem;"></i>
                <h2>Heure</h2>
                <div class="value">)rawliteral" + timeClient.getFormattedTime() + R"rawliteral(</div>
            </div>
        </div>

        <div class="controls">
            <button onclick="location.href='/on'"><i class="fas fa-lightbulb"></i> Allumer LED</button>
            <button onclick="location.href='/off'" class="off"><i class="fas fa-lightbulb"></i> Éteindre LED</button>
            <button onclick="location.href='/data'"><i class="fas fa-database"></i> Données JSON</button>
            <button onclick="location.href='/info'"><i class="fas fa-info-circle"></i> Info système</button>
        </div>

        <div class="footer">
            <div><strong>IP:</strong> )rawliteral" + WiFi.softAPIP().toString() + R"rawliteral(</div>
            <div><strong>Dernière maj:</strong> )rawliteral" + timeClient.getFormattedTime() + R"rawliteral(</div>
            <div><strong>Appareils:</strong> )rawliteral" + String(WiFi.softAPgetStationNum()) + R"rawliteral(</div>
        </div>
    </div>
</body></html>
)rawliteral";
    server.send(200, "text/html", page); });

    // Routes supplémentaires
    server.on("/on", []()
              {
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "text/plain", "LED allumée! Temp: " + String(temperature, 1) + "°C"); });

    server.on("/off", []()
              {
    digitalWrite(LED_PIN, LOW);
    server.send(200, "text/plain", "LED éteinte!"); });

    server.on("/data", []()
              {
    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"time\":\"" + timeClient.getFormattedTime() + "\",";
    json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\"";
    json += "}";
    server.send(200, "application/json", json); });

    server.on("/info", []()
              {
    String info = "Station Météo ESP32\n";
    info += "IP: " + WiFi.softAPIP().toString() + "\n";
    info += "Temp: " + String(temperature, 1) + "°C\n";
    info += "Hum: " + String(humidity, 1) + "%\n";
    info += "Heure: " + timeClient.getFormattedTime() + "\n";
    info += "Appareils connectés: " + String(WiFi.softAPgetStationNum()) + "\n";
    info += "Uptime: " + String(millis() / 1000) + " secondes";
    server.send(200, "text/plain", info); });

    server.onNotFound([]()
                      {
    String message = "Page non trouvée: " + server.uri() + "\n\n";
    message += "Pages disponibles:\n";
    message += "- / : Accueil\n";
    message += "- /on : Allumer LED\n";
    message += "- /off : Éteindre LED\n";
    message += "- /data : Données JSON\n";
    message += "- /info : Info système\n";
    server.send(404, "text/plain", message); });

    Serial.println("Serveur web démarré!");
}
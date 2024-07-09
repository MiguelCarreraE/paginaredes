#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Definición de pines y constantes
#define TTP223_PIN 4  // Pin GPIO 4
#define DHT_PIN 13    // Pin GPIO 13
#define DHT_TYPE DHT11 // Cambia a tu tipo de DHT, DHT11, etc.

const char* ssid = "Totalplay-2.4G-5808";
const char* password = "APNeLmFhEjCS3pMh";
const char* serverName = "https://redes-practice.onrender.com/api/state/4";

DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastTempUpdate = 0;
const long tempInterval = 60000; // Intervalo de 1 minuto

void setup() {
  Serial.begin(115200);
  pinMode(TTP223_PIN, INPUT);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  dht.begin();

  // Prueba inicial del sensor DHT
  delay(2000); // Esperar 2 segundos para estabilizar el sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor! Verifica las conexiones y la configuración.");
  } else {
    Serial.print("Initial Temperature: ");
    Serial.println(t);
    Serial.print("Initial Humidity: ");
    Serial.println(h);
  }
}

void loop() {
  // Leer el estado del TTP223
  int ttpState = digitalRead(TTP223_PIN);

  // Si el TTP223 es presionado
  if (ttpState == HIGH) {
    sendDoorbellState(true);
    Serial.print("TPP223 pressed");
    delay(20000); // Espera 20 segundos
    sendDoorbellState(false);
  }

  // Enviar temperatura cada 1 minuto
  unsigned long currentMillis = millis();
  if (currentMillis - lastTempUpdate >= tempInterval) {
    Serial.println("Reading Temperature...");
    lastTempUpdate = currentMillis;
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
      Serial.println("Failed to read temperature from DHT sensor!");
    } else {
      Serial.print("Temperature: ");
      Serial.println(temperature);
      sendTemperature(temperature);
    }
  }
}

void sendDoorbellState(bool state) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"doorbell\":\"" + String(state ? "true" : "false") + "\"}";
    int httpResponseCode = http.PUT((uint8_t*)payload.c_str(), payload.length());

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending PUT request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void sendTemperature(float temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"temperature\":" + String(temperature, 2) + "}";
    int httpResponseCode = http.PUT((uint8_t*)payload.c_str(), payload.length());

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending PUT request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
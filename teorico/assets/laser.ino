#include <WiFi.h>
#include <HTTPClient.h>

#define LaserPin 18
#define SensorPin 34

int laserThreshold = 800; // Ajusta este valor según tus observaciones
int sensorValue;

const char* ssid = "Totalplay-EAA5_2.4Gnormal";
const char* pass = "SoporteTotalplay123";
const char* apiUrl = "https://redes-practice.onrender.com/api/state/4";

void setup() {
  pinMode(LaserPin, OUTPUT);
  pinMode(SensorPin, INPUT);
  Serial.begin(115200); // Configura la velocidad de baudios a 115200

  WiFi.begin(ssid, pass);
  Serial.print("Conectando a ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Encender el láser
  digitalWrite(LaserPin, HIGH);
  delay(10); // Espera un poco para estabilizar la lectura

  // Leer el sensor
  sensorValue = analogRead(SensorPin);
  Serial.print("Valor del sensor: ");
  Serial.println(sensorValue); // Imprime el valor del sensor para depuración

  // Determinar el estado basado en la lectura del sensor
  String estado = (sensorValue < laserThreshold) ? "Seguro" : "Intruso";
  Serial.print("Estado: ");
  Serial.println(estado);

  // Asegurarse de que el sensor funcione antes de enviar cualquier dato
  if (sensorValue > 0) {
    // Verificar la conexión WiFi antes de enviar datos
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      Serial.println("Conectado a WiFi, enviando datos...");

      http.begin(apiUrl);
      http.addHeader("Content-Type", "application/json");

      String jsonData = "{\"laser\":\"" + estado + "\"}";
      Serial.print("JSON Data: ");
      Serial.println(jsonData);

      int httpResponseCode = http.PUT(jsonData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("Respuesta de la API: ");
        Serial.println(response);
      } else {
        Serial.print("Error en la petición PUT: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("WiFi no está conectado.");
    }
  } else {
    Serial.println("El sensor está marcando 0. Verifica las conexiones.");
  }

  delay(500); // Espera dos segundos antes de la siguiente lectura
}

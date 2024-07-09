#include <WiFi.h>
#include <HTTPClient.h>

const int DOOR_SENSOR_PIN = 13;
int doorState;

const char* ssid = "INFINITUM5F96";
const char* pass = "Cz2Kvzz49v";

void setup() {
  Serial.begin(9600);

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

  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  }

void loop() {
  doorState = digitalRead(DOOR_SENSOR_PIN);
  String doorStatus = (doorState == HIGH) ? "Open" : "Closed";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://redes-practice.onrender.com/api/state/4";

    http.begin(url);
    http.addHeader("Content-Type", "application/json"); 
    
    String jsonData = "{\"doorstatus\":\"" + doorStatus + "\"}";

    int httpResponseCode = http.PUT(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("Error en la petición POST: ");
    }

    http.end();
  }

  delay(10000);
}

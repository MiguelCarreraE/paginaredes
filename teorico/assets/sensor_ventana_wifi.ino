#include <WiFi.h>
#include <HTTPClient.h>

const int WINDOW_SENSOR_PIN = 19;
int windowState;

const char* ssid = "iPhone de Diego";
const char* pass = "12345678";
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

  pinMode(WINDOW_SENSOR_PIN, INPUT_PULLUP);
}

void loop() {
  windowState = digitalRead(WINDOW_SENSOR_PIN);
  String windowStatus = (windowState == HIGH) ? "Open" : "Closed";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://redes-practice.onrender.com/api/state/4";

    http.begin(url);
    http.addHeader("Content-Type", "application/json"); 
    
    String jsonData = "{\"windowstatus\":\"" + windowStatus + "\"}";

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
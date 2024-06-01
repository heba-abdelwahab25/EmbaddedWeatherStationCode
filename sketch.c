#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_BMP280.h>
#include "DHT.h

#define DHT_PIN D3
#define DHT_TYPE DHT11
#define BMP_SDA D2
#define BMP_SCL D1
#define RAIN_SENSOR A0

const char *ssid = "Staff";
const char *pass = "Ejust-Staff-2023";
const char *server = "api.thingspeak.com";
const String apiKey = "S4MZ2MK9ZT337OCA";

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp;

void readSensors();

void setup() {
  Serial.begin(115200);
  delay(10);


  if (!bmp.begin(BMP_SDA, BMP_SCL)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try restarting."));
    while (1);
  }

  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  readSensors();
  delay(1000);
}

void readSensors() {
  // BMP280 sensor
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;

  //DHT sensor
  float humidity = dht.readHumidity();
  float dhtTemperature = dht.readTemperature();

  //Rain sensor
  int rain = analogRead(RAIN_SENSOR);
  rain = map(rain, 0, 1024, 0, 100);

  // ERROR handling
  if (isnan(humidity) || isnan(dhtTemperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  WiFiClient client;
  if (client.connect(server, 443)) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(dhtTemperature);
    postStr += "&field2=";
    postStr += String(humidity);
    postStr += "&field3=";
    postStr += String(pressure, 2);
    postStr += "&field4=";
    postStr += String(rain);
    postStr += "\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(server);
    client.print("\r\n");
    client.print("Connection: close\r\n");
    client.print("X-THINGSPEAKAPIKEY: ");
    client.print(apiKey);
    client.print("\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\r\n\r\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.println(dhtTemperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Absolute Pressure: ");
    Serial.print(pressure, 2);
    Serial.println(" mb");
    Serial.print("Rain: ");
    Serial.println(rain);
  }
  client.stop();
}

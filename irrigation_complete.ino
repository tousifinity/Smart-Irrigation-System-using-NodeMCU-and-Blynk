#define BLYNK_TEMPLATE_ID "*"
#define BLYNK_DEVICE_NAME "*"
#define BLYNK_AUTH_TOKEN  "*"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <SimpleTimer.h>

// Blynk WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "*";
char pass[] = "*";

// DHT11 settings
#define DHTPIN 2          // GPIO2 or D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture Sensor
#define SOIL_SENSOR A0    // Analog input

// Water Pump and LED Pins
#define PUMP_PIN D5
#define LED_PIN  D3

// OneWire for optional DallasTemperature sensor
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Mode control
bool autoMode = true; // true = auto, false = manual

SimpleTimer timer;

// Function to send temperature and humidity
void sendDHTData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V5, h); // Humidity
  Blynk.virtualWrite(V6, t); // Temperature

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("°C, Humidity: ");
  Serial.print(h);
  Serial.println("%");
}

// Function to read soil and control pump (auto mode)
void sendSoilData() {
  int raw = analogRead(SOIL_SENSOR);
  int moisturePercent = 100 - map(raw, 0, 1023, 0, 100); // Calibrated to inverse logic

  // Optional temperature sensor
  sensors.requestTemperatures();
  float soilTemp = sensors.getTempCByIndex(0);

  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  Blynk.virtualWrite(V2, moisturePercent); // Send soil moisture
  // Blynk.virtualWrite(V9, soilTemp); // Optional: Soil temperature

  if (autoMode) {
    if (moisturePercent < 30) {
      digitalWrite(PUMP_PIN, HIGH);  // Turn pump ON
      Blynk.virtualWrite(V4, 1);     
      Serial.println("Auto Mode: Pump ON");
    } else {
      digitalWrite(PUMP_PIN, LOW);   // Turn pump OFF
      Blynk.virtualWrite(V4, 0);
      Serial.println("Auto Mode: Pump OFF");
    }
  }
}

// Manual/Auto mode switch (V7)
BLYNK_WRITE(V7)
{
  int mode = param.asInt(); // 1 = Auto, 0 = Manual
  autoMode = (mode == 1);

  if (autoMode) {
    Serial.println("Switched to AUTO mode");
  } else {
    Serial.println("Switched to MANUAL mode");
  }
}

// Manual pump control (V4)
BLYNK_WRITE(V4)
{
  int value = param.asInt();
  if (!autoMode) {
    if (value == 1) {
      digitalWrite(PUMP_PIN, HIGH);
      Serial.println("Manual: Pump ON");
    } else {
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("Manual: Pump OFF");
    }
  }
}

// Manual LED control (V3)
BLYNK_WRITE(V3)
{
  int value = param.asInt();
  digitalWrite(LED_PIN, value ? HIGH : LOW);
  Serial.println(value ? "LED ON" : "LED OFF");
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  dht.begin();
  sensors.begin();

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  timer.setInterval(2000L, sendDHTData);   // Every 2 seconds
  timer.setInterval(3000L, sendSoilData);  // Every 3 seconds
}

void loop()
{
  Blynk.run();
  timer.run();
}

#define BLYNK_TEMPLATE_ID "TMPLy2IFAm6-"
#define BLYNK_DEVICE_NAME "Soil Moisture Measurement with auto pump on off"
#define BLYNK_AUTH_TOKEN "QGngeotkluNE5H7_Oio9_xJWwy4eP_Bv"

#define BLYNK_PRINT Serial
#include <OneWire.h>
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "You're Hacked!";
char pass[] = "hprog2*1050Ti";

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
SimpleTimer timer;
void sendSensor()
{
float h = dht.readHumidity();
float t = dht.readTemperature();

if (isnan(h) || isnan(t)) {
Serial.println("Failed to read from DHT sensor!");
return;
}

Blynk.virtualWrite(V5, h); //V5 is for Humidity
Blynk.virtualWrite(V6, t); //V6 is for Temperature
}
void setup()
{
Serial.begin(9600);
dht.begin();

timer.setInterval(1000L, sendSensor);
Blynk.begin(auth, ssid, pass);
pinMode(D5,OUTPUT);
pinMode(D3,OUTPUT);
sensors.begin();
}
int sensor=0;
int output=0;
float temp;
void sendTemps()
{
sensor=analogRead(A0);
output=(100-map(sensor,0,1023,0,100)); //in place 145 there is 100(it change with the change in sensor)
//delay(1000);
sensors.requestTemperatures();
temp = sensors.getTempCByIndex(0);
Serial.print("Temperature = ");
Serial.print(temp);
Serial.print("moisture = ");
Serial.print(output);
Serial.println("%");
//Blynk.virtualWrite(V9,temp);
Blynk.virtualWrite(V2,output);
delay(1000);
}

BLYNK_WRITE(V4)
{
  int value = param.asInt();
  Serial.print(value);
  if(value == 1)
  {
    digitalWrite(D5, HIGH);
    Serial.print("Motor ON");
  }
  else
  {
     digitalWrite(D5, LOW);
     Serial.print("Motor OFF");
  }
}

BLYNK_WRITE(V3)
{
  int value = param.asInt();
  Serial.print(value);
  if(value == 1)
  {
    digitalWrite(D3, HIGH);
    Serial.print("LED ON");
  }
  else
  {
     digitalWrite(D3, LOW);
     Serial.print("LED OFF");
  }
}

void loop()
{
Blynk.run();
timer.run();
sendTemps();
}

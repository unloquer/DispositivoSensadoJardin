#include "Arduino.h"
#include "constants.h"
//sensor de humedad
#include "DHT.h"
// https://developer.ibm.com/recipes/tutorials/use-http-to-send-data-to-the-ibm-iot-foundation-from-an-esp8266/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// librerias de cliente ntp
#include <WiFiUdp.h>
#include <NTPClient.h>

#define errorPin 2
WiFiClient client;
HTTPClient http;
// ntp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
// https://github.com/adafruit/DHT-sensor-library



#define DHTPIN 4     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

#define MainPeriod 100  // measurement frame, milliseconds
#define f_input_pin 5   // input pin for pulseIn
#define RELAY_PIN 5

long previousMillis = 0;
unsigned long duration=0; // receive pulse width
long pulsecount=0;

int ligth_read;

/*
  # Request.
  $ curl -XPOST -H 'Content-Type: application/json;' -H 'X-Auth-Token: AUTH_TOKEN' -d '{"value": 23}' http://things.ubidots.com/api/v1.6/variables/ID_VARIABLE/values

  # Response.
  {
  "url": "http:///things.ubidots.com/api/v1.6/values/ID_VARIABLE",
  "value": 23.0,
  "timestamp": 1369847742302,
  "created_at": "2013-08-29T17:15:42.302"
  }
*/

int server_request(float value, String type, unsigned long time_stamp) {

  String url = "http://104.131.1.214:3000/api/SensorEvents";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  //http.addHeader("X-Auth-Token", AUTH_TOKEN);
  int content_length =0;
  String payload = String("{ \"value\":\" " + String(value) + "\",");
  payload += String("\"timestamp\":\"" + String(time_stamp) + "\",");
  payload += String("\"sensorId\" :\"" + type + "\"}");
  //content_length = payload.length();
  //http.addHeader("Content-Length", String(content_length));
  int httpCode = http.POST(payload);
  if(httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("[HTTP] failed, error: ");Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();

  return httpCode;
}

float get_termperature() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  Serial.print(" Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  return t;
}

float get_humidity() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read humidity
  float h = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  Serial.print("Humidity: ");
  Serial.print(h);

  return h;
}

int get_light() {
  // Leer el valor de la fotocelda a través del ADC
  ligth_read = analogRead(A0);

  Serial.print("Luz: ");
  Serial.println(ligth_read);

  return ligth_read;
}

float get_soil_moisture() {
  float Freq;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MainPeriod)
  {
    previousMillis = currentMillis;
    // write current time and F values to the serial port
    //Serial.print(currentMillis);
    //Serial.print(" "); // separator!
    Freq=0.5e6/float(duration); // assume pulse duty cycle 0.5
    Freq*=pulsecount;
    Serial.print("Humedad en la tierra: ");
    Serial.print(Freq);
    Serial.println(" ");
    //Serial.print(pulsecount);
    //Serial.print(" ");
    //Serial.println(duration);
    duration=0;
    pulsecount=0;
  }
  // instead of single measurement per cycle - accumulate and average
  duration += pulseIn(f_input_pin, HIGH, MainPeriod*900);
  pulsecount++;

  return Freq;
}

void setup() {
  pinMode(f_input_pin, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("DHTxx test!");

  dht.begin();

  const char* ssid = ESSID;
  const char* password = WIFI_PASSWORD;

  for (int i=0;i<4; i++){   // let know we are working
    digitalWrite(errorPin ,HIGH);
    delay(200);
    digitalWrite(errorPin ,LOW);
    delay(200);
  }

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000*8);
  // actualiza el timeClient
  timeClient.update();

  unsigned long time_to_send =   timeClient.getRawTime(); 
  // Read humidity
  float h = get_humidity();
  // Read temperature as Celsius
  float t = get_termperature();

  Serial.print(timeClient.getRawTime());
  Serial.print("\t");
  Serial.print(timeClient.getHours());
  Serial.print("\t");
  Serial.print(timeClient.getMinutes());
  Serial.print("\t");
  Serial.print(timeClient.getSeconds());
  Serial.print(timeClient.getFormattedTime());
  Serial.print("\t");
  Serial.print(millis());
  Serial.print("\t");
  Serial.println(time_to_send);
  delay(200);
//  server_request(t, "Temperatura",time_to_send);
 // server_request(h, "Humedad",time_to_send);
//  server_request(t, "Luz",time_to_send);

  if(t >= 30.0) {
    digitalWrite(RELAY_PIN,LOW);
  } else {
    digitalWrite(RELAY_PIN,HIGH);
  }
  delay(2000*8);
  //float freq = get_soil_moisture();
  //int l = get_light();
}

#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHT.h"
#include <HardwareSerial.h>


#define DHTPIN 15  //
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int mqPin = A0;
int s1 = 12; //N   12  D12
int s2 = 14; //NE  14  D14
int s3 = 27; //E   27  D27
int s4 = 26; //SE  26  D26
int s5 = 25; //S   25  D25
int s6 = 33; //SW  33  D33
int s7 = 32; //W   32  D32
int s8 = 35; //NW  35  D35
int val1;
int val2;
int val3;
int val4;
int val5;
int val6;
int val7;
int val8;

 int wd;
// Replace with your network credentials
//const char* ssid = "watpum";
//const char* password = "vcsvcs596";

const char* ssid = "robotlab_2.4G";
const char* password = "vcs112344";
//const char* ssid = "M";
//const char* password = "88888888";

// MQTT Broker details
//const char* mqtt_server = "your_MQTT_Broker_IP"; // e.g., 192.168.1.100
//const int mqtt_port = 1883; // Default port for MQTT

const char* mqtt_server = "iot.varee.ac.th";  // Use your private broker IP
const char* mqtt_user = "root";               // MQTT username
const char* mqtt_password = "Varee123";

WiFiClient espClient;
PubSubClient client(espClient);

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to connect to the MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, you can publish or subscribe to topics
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  pinMode(s1, INPUT);
  pinMode(s2, INPUT);
  pinMode(s3, INPUT);
  pinMode(s4, INPUT);
  pinMode(s5, INPUT);
  pinMode(s6, INPUT);
  pinMode(s7, INPUT);
  pinMode(s8, INPUT);
  setup_wifi();                         // Connect to WiFi
  client.setServer(mqtt_server, 1883);  // Set the MQTT broker
  dht.begin();
}



void loop() {
  wd == 0;
  if (!client.connected()) {
    reconnect();  // Ensure we are connected to the broker
  }
  client.loop();

  // Read gas sensor value
  int sensorValue = analogRead(mqPin);

  // Read humidity and temperature from DHT11 sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();      // Celsius
  float f = dht.readTemperature(true);  // Fahrenheit

  // Check if any reads failed
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  // Print sensor data to Serial Monitor
  Serial.print("Gas Sensor Value: ");
  Serial.println(sensorValue);
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.print("°C ");
  Serial.print(f);
  Serial.print("°F  Heat index: ");
  Serial.print(hic);
  Serial.print("°C ");
  Serial.print(hif);
  Serial.println("°F");

  val1 = digitalRead(s1);
  val2 = digitalRead(s2);
  val3 = digitalRead(s3);
  val4 = digitalRead(s4);
  val5 = digitalRead(s5);
  val6 = digitalRead(s6);
  val7 = digitalRead(s7);
  val8 = digitalRead(s8);
  if (val1 == 0) {
    //wd["N"];
    wd = 0;
    Serial.println("N");
  }
  if (val2 == 0) {
    //wd["NE"];
    wd = 45;
    Serial.println("NE");
  }
  if (val3 == 0) {
    //wd["E"];
    wd = 90;
    Serial.println("E");
  }
  if (val4 == 0) {
    //wd["SE"];
    wd = 135;
    Serial.println("SE");
  }
  if (val5 == 0) {
    //wd["S"];
    wd = 180;
    Serial.println("S");
  }
  if (val6 == 0) {
    //wd["SW"];
    wd = 225;
    Serial.println("SW");
  }
  if (val7 == 0) {
    //wd["W"];
    wd = 270;
    Serial.println("W");
  }
  if (val8 == 0) {
    //wd["NW"];
    wd = 315;    
    Serial.println("NW");
  }

Serial.print("WD : ");
Serial.println(wd);

  // Prepare JSON payload to send over MQTT
  String payload;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["Temp"] = t;
  jsonDoc["HUMI"] = h;
  jsonDoc["CO2"] = sensorValue;
  jsonDoc["Wind_speed"] = "0";      /////// ลม
  jsonDoc["Wind_direction"] = wd;  //////// ลม

  serializeJson(jsonDoc, payload);

  // Publish the sensor data to Node-RED on the topic "sensor/data"
  client.publish("sensor/data", payload.c_str());

  delay(1000);  // Wait for 1 second before next reading
}
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const int sensorPin = 15;             // ขาที่เชื่อมต่อกับเซ็นเซอร์
volatile int count = 0;               // ตัวนับรอบ
int lastValue = HIGH;                 // ค่าของสัญญาณก่อนหน้า
unsigned long lastTime = 0;           // เวลาที่ผ่านมาในการนับรอบ
float windSpeed = 0.0;                // ความเร็วลม (เมตร/วินาที)
const float calibrationFactor = 1.0;  // ปรับค่าให้เหมาะสมกับเซ็นเซอร์

// Replace with your network credentials
//const char* ssid = "watpum";
//const char* password = "vcsvcs596";

const char* ssid = "robotlab_2.4G";
const char* password = "vcs112344";
//const char* ssid = "M";
//const char* password = "88888888";

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
  Serial.begin(9600);         // เริ่มการสื่อสารกับ Serial Monitor
  pinMode(sensorPin, INPUT);  // ตั้งค่าขา sensorPin เป็นอินพุต
  lastTime = millis();        // บันทึกเวลาเริ่มต้น
  setup_wifi();                         // Connect to WiFi
  client.setServer(mqtt_server, 1883); 
}

void loop() {
   if (!client.connected()) {
    reconnect();  // Ensure we are connected to the broker
  }
  client.loop();
  // อ่านค่าปัจจุบันจากเซ็นเซอร์
  int currentValue = digitalRead(sensorPin);
  // ตรวจสอบการเปลี่ยนแปลงจาก 1 เป็น 0
  if (lastValue == HIGH && currentValue == LOW) {
    count++;  // เพิ่มค่าตัวนับรอบเมื่อสัญญาณเปลี่ยนจาก 1 เป็น 0
  }
  lastValue = currentValue;  // อัพเดตค่าของสัญญาณก่อนหน้า
  // คำนวณเวลาในการนับรอบ
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTime;
  // ตรวจสอบว่าผ่าน 1 วินาทีหรือไม่
  if (elapsedTime >= 1000) {
    // คำนวณความเร็วลม (เมตร/วินาที)
    windSpeed = (count / calibrationFactor) / (elapsedTime / 500.0);

    // แสดงค่าตัวนับรอบและความเร็วลมใน Serial Monitor
    Serial.print("Count: ");
    Serial.print(count);
    Serial.print(" | Wind Speed (m/s): ");
    Serial.println(windSpeed);

    // รีเซ็ตค่าตัวนับรอบและเวลาที่ผ่านมา
    count = 0;
    lastTime = currentTime;
  }

   String payload;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["wind_speed"] = windSpeed;
  

  serializeJson(jsonDoc, payload);

  // Publish the sensor data to Node-RED on the topic "sensor/data"
  client.publish("sensor/wind_speed", payload.c_str());

  //delay(1000); 
}
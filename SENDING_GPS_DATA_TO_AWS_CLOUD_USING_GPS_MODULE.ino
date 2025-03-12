#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>  
#include <SoftwareSerial.h>  
#include "env.h"

// GPS TX to ESP8266 GPIO4 (D2)    
// GPS RX to ESP8266 GPIO5 (D1)
TinyGPSPlus gps;
static const int RXPin = 4, TXPin = 5;
SoftwareSerial gpsSerial(RXPin, TXPin);

// WiFi credentials
const char WIFI_SSID[] = "realme 9 Pro 5G";
const char WIFI_PASSWORD[] = "w4534w87";

// Device name from AWS
const char THINGNAME[] = "ESP8266";

// MQTT broker host address from AWS
const char MQTT_HOST[] = "a38y1e4m0vzu5f-ats.iot.ap-south-1.amazonaws.com";

// MQTT topics
const char AWS_IOT_PUBLISH_TOPIC[] = "esp8266/pub";
const char AWS_IOT_SUBSCRIBE_TOPIC[] = "esp8266/sub";

// Publishing interval
const long interval = 5000;

// Timezone offset from UTC
const int8_t TIME_ZONE = -5;

// Last time message was sent
unsigned long lastMillis = 0;

// WiFiClientSecure object for secure communication
WiFiClientSecure net;

BearSSL::X509List cert(cacert);  // X.509 certificate for the CA
BearSSL::X509List client_crt(client_cert); // X.509 certificate for the client
BearSSL::PrivateKey key(privkey);  // RSA private key

// MQTT client instance
PubSubClient client(net);

// Function to connect to NTP server and set time
void NTPConnect() {
  // Set time using SNTP
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 1510592825) { // January 13, 2018
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
}

// Callback function for message reception
void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Function to connect to AWS IoT Core
void connectAWS() {
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  // Connect to NTP server to set time
  NTPConnect();

  // Set CA and client certificate for secure communication
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  // Connect MQTT client to AWS IoT Core
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Connecting to AWS IoT");

  // Attempt to connect to AWS IoT Core
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }

  // Check if connection is successful
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to MQTT topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

// Function to publish message to AWS IoT Core
void collectgpsdata() {
  while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isUpdated()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        float altitude = gps.altitude.meters();
        Serial.print("Latitude: ");
        Serial.print(latitude, 6);
        Serial.print(" Longitude: ");
        Serial.println(longitude, 6);
        Serial.print(" Altitude: ");
        Serial.println(altitude, 6);
    }
    publishGPSData();
    delay(5000);  // Publish every 5 seconds
  }

  
void publishGPSData() {
    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        float altitude = gps.altitude.meters();

        // Create JSON payload
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["latitude"] = latitude;
        jsonDoc["longitude"] = longitude;
        jsonDoc["altitude"] = altitude;

        char jsonBuffer[200];
        serializeJson(jsonDoc, jsonBuffer);
        
        // Publish to AWS IoT
        if (client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer)) {
            Serial.println("GPS Data Published!");
        } else {
            Serial.println("Publish Failed!");
        }
    } else {
        Serial.println("Waiting for valid GPS data...");
    }
}

  
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  gpsSerial.begin(9600);
    delay(1000);
  
  // Connect to AWS IoT Core
  connectAWS();
}


void loop() {
  // Check if it's time to publish a message
  if (millis() - lastMillis > interval) {
    lastMillis = millis();
    if (client.connected()) {
      // Publish message
      collectgpsdata();
    }
  }
  // Maintain MQTT connection
  client.loop();
}

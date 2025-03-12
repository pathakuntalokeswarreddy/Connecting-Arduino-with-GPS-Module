#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#define RX_PIN 4  // GPS TX to ESP8266 GPIO4 (D2)    
#define TX_PIN 5  // GPS RX to ESP8266 GPIO5 (D1) 

SoftwareSerial gpsSerial(RX_PIN, TX_PIN);
TinyGPSPlus gps;

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600);
}

void loop() {
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isUpdated()) {
        Serial.print("Latitude: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(" Longitude: ");
        Serial.println(gps.location.lng(), 6);
    }
}

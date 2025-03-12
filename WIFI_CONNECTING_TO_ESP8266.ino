// Wi-Fi Credentials
//const char* ssid = "****";
//const char* password = "****";

#include <ESP8266WiFi.h>

const char* ssid = "****";
const char* password = "****";

void connectAWS() {
    Serial.println("Setting up AWS certificates...");
    
    // Set up certificate and key
    BearSSL::X509List trustAnchor(aws_root_ca);   // Loads Root CA
    BearSSL::X509List cert(device_cert);         // Loads Device Certificate
    BearSSL::PrivateKey key(private_key);        // Loads Private Key these three are needed to establish a secure TLS connection with AWS

    // Apply them to the secure client(THESE TELLS ESP8266 TO USE THESE CERTIFICATES FOR SECURE COMMUNICATION)
    espClient->setTrustAnchors(&trustAnchor);
    espClient->setClientRSACert(&cert, &key);

    // Connect MQTT client to AWS IoT
    client.setServer(aws_endpoint, 8883);  // AWS MQTT uses port 8883

    // 🔹 Try connecting to AWS IoT
    Serial.println("Connecting to AWS IoT...");
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection... ");
        if (client.connect("ESP_GPS_Device")) {  // Change "ESP_GPS_Device" to a unique client ID
            Serial.println("Connected to AWS IoT!");
        } else {
            Serial.print("Failed. MQTT State: ");
            Serial.println(client.state());  // Print error code
            delay(5000);  // Retry every 5 seconds
        }
    }
}


void connectWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
        attempt++;

        if (attempt > 15) {  // Stop trying after 15 seconds
            Serial.println("\nWiFi Connection Failed!");
            return;
        }
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    delay(1000);  // Small delay to stabilize Serial Monitor
    Serial.println("\nStarting ESP8266...");
    
    connectWiFi();
}

void loop() {
    // Empty for now
}

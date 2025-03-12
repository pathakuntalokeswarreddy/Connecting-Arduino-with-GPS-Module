void setup() {
    pinMode(2, OUTPUT);  // GPIO2 (Built-in LED)
}

void loop() {
    digitalWrite(2, LOW);  // Turn LED ON (Inverted Logic)
    delay(1000);  
    digitalWrite(2, HIGH); // Turn LED OFF
    delay(1000);  
}

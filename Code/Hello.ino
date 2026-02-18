void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Hello ESP32");
}

void loop() {
  Serial.println("tick");
  delay(1000);
}

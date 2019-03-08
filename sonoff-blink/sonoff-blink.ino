/*
 ESP8266 Blink for Sonoff switch
 
*/

#define LED_PIN 13

void setup() {
  Serial.begin(115200);
  Serial.println("Setup output"); 
  pinMode(LED_PIN, OUTPUT);
}

void loop() { 
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED off");
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED on");
  delay(1000);
}

/*

  Sonoff switch MQTT

  Connects to MQTT server and accepts commands of 0 turn off, 1 turn on
  Updates off and on with button press and sends notice to server

  Mike Braden
  2018-feb-11

  v4

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>
#include <EEPROM.h>

//==============================================================
// Network and MQTT settings
// Change these to match your wifi and mqtt server settings

const char* wifiSsid = "YOUR_WIFI_SSID";
const char* wifiPassword = "YOUR_WIFI_PASS";
const char* mqttServer = "192.168.0.X";  // if not using DNS, your Pi server
//const char* mqttServer = "YOUR_MQTT_PI_SERVER";  // if using DNS, your Pi server

const char* mqttUser = "sonusr1";
const char* mqttPassword = "xyz123abc";
const char* outTopic = "sonoff/node01/stat";
const char* inTopic = "sonoff/node01/cmd";

const int mqttPort = 1883;

#define CONNECT_MSG "sonoff01: Connected"
#define RELAY_PIN 12
#define LED_PIN 13
#define SWITCH_PIN 0

bool switchState = LOW;

long lastMsg = 0;
char msg[50];
int value = 0;

WiFiClient netClient;
PubSubClient mqttClient(netClient);
Bounce debouncer = Bounce();

// function protoypes
void callBack(char* topic, byte* payload, unsigned int length);
void setupWifi();
void reconnect();
void loop();
void extButton();
void switchOn();
void switchOff();


void setup() {
  pinMode(RELAY_PIN, OUTPUT);     // Set the relay pin as an output
  pinMode(SWITCH_PIN, INPUT);     // Set the button pin as an input
  pinMode(LED_PIN, OUTPUT);       // Set the led pin as an output

  Serial.begin(115200);

  //Serial.println("delay start");
  //delay(10000);                   // delay to allow catching start in serial monitor

  EEPROM.begin(512);              // Begin eeprom to store on/off state

  switchState = EEPROM.read(0);

  Serial.print("switchState from EEPROM: ");
  Serial.println(switchState);

  // update outputs directly, can't use swithon/off until mqtt is setup
  if (switchState == 0) {
    digitalWrite(LED_PIN, HIGH);  // Turn the LED off with high
    digitalWrite(RELAY_PIN, LOW);     // Turn the relay off (open) with low
  }
  else {
    digitalWrite(LED_PIN, LOW);   // Turn the LED on with low
    digitalWrite(RELAY_PIN, HIGH);    // Turn the relay on (closed) with high
  }

  debouncer.attach(SWITCH_PIN);   // Use the bounce2 library to debounce the built in button
  debouncer.interval(50);         // interval of 50 ms

  setupWifi();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callBack);

}

void setupWifi() {

  // Connect to network ssid
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSsid);

  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    // use loop instead of delay to keep button active
    extButton();
    for (int i = 0; i < 500; i++) {
      extButton();
      delay(1);
    }
    Serial.print(".");
  }

  // once connected print status info - IP address, gateway, DNS, signal strength
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.printf("Subnet mask: %s\n", WiFi.subnetMask().toString().c_str());
  Serial.printf("Gataway IP: %s\n", WiFi.gatewayIP().toString().c_str());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);

  Serial.printf("DNS 1 IP:  %s\n", WiFi.dnsIP(0).toString().c_str());
  Serial.printf("DNS 2 IP:  %s\n", WiFi.dnsIP(1).toString().c_str());

  Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());
}

void callBack(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch the LED and relay if first character recieved is 0 or 1
  if ((char)payload[0] == '0') {
    mqttClient.publish(outTopic, "Switch command received");
    switchOff();
  } else if ((char)payload[0] == '1') {
    mqttClient.publish(outTopic, "Switch command received");
    switchOn();
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
      // Once connected, publish the connect message and the current switch state
      mqttClient.publish(outTopic, CONNECT_MSG);
      if (switchState == 0)
        mqttClient.publish(outTopic, "0");
      else if (switchState == 1)
        mqttClient.publish(outTopic, "1");
      // subscribe to topic
      mqttClient.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      // using a loop to update the button status
      for (int i = 0; i < 5000; i++) {
        extButton();
        delay(1);
      }
    }
  }
}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }

  // upadate the mqtt client
  mqttClient.loop();

  // update the button
  extButton();
  
}

void extButton() {
  debouncer.update();

  // Call code if Bounce fell (transition from HIGH to LOW) :
  if ( debouncer.fell() ) {
    Serial.println("Debouncer fell");
    // Toggle relay state :
    switchState = !switchState;

    Serial.print("switchState changed button press: ");
    Serial.println(switchState);

    // send notice of button press
    mqttClient.publish(outTopic, "Button pressed");

    if (switchState == 0) {
      switchOff();
    }
    else if (switchState == 1) {
      switchOn();
    }
  }
}

void switchOn() {
  // set the led, relay and save the setting to eeprom
  // state is switch state 0=low=off, 1=high=on
  Serial.println("switchOn");
  digitalWrite(LED_PIN, LOW);   // Turn the LED on with low
  digitalWrite(RELAY_PIN, HIGH);  // Turn the relay on (closed) with high
  switchState = HIGH;   // state is switch state 0=low=off, 1=high=on
  EEPROM.write(0, switchState);    // Write state to EEPROM addr 0
  EEPROM.commit();
  mqttClient.publish(outTopic, "1");  // publish the change to the mqtt topic
}

void switchOff() {
  // set the led, relay and save the setting to eeprom
  // state is switch state 0=low=off, 1=high=on
  Serial.println("switchOff");
  digitalWrite(LED_PIN, HIGH);   // Turn the LED off with high
  digitalWrite(RELAY_PIN, LOW);  // Turn the relay off (open) with low
  switchState = LOW;   // state is switch state 0=low=off, 1=high=on
  EEPROM.write(0, switchState);    // Write state to EEPROM addr 0
  EEPROM.commit();
  mqttClient.publish(outTopic, "0");  // publish the change to the mqtt topic
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi and Ubidots credentials
#define WIFISSID "IOT"                               // Your WiFi SSID
#define PASSWORD "IOT12345"                          // Your WiFi password
#define TOKEN "BBFF-XWbU5zDqWfNbgVgZlbMdSkjuewEHVn"  // Your Ubidots TOKEN
#define MQTT_CLIENT_NAME "123muraliecg"              // Unique MQTT client name

// Ubidots constants
#define VARIABLE_LABEL "myecg"  // Variable label in Ubidots
#define DEVICE_LABEL "esp8266"  // Device label in Ubidots
#define SENSOR A0               // Analog sensor pin

// MQTT broker
const char mqttBroker[] = "industrial.api.ubidots.com";

// Buffers for MQTT payload and topic
char payload[100];
char topic[150];

// Buffer to store sensor values
char str_sensor[10];

// WiFi and MQTT clients
WiFiClient ubidots;
PubSubClient client(ubidots);

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
}

// Reconnect to the MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

// Setup function
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  pinMode(SENSOR, INPUT);

  Serial.println();
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
}

// Main loop function
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // Prepare the topic and payload
  snprintf(topic, sizeof(topic), "/v1.6/devices/%s", DEVICE_LABEL);
  float myecg = analogRead(SENSOR);
  dtostrf(myecg, 4, 2, str_sensor);
  snprintf(payload, sizeof(payload), "{\"%s\": {\"value\": %s}}", VARIABLE_LABEL, str_sensor);

  // Publish data to Ubidots
  Serial.print("Publishing data to Ubidots: ");
  Serial.println(myecg);
  client.publish(topic, payload);
  client.loop();

  delay(1000);  // Adjust the delay as needed
}

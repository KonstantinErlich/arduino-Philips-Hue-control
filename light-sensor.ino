#include <WiFiS3.h> //used on the Arduino Uno R4 Wifi, for a different board see which library you need.
#include "secrets.h" //used to store sensitive data
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

const int threshold = 65; // Adjust based on your light sensor and desired sensitivity

const int potentiometerPin = A1; 
int lastBrightness = -1;
int state = 0;

WiFiClient client;

IPAddress hueHub(SECRET_HUE_IP); // Replace with your Hue Bridge IP
const char* hueUsername = SECRET_HUE_USERNAME;
const int hueLight = 1; // The ID of the Hue light you want to control

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize the BH1750 sensor
  if (lightMeter.begin()) {
    Serial.println("BH1750 initialized successfully");
  } else {
    Serial.println("Error initializing BH1750");
  }
  while (!Serial);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

Serial.print("Attempting to connect to SSID: ");
Serial.println(ssid);

int status = WL_IDLE_STATUS;
while (status != WL_CONNECTED) {
  Serial.print("WiFi status: ");
  Serial.println(status);
  status = WiFi.begin(ssid, pass);
  delay(10000);  // Wait 10 seconds for connection
}
  Serial.println("WiFi connected");
}

void loop() {
  int lightLevel = lightMeter.readLightLevel();
  int potValue = analogRead(potentiometerPin);

  int brightness = map(potValue, 0, 1023, 0, 254);

  //controls:
  if (lightLevel < threshold and state == 0) {
    turnOnHueLight();
    state = 1;
  } else if (lightLevel > threshold and state == 1) {
    turnOffHueLight();
    state = 0;
  }
  if (brightness != lastBrightness and state == 1) {
    changeHueBrightness(brightness);
  }
  delay(100); 
}

//called if the light level is low and light is not yet on
void turnOnHueLight() {
    sendHueCommand("{\"on\":true}");
}

//called if the light is on and the potentiometer has been moved
void changeHueBrightness(int brightness){
  char command[50];
  sprintf(command, "{\"bri\":%d}", brightness);
  sendHueCommand(command);
}

//called if the light level is high and light is still on 
void turnOffHueLight() {
  sendHueCommand("{\"on\":false}");
}

void sendHueCommand(const char* command) {
  if (client.connect(hueHub, 80)) {
    client.print("PUT /api/");
    client.print(hueUsername);
    client.print("/lights/");
    client.print(hueLight);
    client.println("/state HTTP/1.1");
    client.print("Host: ");
    client.println(hueHub);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(strlen(command));
    client.println();
    client.println(command);
  }
  client.stop();
}

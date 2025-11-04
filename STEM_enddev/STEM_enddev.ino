/*
 *  This end device is running on an Firebeetle ESP32-E
 *  It is connecting to an MQTT broker over WiFi and TCP/IP
 *  Subscribing to Led1, Led2 and Led3 channel 
 *  Publiching on sw1, an0, temp, humidity
 *  Device ID is shown in I2C display
 *
 *  UCN/IT-Techonlogy/Ib Helmer Nielsen/2023
 */
#define DEBUG           // Enable DBPRINT
#define VERSION "0.60"
#include "DebugUtils.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DFRobot_RGBLCD1602.h"
#include "DHT.h"
#include <Adafruit_NeoPixel.h>

#define PIN_LED D5     // Control signal, connect to DI of the LED
#define NUM_LED 1       

// Custom colour1: Yellow
#define RED_VAL_1       255
#define GREEN_VAL_1     255
#define BLUE_VAL_1      0

// Constants
// IoT Device
// Subscription and publish channel
// Change the deviceid to match the end device
#define      DEVID     "esp99"
#define      DEVCLIENT "esp99Client"
#define      LED1SUB   "esp99/Led1"
#define      LED2SUB   "esp99/Led2"
#define      LED3SUB   "esp99/Led3"
#define      SW1PUB    "esp99/Sw1"
#define      AN0PUB    "esp99/An0"
#define      TEMPPUB   "esp99/Temp"
#define      HUMPUB    "esp99/Hum"
#define      NEORSUB   "esp99/NeoRed"
#define      NEOGSUB   "esp99/NeoGreen"
#define      NEOBSUB   "esp99/NeoBlue"

// Measurement setup and constants
#define  DHTTYPE        DHT22
#define  DHTPIN         D11

// WiFi Setup and constants
#include "wifi_setup.h"
#define  WIFI_TIMEOUT_MS  20000

// MQTT Broker address and port
const char * mqtt_server = "mqtt.sof60.dk";   // IP address of MQTT broker
const uint16_t mqtt_port = 1883;              // TCP port used by the MQTT broker

// LCD back light color
const int colorFull = 255;
const int colorHalf = 128;
const int colorNo   = 0;

// LED
int led1Pin = D0;                             // For some reason D1 don't work
int led2Pin = D2;                             // So D1 is ormitted in the assignment
int led3Pin = D3;

// Switch (Push buttom)
int sw1Pin = D7;
int sw2Pin = D9;

// Analog input
int an0Pin = A0;

// Objects 
WiFiClient esp32;                                     // WiFi
PubSubClient client(esp32);                           // MQTT client object
DFRobot_RGBLCD1602 lcd(/*lcdCols*/16,/*lcdRows*/2);   // 16 characters and 2 lines of show
DHT dht(DHTPIN, DHTTYPE);                             // DHT temperatur and humidity sensor
Adafruit_NeoPixel RGB_Strip = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800);  // NeoPixel LED

// Defining mutex for excluding code from preemptive task switching 
portMUX_TYPE taskMux = portMUX_INITIALIZER_UNLOCKED; 

void setup()
{
    // Setup serial port for debugging and data exchange
    Serial.begin(115200);
    // Init RGB display
    lcd.init();
    // Setup ESP's Pin's
    pinMode(led1Pin, OUTPUT); digitalWrite(led1Pin, LOW);
    pinMode(led2Pin, OUTPUT); digitalWrite(led2Pin, LOW);
    pinMode(led3Pin, OUTPUT); digitalWrite(led3Pin, LOW);
    pinMode(sw1Pin,INPUT);  
    pinMode(sw2Pin,INPUT);
     // Start Temperatur and humidity measurement
    dht.begin();
    // Setup neopixel LED
    RGB_Strip.begin();
    RGB_Strip.setBrightness(128);    // Set brightness, 0-255 (darkest - brightest)
    // Setup MQTT Client
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    // Setup WiFi task for ensuring connection to WiFi
    xTaskCreatePinnedToCore(
      keepWiFiAlive,
      "Keep WiFi Alive",
      4096,
      NULL,
      1,
      NULL,
      CONFIG_ARDUINO_RUNNING_CORE
    );
    // Setting up messurement task
    xTaskCreatePinnedToCore(
      measurement,
      "Measurement task",
      4096,
      NULL,
      1,
      NULL,
      CONFIG_ARDUINO_RUNNING_CORE
    );
    // Setting up task for scanning pushbuttom
    xTaskCreatePinnedToCore(
      keyscan,
      "Key scanning task",
      4096,
      NULL,
      1,
      NULL,
      CONFIG_ARDUINO_RUNNING_CORE
    );     
    
}
// Check wifi connection
void keepWiFiAlive(void * parameters){
  for(;;){
    if(WiFi.status() == WL_CONNECTED){
      DBPRINTLN("WiFi is connected");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      continue; 
    }
    DBPRINTLN("WiFi is not connected\n");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passwd);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){}
    if (WiFi.status() != WL_CONNECTED){
      DBPRINTLN("WiFi Failed!!");
      vTaskDelay(20000 / portTICK_PERIOD_MS);
      continue;
    }
    DBPRINT("WiFi is Connected : "); // 
    DBPRINTLN(WiFi.localIP());
    lcd.setRGB(colorNo, colorHalf, colorNo);
    lcd.print(WiFi.localIP());
    lcd.setCursor(0,2);
    lcd.print("Dev ID: ");
    lcd.print(DEVID); 
  }
}

// Variables
//long lastMsg = 0;
//char msg[8];  
//int value = 0;

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp; 
  static int r=0,b=0,g=0;
  DBPRINT("Message arrived on topic: ");
  DBPRINTLN(topic);
  DBPRINT("Message: ");
    
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  DBPRINTLN(messageTemp);
  if (String(topic) == String(LED1SUB)) {  
    DBPRINT("Led1 changing output to ->");
    if(messageTemp == "on"){
      DBPRINTLN("on");    
      digitalWrite(led1Pin, HIGH);
    }
    else if(messageTemp == "off"){
      DBPRINTLN("off");
      digitalWrite(led1Pin, LOW);
    }
  }
  if (String(topic) == String(LED2SUB)) {
    DBPRINT("Led2 changing output to ->");
    if(messageTemp == "on"){
      DBPRINTLN("on");    
      digitalWrite(led2Pin, HIGH);
    }
    else if(messageTemp == "off"){
      DBPRINTLN("off");
      digitalWrite(led2Pin, LOW);
    }
  }
  if (String(topic) == String(LED3SUB)) {
    DBPRINT("Led3 changing output to ->");
    if(messageTemp == "on"){
      DBPRINTLN("on");    
      digitalWrite(led3Pin, HIGH);
    }
    else if(messageTemp == "off"){
      DBPRINTLN("off");
      digitalWrite(led3Pin, LOW);
    }
  }
  if (String(topic) == String(NEORSUB)) {
    r=messageTemp.toInt();
    RGB_Strip.clear();
    RGB_Strip.setPixelColor(0,r,g,b);
    RGB_Strip.show();
    DBPRINT("NEO Red Value ->");
    DBPRINTLN(r);
  }
    if (String(topic) == String(NEOGSUB)) {
    g=messageTemp.toInt();
    RGB_Strip.clear();
    RGB_Strip.setPixelColor(0,r,g,b);
    RGB_Strip.show();
    DBPRINT("NEO Geen Value ->");
    DBPRINTLN(messageTemp);
  }
    if (String(topic) == String(NEOBSUB)) {
    b=messageTemp.toInt();
    RGB_Strip.clear();
    RGB_Strip.setPixelColor(0,r,g,b);
    RGB_Strip.show();
    DBPRINT("NEO Blue Value ->");
    DBPRINTLN(messageTemp);
  }
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    lcd.setRGB(colorHalf, colorNo, colorNo); // Display red if not connected to broker
    DBPRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(DEVCLIENT)) {
      DBPRINTLN("connected");
      // Subscribe to channel
      client.subscribe(LED1SUB);
      client.subscribe(LED2SUB);
      client.subscribe(LED3SUB);
      client.subscribe(NEORSUB);
      client.subscribe(NEOGSUB);
      client.subscribe(NEOBSUB);   
    } else {
      DBPRINT("failed, rc=");
      DBPRINTLN(client.state());
      DBPRINTLN("Try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  lcd.setRGB(colorNo, colorHalf, colorNo); // Display geeen if connected to broker
}
void keyscan(void * parameters){
  int sw1State = 0;
  int lastSw1State = 0;
  for(;;)
  {
    sw1State = digitalRead(sw1Pin);
    if (sw1State != lastSw1State){
      if (sw1State==HIGH)
      {
        client.publish(SW1PUB,"on");
        DBPRINTLN("SW1 Pressed");
      } else {
        client.publish(SW1PUB,"off");
      } 
      lastSw1State = sw1State;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);  
  }
}

void measurement(void * parameters){

  char rstr[7]; // Buffer big enough for 6-character float
  for(;;)
  {
    portENTER_CRITICAL (&taskMux);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    portEXIT_CRITICAL(&taskMux);
    if (isnan(h) || isnan(t)){
      DBPRINTLN("Measurement error");
    }
    else { 
      DBPRINT(h);
      DBPRINT(" , ");
      DBPRINTLN(t);
      dtostrf(t, 3, 2, rstr);
      client.publish(TEMPPUB,rstr);
      dtostrf(h, 3, 2, rstr);
      client.publish(HUMPUB,rstr);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void loop()
{
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    //vTaskDelay(10/portTICK_PERIOD_MS);
}

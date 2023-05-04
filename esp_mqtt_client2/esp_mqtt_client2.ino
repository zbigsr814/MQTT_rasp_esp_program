#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// duża płytka esp


const char* ssid = "wifi1";
const char* password = "123123123";

const char* mqtt_server = "192.168.160.125";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

#define ledPin 2

void blink_led(unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW); 
    delay(200);
  }
}

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    blink_led(2,200); 
    delay(1000); 
    Serial.print(".");
    c=c+1;
    if(c>10){
        ESP.restart(); 
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void connect_mqttServer() {
  // Loop until we're reconnected
  while (!client.connected()) {

        if(WiFi.status() != WL_CONNECTED){
          setup_wifi();
        }

        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32_client2")) { 
          Serial.println("connected");
          client.subscribe("rpi/broadcast");
          //client.subscribe("rpi/xyz");
          
        } 
        else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" trying again in 2 seconds");
    
          blink_led(3,200); 
          delay(2000);
        }
  }
  
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "rpi/broadcast") {
      if(messageTemp == "10"){
        Serial.println("Action: blink LED");
        blink_led(1,1250);
      }
  }

}

// Ustawienia czujnika DS18B20
#define ONE_WIRE_BUS 0  // Pin, do którego podłączony jest czujnik DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);

   pinMode(16, INPUT);
}

void loop() {
  
  if (!client.connected()) {
    connect_mqttServer();
  }

  client.loop();
  
  long now = millis();
  if (now - lastMsg > 4000) {
    lastMsg = now;

    sensors.requestTemperatures();  // Wysłanie komendy do czujnika DS18B20 w celu odczytu temperatury
    float temperature = sensors.getTempCByIndex(0);  // Odczytanie wartości temperatury w stopniach Celsiusza
    String strTemp = String(temperature);

    int value = analogRead(A0); // odczyt wartości z A0
    String str = String(value);

    String megaStr = strTemp + "#" + str;

    client.publish("esp32/sensor2", megaStr.c_str());
  }
  
}
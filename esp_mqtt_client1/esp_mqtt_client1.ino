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

// mała płytka esp

// nazwa i hasło do sieci wifi
const char* ssid = "wifi1";
const char* password = "123123123";

// adres IP serwera MQTT
const char* mqtt_server = "192.168.160.125";

// inicjalizacja urządzenia do komunikacji w protokole TCP/IP
WiFiClient espClient;
// inicjalizacja urządzenia do komunikacji w protokole MQTT
PubSubClient client(espClient);

long lastMsg = 0;

#define ledPin 2

// funkcja służąca do migania diodą LED
void blink_led(unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW); 
    delay(200);
  }
}

// funkcja służąca do łączenia z siecią WiFi
void setup_wifi() {
  delay(50);      // stabilizacja układu przed rozpoczęciem połączenia z siecią
  Serial.println();     // pusta linia w monitorze szeregowym
  Serial.print("Connecting to ");     // napis "Connecting to " w monitorze szeregowym
  Serial.println(ssid);     // wypisanie nazwy sieci WiFi w monitorze szeregowym

  WiFi.begin(ssid, password);   // rozpoczęcie połączenia z siecią WiFi

  int c=0;
  // oczekiwanie na połączenie z siecią WiFi
  while (WiFi.status() != WL_CONNECTED) {
    blink_led(2,200); // mruganie diodą kiedy nawiązywane jest połączenie z siecią
    delay(1000); // oczekiwaniy czas na kolejną próbę nawiązania połączenia
    Serial.print(".");    // wizualne śledzenie procesu łączenia z siecią
    c=c+1;      // zliczanie liczby prób połączenia
    if(c>10){   // po 10 sekundach nie nawiązania połączenia, restart ESP
        ESP.restart(); //restart ESP after 10 seconds
    }
  }

  // jeśli połączenie zostanie nawiązane, zakomunikuj to na monitorze szeregownm, wyświetl adres IP ESP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

// funkcja służąca do łączenia z serwerem/brokerem MQTT i subskrybcję tematów
void connect_mqttServer() {
  // sprawdzenie czy połączenie z serwerem MQTT zostało nawiązane
  while (!client.connected()) {

        // sprawdzenie połączenie z siecią WiFi zostało nawiązane
        if(WiFi.status() != WL_CONNECTED){
          setup_wifi();   //nawiąż połączenie z sieczią WiFi
        }

        // wypisanie informacji w monitorze szeregowym
        Serial.print("Attempting MQTT connection...");
        // dokonanie próby połączenia z serwerem MQTT, nadanie nazwy klientowi ESP
        if (client.connect("ESP32_client1")) { 
          // połączenie nawiązane, monitor szeregowy
          Serial.println("connected");
          // subskrybowany temat - broadcast
          client.subscribe("rpi/broadcast");
          // ... możliwość subskrybowania większej ilości tematów
          
        } 
        else {
          // nie udało się połączyć z serwerem MQTT
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" trying again in 2 seconds");
    
          blink_led(3,200); // mruganie diodą
          // czekanie 2 sekundy na ponowną próbę połączenia z serwerem
          delay(2000);
        }
  }
  
}

// funkcja wyświetla na monitorze szeregowym otrzymane dane subskrybowane w określonym temacie
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");  // informacje wyświetlane na monitorze szeregowym
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();     // informacje wyświetlane na monitorze szeregowym

  // odebrane informacje z tematu "rpi/broadcast"
  if (String(topic) == "rpi/broadcast") {
      if(messageTemp == "10"){
        Serial.println("Action: blink LED");
        blink_led(1,1250); //mruganie diodą
      }
  }

  // ... można dodać dodatkowe tematy
}

// Ustawienia czujnika DS18B20
#define ONE_WIRE_BUS 0  // Pin, do którego podłączony jest czujnik DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  pinMode(ledPin, OUTPUT);    // dioda LED
  Serial.begin(115200);     // port szeregowy

  setup_wifi();     // nawiązanie połączenia z siecią WiFi
  client.setServer(mqtt_server,1883); // nawiązanie połączenia z serwerem MQTT
  client.setCallback(callback); // subskrybcja tematów
}

void loop() {
  
  if (!client.connected()) {
    connect_mqttServer();   // nawiązanie połączenia z serwerem MQTT
  }

  client.loop();    // obsługi komunikacji z serwerem MQTT
  
  // co 4 sekundy dokonywany jest pomiar temperatury i wartości z przetwornika ADC
  long now = millis();
  if (now - lastMsg > 4000) {
    lastMsg = now;

    sensors.requestTemperatures();  // Wysłanie komendy do czujnika DS18B20 w celu odczytu temperatury
    float temperature = sensors.getTempCByIndex(0);  // Odczytanie wartości temperatury w stopniach Celsiusza
    String strTemp = String(temperature);

    int value = analogRead(A0); // odczyt wartości z przetwornika ADC
    String str = String(value);

    String megaStr = strTemp + "#" + str;   // tworzenie wiadomości w formacie temperatura#światło

    client.publish("esp32/sensor1", megaStr.c_str());   // publikowanie wiadomości w temacie sensor1
  }
}
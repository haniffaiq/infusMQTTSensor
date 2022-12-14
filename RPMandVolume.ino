#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#define trig 12
#define ech 13
int encoder_pin = 14;
float REV = 0;
int RPM_VALUE;
int PREVIOUS = 0;
int TIME;
char buffer[256];

const char *ssid = "Teh Manis Anget";
const char *password = "gatauapaanguajuga";
const char *mqtt_broker = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *topic = "/tugasAkhir/SensorInfus";  

WiFiClient espClient;
PubSubClient client(espClient);
String data = "";
StaticJsonDocument<256> doc;
void ICACHE_RAM_ATTR INTERRUPT()
{
  REV++;
}

void setup() {
  Serial1.begin(115200);
  Serial.begin (9600);
  
 WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str())) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic, "hello emqx");
  client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
  pinMode(trig, OUTPUT);
  pinMode(ech, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(encoder_pin), INTERRUPT, RISING);
}


long getVolume (){
  long kosong = 704 - 76;
  long waktu, jarak, volume;
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  waktu = pulseIn(ech, HIGH);
  jarak = (waktu/2) * 0.343;
  
  //Serial.print(jarak);
  //Serial.println(" mm");
  volume = kosong - (jarak * 4);
  
  
  
  delay(500);
  return volume;
}

int getSpeed(){
  detachInterrupt(0);                   
  TIME = millis() - PREVIOUS;          
  RPM_VALUE = abs(((REV/TIME) * 1000 - 2 ) * 60);      
  PREVIOUS = millis();                  
  REV = 0;
  //Serial.print(RPM_VALUE);
  //Serial.println("  Tetes/Menit");
  attachInterrupt(1, INTERRUPT, RISING);
  return RPM_VALUE;
  }

void loop() {
 long volume = getVolume();
 int rpm = getSpeed();
 Serial.print(volume);
 Serial.print(" ml");
 Serial.print("     |     ");
 Serial.print(rpm);
 Serial.println("  Tetes/Menit");
String data = "";
StaticJsonDocument<256> doc;
  doc["sensor1"] = volume;
  doc["sensor2"] = rpm;
  char out[128];
  int b =serializeJson(doc, out);
  client.loop();
  client.publish(topic, out);
 
 delay(1000);
}

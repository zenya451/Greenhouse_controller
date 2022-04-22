#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ThingSpeak.h"

#define MSG_BUFFER_SIZE  (50)

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

unsigned long Channel_ID_1 = ;
unsigned long Channel_ID_2 = ;
unsigned long Channel_ID_3 = ;
const char* Write_API_Key_1 = "";
const char* Write_API_Key_2 = "";
const char* Write_API_Key_3 = "";

unsigned long lastMqttMsg = 0;
unsigned long lastThingMsg = 0;

char msg[MSG_BUFFER_SIZE];
char temp1[MSG_BUFFER_SIZE];
char temp2[MSG_BUFFER_SIZE];
char temp3[MSG_BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);



void setup() {
  Serial.begin(115200);
  wifi_setup();
  client.setServer(mqtt_server, 1883);
  ThingSpeak.begin(espClient);
}


void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  MQTT();
  thingspeak();
}


void wifi_setup() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void MQTT() {
  unsigned long nowM = millis();
  if (nowM - lastMqttMsg > 2000) {
    lastMqttMsg = nowM;
    
    //long t1 = (WiFi.RSSI() * -1);
    long t1 = random(-20, 40);
    snprintf (temp1, MSG_BUFFER_SIZE, "%ld", t1);
    client.publish("Topic1", temp1);

    long t2 = random(-20, 40);
    snprintf (temp2, MSG_BUFFER_SIZE, "%ld", t2);
    client.publish("Topic2", temp2);

    long t3 = random(-20, 40);
    snprintf (temp3, MSG_BUFFER_SIZE, "%ld", t3);
    client.publish("Topic3", temp3);
  }
}

void thingspeak() {
  unsigned long nowT = millis();
  if (nowT - lastThingMsg == 20000) {
    int t_1 = random(-20, 40);
    int httpCode_1 = ThingSpeak.writeField(Channel_ID_1, 1, t_1, Write_API_Key_1);
  }
  if (nowT - lastThingMsg == 40000) {
    int t_2 = random(-20, 40);
    int httpCode_2 = ThingSpeak.writeField(Channel_ID_2, 1, t_2, Write_API_Key_2);
  }
  if (nowT - lastThingMsg == 60000) {
    int t_3 = random(-20, 40);
    int httpCode_3 = ThingSpeak.writeField(Channel_ID_3, 1, t_3, Write_API_Key_3);
    lastThingMsg = nowT;
  }
}

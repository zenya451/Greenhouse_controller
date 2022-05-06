#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>

#define MSG_BUFFER_SIZE  (10)
#define ONE_WIRE_BUS D3

uint8_t deviceCount = 0;

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
unsigned long lastUdpMsg = 0;

bool s1 = true;
bool s2 = true;

char msg[MSG_BUFFER_SIZE];
char temp1[MSG_BUFFER_SIZE];
char temp2[MSG_BUFFER_SIZE];
char temp3[MSG_BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress temperatureSensors[3];
WiFiUDP Udp;
ESP8266WebServer http_server(80);

void setup() {
  Serial.begin(115200);
  wifi_setup();
  client.setServer(mqtt_server, 1883);
  ThingSpeak.begin(espClient);
  ArduinoOTA.setHostname("ESP8266");
  ArduinoOTA.begin();
  sensors.begin();
  deviceCount = sensors.getDeviceCount();
  for (uint8_t index = 0; index < deviceCount; index++){
    sensors.getAddress(temperatureSensors[index], index);
  }
  http_server.begin();
  http_server.on("/", handleRoot);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  MQTT();
  thingspeak();
  udp();
  ArduinoOTA.handle();
  http_server.handleClient();
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
    udp();
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
  if (nowM - lastMqttMsg > 1000) {
    lastMqttMsg = nowM;
    
    sensors.requestTemperatures();
    
    //long t1 = (WiFi.RSSI() * -1);
    float t1 = sensors.getTempC(temperatureSensors[0]);
    snprintf (temp1, MSG_BUFFER_SIZE, "%f", t1);
    client.publish("Topic1", temp1);

    float t2 = sensors.getTempC(temperatureSensors[1]);
    snprintf (temp2, MSG_BUFFER_SIZE, "%f", t2);
    client.publish("Topic2", temp2);

    float t3 = sensors.getTempC(temperatureSensors[2]);
    snprintf (temp3, MSG_BUFFER_SIZE, "%f", t3);
    client.publish("Topic3", temp3);
  }
}

void thingspeak() {
  unsigned long nowT = millis();
  if (nowT - lastThingMsg >= 7200000 && s1 == true) {
    sensors.requestTemperatures();
    float t_1 = sensors.getTempC(temperatureSensors[0]);
    int httpCode_1 = ThingSpeak.writeField(Channel_ID_1, 1, t_1, Write_API_Key_1);
    s1 = false;
  }
  if (nowT - lastThingMsg >= 7230000 && s2 == true) {
    sensors.requestTemperatures();
    float t_2 = sensors.getTempC(temperatureSensors[1]);
    int httpCode_2 = ThingSpeak.writeField(Channel_ID_2, 1, t_2, Write_API_Key_2);
    s2 = false;
  }
  if (nowT - lastThingMsg >= 7260000) {
    sensors.requestTemperatures();
    float t_3 = sensors.getTempC(temperatureSensors[2]);
    int httpCode_3 = ThingSpeak.writeField(Channel_ID_3, 1, t_3, Write_API_Key_3);
    lastThingMsg = nowT;
    s1 = true;
    s2 = true;
  }
}

void udp(){
  unsigned long nowU = millis();
  if (nowU - lastUdpMsg > 10000) {
    lastUdpMsg = nowU;
    Serial.print("udp");
    
    sensors.requestTemperatures();
    
    char str1[30] = "Sensor 1: ";
    char str2[30] = "Sensor 2: ";
    char str3[30] = "Sensor 3: ";
    
    float t1 = sensors.getTempC(temperatureSensors[0]);
    snprintf (temp1, MSG_BUFFER_SIZE, "%f", t1);
    float t2 = sensors.getTempC(temperatureSensors[1]);
    snprintf (temp2, MSG_BUFFER_SIZE, "%f", t2);
    float t3 = sensors.getTempC(temperatureSensors[2]);
    snprintf (temp3, MSG_BUFFER_SIZE, "%f", t3);
    
    strcat(str1, temp1);
    strcat(str2, temp2);
    strcat(str3, temp3);
    
    Udp.beginPacket("192.168.0.101", 4210);
    
    Udp.write(str1);
    Udp.endPacket();
    Udp.write(str2);
    Udp.endPacket();
    Udp.write(str3);
    Udp.endPacket();
  }
}

void handleRoot(){
  String webpage;
  webpage = "<html><head><meta charset=utf-8><meta http-equiv=\"refresh\" content=\"2\"><title>ESP</title></head><body><div>Датчик 1: "
            + T(0) + "</div><div>Датчик 2: " + T(1) +"</div><div>Датчик 3: " + T(2) + "</div></body></html>";
  http_server.send(200, "text/html", webpage);
}

String T(int i){
  sensors.requestTemperatures();
  char temp[8];
  float t = sensors.getTempC(temperatureSensors[i]);
  snprintf (temp, 8, "%f", t);
  return temp;
}

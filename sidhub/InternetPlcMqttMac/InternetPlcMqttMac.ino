#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <EEPROM.h>

//json을 위한 설정
StaticJsonDocument<200> doc;
DeserializationError error;
JsonObject root;

char ssid[32] = "";
char password[32] = "";
//const char* ssid = "i2r";
//const char* password = "00000000";

// AP 설정을 위한 변수
const char *softAP_ssid = "ap_";
const char *softAP_password = "00000000";
/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
boolean connect;
unsigned long lastConnectTry = 0;
String sAP_ssid; // mac address를 문자로 기기를 구분하는 기호로 사용
char cAP_ssid[40];

//mqtt 통신 변수
const char* mqtt_server = "broker.mqtt-dashboard.com"; //브로커 주소
const char* outTopic = "/sidhub/outTopic"; // 이름이 중복되지 않게 설정 기록
const char* inTopic = "/sidhub/inTopic"; // 이름이 중복되지 않게 설정 기록
const char* clientName = "";  // setup 함수에서 자동생성
String sChipID; // mac address를 문자로 기기를 구분하는 기호로 사용
char cChipID[40];
int value=0;
WiFiClient espClient;
PubSubClient client(espClient);

//plc 제어 변수
const int led = 2;
String s="";
int P4[4]={0};
int P0[6]={0};
int lastRead=0,lastMqtt=0,autoRead=0;
boolean stringComplete = false; 
String inputString;
char msg[100];

//sidhub 변수
int drive,power,wind; 

ESP8266WebServer server(80);

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
  Serial.begin(9600);
  Serial1.begin(9600);

  //이름 자동으로 생성
  uint8_t chipid[6]="";
  WiFi.macAddress(chipid);
  sprintf(cChipID,"%02x%02x%02x%02x%02x%02x%c",chipid[5], chipid[4], chipid[3], chipid[2], chipid[1], chipid[0],0);
  sChipID=String(cChipID);
  sAP_ssid=String(softAP_ssid)+sChipID;
  sAP_ssid.toCharArray(cAP_ssid,sAP_ssid.length()+1);
  softAP_ssid=&cAP_ssid[0];
  setupAp();
  Serial.println(sChipID);
  Serial.println(softAP_ssid);
  
  server.on("/", handleRoot);
  server.on("/drive", handleDrive);
  server.on("/power", handlePower);
  server.on("/wind", handleWind);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  loadCredentials(); // Load WLAN credentials from network
  if(strlen(ssid) > 0) { // EEPROM에 와이파이 이름 저장 되어 있으면 WLAN 연결
    connectWifi();
  }
}

void setupAp() {
  Serial.println("");
  Serial.println(softAP_ssid);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  //WiFi.mode(WIFI_STA); //이 모드를 설정 않해야 AP가 살아있습니다.
  WiFi.begin(ssid, password);

  int connRes = WiFi.waitForConnectResult();
  Serial.print ( "connRes: " );
  Serial.println ( connRes );
  if(connRes == WL_CONNECTED){
    Serial.println("WiFi well connected");
    Serial.println(WiFi.localIP());
  }
  else
    WiFi.disconnect();
}

// 통신에서 문자가 들어오면 이 함수의 payload 배열에 저장된다.
void callback(char* topic, byte* payload, unsigned int length) {
  /*
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Serial.print((char)payload[i]);
  }
  Serial.println();
  */

  deserializeJson(doc,payload);
  root = doc.as<JsonObject>();
  const char* rChip = root["chip"];
  drive = root["drive"];
  power = root["power"];
  wind = root["wind"];
  if(!rChip)
    return;
  if(strcmp(rChip,cChipID)==0) {
    Serial.print("drive : ");
    Serial.println(drive);
    Serial.print("power : ");
    Serial.println(power);
    Serial.print("wind : ");
    Serial.println(wind);
  }
}

// mqtt 통신에 지속적으로 접속한다.
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(cChipID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Reconnected");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(void) {
  unsigned int sWifi = WiFi.status();
  if (!client.connected() && sWifi==WL_CONNECTED) {
    reconnect();
  }
  client.loop();
  server.handleClient();

  long now = millis();
  //6초에 한번 와이파이 끊기면 다시 연결
  if (sWifi == WL_IDLE_STATUS && now > (lastConnectTry + 60000) && strlen(ssid) > 0 ) {
    lastConnectTry=now;
    Serial.println ( "Connect requested" );
    connectWifi();
  }
  
  if ((now - lastRead > 1000)){
    lastRead = now;
    serialEvent();
    if(autoRead==1) {
      String s="";
      s=char(5);
      s+="00RSS0104%PW0";
      s+=char(4);
      Serial1.print(s);
    }
  }
}

void serialEvent() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    /*
    inputString.toCharArray(msg, inputString.length());
    inputString="{'chip':'"+sChipID+"','power':'"+power+"'} ";
    inputString.toCharArray(msg, inputString.length());
    client.publish(outTopic, msg);
    // clear the string:
    inputString = "";
    */
    stringComplete = false;
   }
  
  if(Serial.available() == false) 
    return;
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    //Serial.print(inChar);
    // add it to the inputString:
    inputString += inChar;
  }
  stringComplete = true;
}

void plcOut() {
  String out;
  String s="";
  s="{\"chip\":\""+sChipID+"\",\"drive\":"+drive+",\"power\":"+power+",\"wind\":"+wind+"} ";
  Serial.println(s);
  s.toCharArray(msg, s.length());
  client.publish(outTopic, msg);
  
}

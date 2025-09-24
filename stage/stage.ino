#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <PubSubClient.h>

// ===== Wi-Fi =====
const char* ssid = "Stage_CTRL";         // 무대 전용 공유기
const char* password = "your_wifi_password";

// ===== HiveMQ Cloud Broker =====
const char* mqtt_server = "7373e0ca890b45a789270b5e6cba0c77.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "KIM_STAGE";
const char* mqtt_pass   = "7142Hanmom!!";

WiFiSSLClient wifiClient;
PubSubClient client(wifiClient);

// ===== RGB 그룹 핀 매핑 (예시) =====
// 1. 스테이지 천정 핀조명 (3개 RGB)
// 2. 전면 중앙 천정 핀조명 (2개 RGB)
// 3. 무대 스트립 RGB (8개 RGB)
// 4. 이동식 스탠딩 RGB (N개 RGB)
int groupPins[4][3] = {
  {2, 3, 4},   // Group1 R,G,B
  {5, 6, 7},   // Group2 R,G,B
  {8, 9, 10},  // Group3 R,G,B
  {11, 12, 13} // Group4 R,G,B
};

void setRGB(int group, int r, int g, int b) {
  if (group < 1 || group > 4) return;
  int idx = group - 1;
  analogWrite(groupPins[idx][0], r);
  analogWrite(groupPins[idx][1], g);
  analogWrite(groupPins[idx][2], b);
  Serial.print("[Group "); Serial.print(group); Serial.print("] ");
  Serial.print("R="); Serial.print(r);
  Serial.print(" G="); Serial.print(g);
  Serial.print(" B="); Serial.println(b);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i=0; i<length; i++) msg += (char)payload[i];
  Serial.print("[MQTT] "); Serial.println(msg);

  if (msg.startsWith("RGB")) {
    int g,r,gc,b;
    sscanf(msg.c_str(), "RGB %d %d %d %d", &g, &r, &gc, &b);
    setRGB(g, r, gc, b);
  }
  else if (msg == "ON12") {
    setRGB(1,255,255,255);
    setRGB(2,255,255,255);
  }
  else if (msg == "OFF12") {
    setRGB(1,0,0,0);
    setRGB(2,0,0,0);
  }
  else if (msg == "ON123") {
    for (int i=1;i<=3;i++) setRGB(i,255,255,255);
  }
  else if (msg == "OFF123") {
    for (int i=1;i<=3;i++) setRGB(i,0,0,0);
  }
  else if (msg == "ON1234") {
    for (int i=1;i<=4;i++) setRGB(i,255,255,255);
  }
  else if (msg == "OFF1234") {
    for (int i=1;i<=4;i++) setRGB(i,0,0,0);
  }
}

void setup_wifi() {
  Serial.print("WiFi Connecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" Connected!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT connecting...");
    if (client.connect("StageR4", mqtt_user, mqtt_pass)) {
      Serial.println(" connected!");
      client.subscribe("stage/rgb");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  for (int g=0; g<4; g++) {
    for (int c=0; c<3; c++) {
      pinMode(groupPins[g][c], OUTPUT);
      analogWrite(groupPins[g][c], 0);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}

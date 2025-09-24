#include <WiFiS3.h>
#include <PubSubClient.h>

// ===== Wi-Fi =====
const char* ssid = "Stage_CTRL";
const char* password = "your_password";

// ===== MQTT =====
const char* mqtt_server = "192.168.0.10";
const int mqtt_port = 1883;
const char* mqtt_topic = "stage/rgb";
const char* mqtt_status = "stage/status"; // 상태 전송용 토픽

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ===== RGB 그룹 핀 =====
struct RGBGroup {
  int R, G, B;
  bool state;
};

RGBGroup groups[4] = {
  {3, 5, 6, false},   
  {9, 10, 11, false}, 
  {A0, A1, A2, false},
  {A3, A4, A5, false}
};

// ===== 함수: RGB 출력 =====
void setRGB(int idx, int r, int g, int b) {
  if (idx < 0 || idx > 3) return;
  if (!groups[idx].state) {
    analogWrite(groups[idx].R, 0);
    analogWrite(groups[idx].G, 0);
    analogWrite(groups[idx].B, 0);
    return;
  }
  analogWrite(groups[idx].R, r);
  analogWrite(groups[idx].G, g);
  analogWrite(groups[idx].B, b);

  Serial.print("[Group ");
  Serial.print(idx+1);
  Serial.print("] R:");
  Serial.print(r);
  Serial.print(" G:");
  Serial.print(g);
  Serial.print(" B:");
  Serial.println(b);
}

// ===== MQTT 콜백 =====
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i=0; i<length; i++) msg += (char)payload[i];

  Serial.print("[MQTT] ");
  Serial.println(msg);

  if (msg.startsWith("SET")) {
    int idx, r,g,b;
    sscanf(msg.c_str(), "SET %d %d %d %d", &idx, &r, &g, &b);
    if (idx >= 1 && idx <= 4) setRGB(idx-1, r,g,b);
  } 
  else if (msg.startsWith("ON")) {
    String groupsStr = msg.substring(3);
    for (int i=0; i<groupsStr.length(); i+=2) {
      int idx = groupsStr.substring(i,i+1).toInt()-1;
      if (idx >=0 && idx<4) groups[idx].state = true;
    }
  } 
  else if (msg.startsWith("OFF")) {
    String groupsStr = msg.substring(4);
    for (int i=0; i<groupsStr.length(); i+=2) {
      int idx = groupsStr.substring(i,i+1).toInt()-1;
      if (idx >=0 && idx<4) {
        groups[idx].state = false;
        setRGB(idx,0,0,0);
      }
    }
  }
}

// ===== Wi-Fi 연결 =====
void setup_wifi() {
  Serial.print("WiFi Connecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

// ===== MQTT 재연결 =====
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("StageRGB")) {
      Serial.println("MQTT connected");
      client.subscribe(mqtt_topic);
      client.publish(mqtt_status, "ONLINE"); // 연결 시 상태 전송
    } else {
      Serial.println("MQTT connect failed, retrying...");
      delay(5000);
    }
  }
}

// ===== setup =====
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  for (int i=0; i<4; i++) {
    pinMode(groups[i].R, OUTPUT);
    pinMode(groups[i].G, OUTPUT);
    pinMode(groups[i].B, OUTPUT);
    groups[i].state = false;
    setRGB(i,0,0,0);
  }
  Serial.println("Arduino UNO R4 Stage Lighting Ready!");
  Serial.println("명령 예시: ON 1, OFF 2, SET 3 200 150 100");
}

// ===== loop =====
unsigned long lastStatus = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
    client.publish(mqtt_status, "OFFLINE"); 
  }
  client.loop();

  // === 시리얼 명령 처리 ===
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    Serial.print("[Serial CMD] ");
    Serial.println(cmd);

    if (cmd.startsWith("ON")) {
      int idx = cmd.substring(3).toInt();
      if (idx>=1 && idx<=4) groups[idx-1].state = true;
    } 
    else if (cmd.startsWith("OFF")) {
      int idx = cmd.substring(4).toInt();
      if (idx>=1 && idx<=4) {
        groups[idx-1].state = false;
        setRGB(idx-1,0,0,0);
      }
    }
    else if (cmd.startsWith("SET")) {
      int idx,r,g,b;
      sscanf(cmd.c_str(), "SET %d %d %d %d", &idx,&r,&g,&b);
      if (idx>=1 && idx<=4) {
        groups[idx-1].state = true;
        setRGB(idx-1,r,g,b);
      }
    }
  }

  // === 5초마다 상태 전송 ===
  unsigned long now = millis();
  if (now - lastStatus > 5000) {
    if (client.connected()) {
      client.publish(mqtt_status, "ONLINE");
      Serial.println("Status: ONLINE");
    } else {
      Serial.println("Status: OFFLINE");
    }
    lastStatus = now;
  }
}

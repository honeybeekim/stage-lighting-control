#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <PubSubClient.h>

// ===== Wi-Fi 정보 =====
const char* ssid = "Stage_CTRL";         // 공유기 SSID
const char* password = "your_wifi_password";  // 공유기 비밀번호

// ===== HiveMQ Cloud Broker =====
const char* mqtt_server = "7373e0ca890b45a789270b5e6cba0c77.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "KIM_STAGE";
const char* mqtt_pass   = "7142Hanmom!!";

WiFiSSLClient wifiClient;   // TLS 보안 연결
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i=0; i<length; i++) msg += (char)payload[i];
  Serial.print("[MQTT] "); Serial.println(msg);
}

void setup_wifi() {
  Serial.print("WiFi Connecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
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
      Serial.print(" failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}

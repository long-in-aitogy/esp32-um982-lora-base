#include "functions/MQTT_Manager.h"
#include "Top_Lvl_Config.h"
#include "Prog_Config.h"
#include "functions/cmd_handler.h"

// ================= ĐỊNH NGHĨA CÁC ĐỐI TƯỢNG CẦN CHO KẾT NỐI =================
#if CONNECT_USING_WIFI
#include "hardware/Wifi_handler.h"
static WiFiClient espClient;
#endif
#if CONNECT_USING_4G
#include "hardware/Sim_handler.h"
extern TinyGsm modem;
static TinyGsmClient espClient(modem, 1);
#endif
PubSubClient mqtt(espClient);

// ================= ĐỊNH NGHĨA HÀM =================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_SUB_CMD) == 0) {
    String cmd = "";
    for (int i = 0; i < length; i++) cmd += (char)payload[i];
    
    Serial.print("\n[MQTT DOWNLINK] Lenh: ");
    Serial.println(cmd);

    if (cmd.isEmpty()) {
      Serial.println("[MQTT DOWNLINK] Lenh rong, khong xu ly.");
      return;
    }
    
    // Đẩy lệnh xuống UM980 qua Serial1
    std::vector<String> cmdWords = splitCommand(cmd);
    cmd_action_t action = handleCommand(cmdWords);
    String gnssResponse = "";
    switch (action) {
      case CMD_ACTION_PASS_TO_GNSS_MODULE:
        Serial.println("[MQTT DOWNLINK] Gui lenh den UM980 qua Serial1");
        for (const auto& word : cmdWords) {
          Serial1.print(word);
          Serial1.print(" ");
        }
        Serial1.print("\r\n");

        gnssResponse = Serial1.readStringUntil('\n');
        Serial.print("[UM980 RESPONSE] ");
        Serial.println(gnssResponse);
        break;

      case CMD_ACTION_ESP_RESTART:
        Serial.println("[ESP32] Khoi dong lai ESP32...");
        ESP.restart();
        break;

      default:
        break;
    }
  }
}

int setupMQTT() {
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  return 0;
}

int connectMQTT() {
  if (!mqtt.connected()) {
    Serial.println("\n[MQTT] Dang ket noi Broker...");
    String clientId = "ESP32_GW_" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("[MQTT] Da ket noi thanh cong!");
      mqtt.subscribe(TOPIC_SUB_CMD);
      return 0;
    } else {
      Serial.print("[MQTT] Loi rc=");
      Serial.print(mqtt.state());
      Serial.println(" -> Thu lai sau 5s");
      return -1;
    }
  }
  return 0;
}

int publishRaw(const String& payload) {
  if (mqtt.connected() && payload.length() > 0) {
    mqtt.publish(TOPIC_PUB_RAW_RTCM, payload.c_str());
    Serial.println("[UM982 GNSS RAW CORRECTION DATA] Da publish thanh cong !");
    return 0;
  }
  return -1;
}

int publishHealth(const String& payload) {
  mqtt.publish(TOPIC_PUB_HEALTH, payload.c_str());
  Serial.print("[MQTT] Da gui thong tin suc khoe len topic: ");
  Serial.println(TOPIC_PUB_HEALTH);
  return 0;
}

bool isMqttConnected() {
  return mqtt.connected();
}
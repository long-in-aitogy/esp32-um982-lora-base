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
    
    #if PROGRAM_DEBUG
    Serial.print("\n[MQTT DOWNLINK] Lenh: ");
    Serial.println(cmd);
    #endif

    if (cmd.isEmpty()) {
      #if PROGRAM_DEBUG
      Serial.println("[MQTT DOWNLINK] Lenh rong, khong xu ly.");
      #endif
      return;
    }
    
    // Đẩy lệnh xuống UM980 qua Serial1
    std::vector<String> cmdWords = splitCommand(cmd);
    cmd_action_t action = handleCommand(cmdWords);
    String gnssResponse = "";
    switch (action) {
      case CMD_ACTION_PASS_TO_GNSS_MODULE:
        Serial.println("[MQTT DOWNLINK] Gui lenh den UM980 qua Serial1");
        Serial.flush();
        for (const auto& word : cmdWords) {
          Serial1.print(word);
          Serial1.print(" ");
        }
        Serial1.print("\r\n");

        #if PROGRAM_DEBUG
        gnssResponse = Serial1.readStringUntil('\n');
        Serial.print("[UM980 RESPONSE] ");
        Serial.println(gnssResponse);
        #endif

        Serial1.print("SAVECONFIG\r\n");

        #if PROGRAM_DEBUG
        gnssResponse = Serial1.readStringUntil('\n');
        Serial.print("[UM980 RESPONSE] ");
        Serial.println(gnssResponse);
        #endif
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
  if (payload.isEmpty()) return -1;

  // Wait for MQTT connection (timeout after 5s)
  const uint32_t start = millis();
  while (!mqtt.connected()) {
    vTaskDelay(pdMS_TO_TICKS(100));
    if (millis() - start > 5000) {
      Serial.println("[MQTT] publishRaw: MQTT not connected, aborting publish");
      return -1;
    }
  }

  // Take tcpStreamMutex before publishing to avoid concurrent network ops
  if (tcpStreamMutex != nullptr && xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
    bool ok = mqtt.publish(TOPIC_PUB_RAW_RTCM, payload.c_str());
    xSemaphoreGive(tcpStreamMutex);
    if (ok) {
      Serial.println("[UM982 GNSS RAW CORRECTION DATA] Da publish thanh cong !");
      return 0;
    }
    return -1;
  }
  return -1;
}

int publishHealth(const String& payload) {
  if (payload.isEmpty()) return -1;

  const uint32_t start = millis();
  while (!mqtt.connected()) {
    vTaskDelay(pdMS_TO_TICKS(100));
    if (millis() - start > 5000) {
      #if PROGRAM_DEBUG
      Serial.println("[MQTT] publishHealth: MQTT not connected, aborting publish");
      #endif
      return -1;
    }
  }

  if (tcpStreamMutex != nullptr && xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
    bool ok = mqtt.publish(TOPIC_PUB_HEALTH, payload.c_str());
    xSemaphoreGive(tcpStreamMutex);
    if (ok) {
      #if PROGRAM_DEBUG
      Serial.print("[MQTT] Da gui thong tin suc khoe len topic: ");
      Serial.println(TOPIC_PUB_HEALTH);
      #endif
      return 0;
    }
    return -1;
  }
  return -1;
}

bool isMqttConnected() {
  return mqtt.connected();
}
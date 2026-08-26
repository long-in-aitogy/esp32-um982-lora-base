#include "functions/MQTT_Manager.h"
#include "Top_Lvl_Config.h"
#include "Prog_Config.h"
#include "functions/cmd_handler.h"
#include <Preferences.h>
#include "helper.h"

// ================= ĐỊNH NGHĨA CÁC ĐỐI TƯỢNG CẦN CHO KẾT NỐI =================
#if CONNECT_USING_WIFI
#include "hardware/Wifi_handler.h"
static WiFiClient espClient;
#endif
#if CONNECT_USING_4G
#include "hardware/Sim_handler.h"
extern TinyGsm modem;
static TinyGsmClient espClient(modem, 1);
extern TinyGsmClient ntripClient;
#endif
PubSubClient mqtt(espClient);

extern Preferences prefs;

static String mqttServerHost;
static uint16_t mqttServerPort = MQTT_PORT;

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
    if (action == CMD_ACTION_ESP_RESTART) {
      Serial.println("[MQTT DOWNLINK] Yeu cau ESP32 khoi dong lai.");
      shutdownTcpTransportBeforeRestart();
      ESP.restart();
    }
  }
}

int setupMQTT() {
  prefs.begin("myPrefs", false);
  mqttServerHost = prefs.getString("MQTT_SERVER", String(MQTT_SERVER));
  mqttServerPort = prefs.getUShort("MQTT_PORT", MQTT_PORT);
  prefs.end();
  mqttServerHost.toCharArray(mqttServerHostBuffer, MQTT_SERVER_HOST_BUFFER_SIZE);
  mqtt.setServer(mqttServerHostBuffer, mqttServerPort);
  mqtt.setCallback(mqttCallback);
  return 0;
}

int connectMQTT() {
  if (!mqtt.connected()) {
    Serial.println("\n[MQTT] Dang ket noi Broker...");
    String clientId = "ESP32_GW_" + String(random(0xffff), HEX);
    prefs.begin("myPrefs", false);
    String mqttUser = prefs.getString("MQTT_USER", String(MQTT_USER));
    String mqttPass = prefs.getString("MQTT_PASS", String(MQTT_PASS));
    String topicSubCmd = prefs.getString("TPC_SUB_CMD", String(TOPIC_SUB_CMD));
    prefs.end();
    if (mqtt.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str())) {
      Serial.println("[MQTT] Da ket noi thanh cong!");
      mqtt.subscribe(topicSubCmd.c_str());
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

int publishRaw(const uint8_t* payload, size_t length) {
  if (payload == nullptr || length == 0) return -1;

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
    prefs.begin("myPrefs", false);
    String topicPubRaw = prefs.getString("TPC_RAW_RTCM", String(TOPIC_PUB_RAW_RTCM));
    prefs.end();
    bool ok = mqtt.publish(topicPubRaw.c_str(), payload, static_cast<unsigned int>(length));
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
    prefs.begin("myPrefs", false);
    String topicPubHealth = prefs.getString("TPC_HEALTH", String(TOPIC_PUB_HEALTH));
    prefs.end();
    bool ok = mqtt.publish(topicPubHealth.c_str(), payload.c_str());
    xSemaphoreGive(tcpStreamMutex);
    if (ok) {
      #if PROGRAM_DEBUG
      Serial.print("[MQTT] Da gui thong tin suc khoe len topic: ");
      Serial.println(topicPubHealth);
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

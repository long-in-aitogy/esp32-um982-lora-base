#include "helper.h"
#include "functions/cmd_handler.h"

extern String latestRtcm;

namespace {
    constexpr size_t SERIAL_COMMAND_MAX_LENGTH = 256;
    String serialCommandBuffer;

    inline void executeSerialCommand(String command) {
        command.trim();
        if (command.isEmpty()) {
            return;
        }

        Serial.println("[SERIAL COMMAND] " + command);
        std::vector<String> cmdWords = splitCommand(command);
        if (handleCommand(cmdWords) == CMD_ACTION_ESP_RESTART) {
            Serial.println("[SERIAL COMMAND] Yeu cau ESP32 khoi dong lai.");
            shutdownTcpTransportBeforeRestart();
            ESP.restart();
        }
    }
}

void processPendingSerialCommands() {
    while (Serial.available() > 0) {
        const char character = static_cast<char>(Serial.read());

        if (character == '\r') {
            continue;
        }

        if (character == '\n') {
            executeSerialCommand(serialCommandBuffer);
            serialCommandBuffer = "";
            continue;
        }

        if (serialCommandBuffer.length() < SERIAL_COMMAND_MAX_LENGTH) {
            serialCommandBuffer += character;
        }
    }
}

void shutdownTcpTransportBeforeRestart() {
    Serial.println("[SETUP] Dong cac ket noi TCP va GPRS truoc khi khoi dong lai...");
    mqtt.disconnect();
#if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
    activeNtripClient().stop();
#endif
    if (isGsmConnection() && modem.isGprsConnected()) {
        modem.gprsDisconnect();
    }
    if (isWifiConnection()) {
        WiFi.disconnect();
        delay(500);
    }
}

String formDeviceHealthString([[maybe_unused]] int32_t signalQualityDbm)
{
    // 1. Lấy các thông số hệ thống
    unsigned long uptime_s = millis() / 1000;
    uint32_t freeHeap = ESP.getFreeHeap();

    const int32_t rssi = isWifiConnection() ? WiFi.RSSI() : signalQualityDbm;
    const String connected_via = isWifiConnection() ? "WiFi" : "GSM";

    bool mqttOk = isMqttConnected();
#if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
    bool ntripOk = isNtripConnected();
#else
    // Nếu dùng LoRa thì không có NTRIP qua TCP/IP, sẽ có cách khác để kiểm tra. Hiện chưa có mã nguồn cho LoRa nên tạm thời để false.
    bool ntripOk = false;
#endif
    bool gnssOk = (latestRtcm.length() > 10); // Nếu có chuỗi NMEA hợp lệ

    // 2. Đóng gói thành JSON
    std::string healthPayload = "{";
    healthPayload += "\"uptime_s\":" + std::to_string(uptime_s);
    healthPayload += ",\"free_heap_bytes\":" + std::to_string(freeHeap);
    healthPayload += R"(,"connected_via":")" + std::string(connected_via.c_str()) + "\"";
    healthPayload += ",\"rssi_dbm\":" + std::to_string(rssi);
    healthPayload += ",\"mqtt_ok\":" + std::string(mqttOk ? "true" : "false");
    healthPayload += ",\"ntrip_ok\":" + std::string(ntripOk ? "true" : "false");
    healthPayload += ",\"gnss_data_ok\":" + std::string(gnssOk ? "true" : "false");
    healthPayload += "}";
    // 3. Trả về payload để có thể log hoặc dùng cho mục đích khác nếu cần
    return String(healthPayload.c_str());
}

#include "functions/cmd_handler.h"
#include <vector>
#include <Preferences.h>

extern Preferences prefs;

std::vector<String> splitCommand(const String &command) {
    std::vector<String> cmdWords;

    int start = 0;
    const int commandLength = command.length();

    while (start <= commandLength) {
        const int end = command.indexOf(' ', start);
        String word = end == -1 ? command.substring(start) : command.substring(start, end);
        word.trim();

        if (!word.isEmpty()) {
            cmdWords.emplace_back(word);
        }

        if (end == -1) {
            break;
        }

        start = end + 1;
    }

    return cmdWords;
}

cmd_action_t handleCommand(std::vector<String> &cmdWords) {
    if (cmdWords.empty() || cmdWords[0] != "ATG") {
        Serial.println("Error: Empty or invalid command format. Command must start with 'ATG'.");
        return CMD_ACTION_NONE;
    }
    if (cmdWords[1] == "UM") {
        cmdWords.erase(cmdWords.begin(), cmdWords.begin() + 2);

        return CMD_ACTION_PASS_TO_GNSS_MODULE;
    }    
    if (cmdWords[1] == "ESP") {
        cmdWords.erase(cmdWords.begin(), cmdWords.begin() + 2);
        if (cmdWords[0] == "AT+RST") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau khoi dong lai ESP32");
            return CMD_ACTION_ESP_RESTART;
        }
        if (cmdWords[0] == "SET" && cmdWords[1] == "GNSS" && cmdWords[2] == "TX") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau cau hinh GNSS TX");
            prefs.begin("myPrefs", false);
            int gnssTX = cmdWords[3].toInt();
            prefs.putInt("GNSS_TX", gnssTX);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[0] == "SET" && cmdWords[1] == "GNSS" && cmdWords[2] == "RX") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau cau hinh GNSS RX");
            prefs.begin("myPrefs", false);
            int gnssRX = cmdWords[3].toInt();
            prefs.putInt("GNSS_RX", gnssRX);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[0] == "SET" && cmdWords[1] == "4G" && cmdWords[2] == "APN") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau cau hinh 4G APN");
            prefs.begin("myPrefs", false);
            String apn = cmdWords[3];
            prefs.putString("APN", apn);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[0] == "SET" && cmdWords[1] == "4G" && cmdWords[2] == "USER") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau cau hinh 4G USER");
            prefs.begin("myPrefs", false);
            String user = cmdWords[3];
            prefs.putString("GPRS_USER", user);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[0] == "SET" && cmdWords[1] == "4G" && cmdWords[2] == "PASS") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau cau hinh 4G PASS");
            prefs.begin("myPrefs", false);
            String pass = cmdWords[3];
            prefs.putString("GPRS_PASS", pass);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        Serial.println("This command is not yet implemented.");
        return CMD_ACTION_NONE;
    }
    if (cmdWords[1] == "MQTT") {
        if (cmdWords[2] == "SET" && cmdWords[3] == "SERVER") {
            prefs.begin("myPrefs", false);
            String serverAddress = cmdWords[4];
            prefs.putString("MQTT_SERVER", serverAddress);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "PORT") {
            prefs.begin("myPrefs", false);
            auto port = (uint16_t)(cmdWords[4].toInt());
            prefs.putUShort("MQTT_PORT", port);
            prefs.end();    
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "USER") {
            prefs.begin("myPrefs", false);
            String user = cmdWords[4];
            prefs.putString("MQTT_USER", user);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "PASS") {
            prefs.begin("myPrefs", false);
            String pass = cmdWords[4];
            prefs.putString("MQTT_PASS", pass);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "PUBTPCHEALTH") {
            prefs.begin("myPrefs", false);
            String topic = cmdWords[4];
            prefs.putString("TPC_HEALTH", topic);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "PUBTPCRAW") {
            prefs.begin("myPrefs", false);
            String topic = cmdWords[4];
            prefs.putString("TPC_RAW_RTCM", topic);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "SUBTPCCMD") {
            prefs.begin("myPrefs", false);
            String topic = cmdWords[4];
            prefs.putString("TPC_SUB_CMD", topic);
            prefs.end();
            return CMD_ACTION_NONE;
        }
    }
    if (cmdWords[1] == "NTRIP") {
        if (cmdWords[2] == "SET" && cmdWords[3] == "CSTRADDR") {
            prefs.begin("myPrefs", false);
            String serverAddress = cmdWords[4];
            prefs.putString("NTRIP_SERVER", serverAddress);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "CSTRPORT") {
            prefs.begin("myPrefs", false);
            auto port = (uint16_t)(cmdWords[4].toInt());
            prefs.putUShort("NTRIP_PORT", port);
            prefs.end();    
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "MNTPNT") {
            prefs.begin("myPrefs", false);
            String mountPoint = cmdWords[4];
            prefs.putString("NTRIP_MPT", mountPoint);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "SET" && cmdWords[3] == "CSTRAUTH") {
            prefs.begin("myPrefs", false);
            String auth = cmdWords[4];
            prefs.putString("NT_AUTH_BS", auth);
            prefs.end();
            return CMD_ACTION_NONE;
        }
    }
    if (cmdWords[1] == "CONFIG") {
        if (cmdWords[2] == "RESET") {
            Serial.println("[MQTT DOWNLINK] Lenh yeu cau reset ve cau hinh mac dinh");
            prefs.begin("myPrefs", false);
            prefs.putBool("NEED_RESET", true);
            prefs.end();
            return CMD_ACTION_ESP_RESTART;
        }
    }
    Serial.println("Unknown command: " + cmdWords[1]);
    return CMD_ACTION_NONE;
}
#include "functions/cmd_handler.h"
#include <vector>
#include <Preferences.h>
#include "functions/ubx_cmd_builder.h"

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

cmd_action_t handleGnssBaseCommand(const std::vector<String> &cmdWords) {
    UbxCmdBuilder::GnssOptions options;
    UbxCmdBuilder::CommandList commands;
    UbxCmdBuilder::Command commandsBytes;

    if (cmdWords[0] == "SURVEY_IN") {
        Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh GNSS BASE SURVEY_IN");
        if (cmdWords.size() < 3) {
            Serial.println("[MQTT COMMAND DOWNLINK] Error: Lenh GNSS BASE SURVEY_IN khong day du.");
            return CMD_ACTION_NONE;
        }
        else if (cmdWords.size() > 3) {
            Serial.println("[MQTT COMMAND DOWNLINK] Error: Lenh qua dai !");
            return CMD_ACTION_NONE;
        }
        uint32_t duration = cmdWords[1].toInt();
        float accuracy = cmdWords[2].toFloat();
        
        commands = UbxCmdBuilder::buildBaseSurveyInCommand(duration, accuracy, options);
        commandsBytes = UbxCmdBuilder::commandListToBytes(commands);

        for (const auto &byte : commandsBytes) {
            Serial1.write(byte);
        }
        Serial1.flush();
        Serial.println("[MQTT COMMAND DOWNLINK] Da gui lenh cau hinh GNSS BASE SURVEY_IN.");

        return CMD_ACTION_NONE;
    } else if (cmdWords[0] == "FIXED") {
        Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh GNSS FIXED LLA");
        if (cmdWords.size() < 5) {
            Serial.println("[MQTT COMMAND DOWNLINK] Error: Lenh khong day du !");
            return CMD_ACTION_NONE;
        }
        else if (cmdWords.size() > 5) {
            Serial.println("[MQTT COMMAND DOWNLINK] Error: Lenh qua dai !");
            return CMD_ACTION_NONE;
        }
        double lat = cmdWords[1].toDouble();
        double lon = cmdWords[2].toDouble();
        double alt = cmdWords[3].toDouble();
        float accuracy = cmdWords[4].toFloat();

        commands = UbxCmdBuilder::buildBaseFixedLlaCommand(lat, lon, alt, accuracy, options);

        commandsBytes = UbxCmdBuilder::commandListToBytes(commands);

        for (const auto &byte : commandsBytes) {
            Serial1.write(byte);
        }
        Serial1.flush();
        Serial.println("[MQTT COMMAND DOWNLINK] Da gui lenh cau hinh GNSS BASE FIXED LLA.");

        return CMD_ACTION_NONE;
    } else {
        Serial.println("[MQTT COMMAND DOWNLINK] LOI - Lenh khong kha dung: " + cmdWords[0]);
        return CMD_ACTION_NONE;
    }
}

cmd_action_t handleEspCommand(const std::vector<String> &cmdWords) {
    if (cmdWords[0] == "AT+RST") {
        Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau khoi dong lai ESP32");
        return CMD_ACTION_ESP_RESTART;
    }
    prefs.begin("myPrefs", false);
    if (cmdWords[0] == "SET") {
        if (cmdWords[1] == "GNSS" && cmdWords[2] == "TX") {
            Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh GNSS TX");
            int gnssTX = cmdWords[3].toInt();
            prefs.putInt("GNSS_TX", gnssTX);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[1] == "GNSS" && cmdWords[2] == "RX") {
            Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh GNSS RX");
            prefs.begin("myPrefs", false);
            int gnssRX = cmdWords[3].toInt();
            prefs.putInt("GNSS_RX", gnssRX);
            prefs.end();
            return CMD_ACTION_NONE;
        }
        if (cmdWords[1] == "4G" && cmdWords[2] == "APN") {
            Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh 4G APN");
            prefs.begin("myPrefs", false);
            String apn = cmdWords[3];
            prefs.putString("APN", apn);
            return CMD_ACTION_NONE;
        }
        if (cmdWords[1] == "4G" && cmdWords[2] == "USER") {
            Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh 4G USER");
            prefs.begin("myPrefs", false);
            String user = cmdWords[3];
            prefs.putString("GPRS_USER", user);
            return CMD_ACTION_NONE;
        }
        if (cmdWords[1] == "4G" && cmdWords[2] == "PASS") {
            Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau cau hinh 4G PASS");
            prefs.begin("myPrefs", false);
            String pass = cmdWords[3];
            prefs.putString("GPRS_PASS", pass);
            return CMD_ACTION_NONE;
        }
        prefs.end();
    }
    
    Serial.println("[MQTT COMMAND DOWNLINK] This command is not yet implemented.");
    return CMD_ACTION_NONE;
}

cmd_action_t handleMqttCommand(const std::vector<String> &cmdWords) {
    if (cmdWords[0] == "SET" && cmdWords[1] == "SERVER") {
        prefs.begin("myPrefs", false);
        String serverAddress = cmdWords[2];
        prefs.putString("MQTT_SERVER", serverAddress);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "PORT") {
        prefs.begin("myPrefs", false);
        auto port = (uint16_t)(cmdWords[2].toInt());
        prefs.putUShort("MQTT_PORT", port);
        prefs.end();    
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "USER") {
        prefs.begin("myPrefs", false);
        String user = cmdWords[2];
        prefs.putString("MQTT_USER", user);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "PASS") {
        prefs.begin("myPrefs", false);
        String pass = cmdWords[2];
        prefs.putString("MQTT_PASS", pass);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "PUBTPCHEALTH") {
        prefs.begin("myPrefs", false);
        String topic = cmdWords[2];
        prefs.putString("TPC_HEALTH", topic);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "PUBTPCRAW") {
        prefs.begin("myPrefs", false);
        String topic = cmdWords[2];
        prefs.putString("TPC_RAW_RTCM", topic);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "SUBTPCCMD") {
        prefs.begin("myPrefs", false);
        String topic = cmdWords[2];
        prefs.putString("TPC_SUB_CMD", topic);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    Serial.println("[MQTT COMMAND DOWNLINK] Lenh sai hoac khong kha dung.");
    return CMD_ACTION_NONE;
}

cmd_action_t handleNtripCommand(const std::vector<String> &cmdWords) {
    if (cmdWords[0] == "SET" && cmdWords[1] == "CSTRADDR") {
        prefs.begin("myPrefs", false);
        String serverAddress = cmdWords[2];
        prefs.putString("NTRIP_SERVER", serverAddress);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "CSTRPORT") {
        prefs.begin("myPrefs", false);
        auto port = (uint16_t)(cmdWords[2].toInt());
        prefs.putUShort("NTRIP_PORT", port);
        prefs.end();    
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "MNTPNT") {
        prefs.begin("myPrefs", false);
        String mountPoint = cmdWords[2];
        prefs.putString("NTRIP_MPT", mountPoint);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    if (cmdWords[0] == "SET" && cmdWords[1] == "CSTRAUTH") {
        prefs.begin("myPrefs", false);
        String auth = cmdWords[2];
        prefs.putString("NT_AUTH_BS", auth);
        prefs.end();
        return CMD_ACTION_NONE;
    }
    return CMD_ACTION_NONE;
}

cmd_action_t handleCommand(std::vector<String> &cmdWords) {
    if (cmdWords.empty() || cmdWords[0] != "ATG") {
        Serial.println("[MQTT COMMAND DOWNLINK] Error: Empty or invalid command format. Command must start with 'ATG'.");
        return CMD_ACTION_NONE;
    }
    if (cmdWords[1] == "GNSS") {
        if (cmdWords.size() < 3) {
            Serial.println("[MQTT COMMAND DOWNLINK] Error: Incomplete GNSS command.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[2] == "BASE") {
            cmdWords.erase(cmdWords.begin(), cmdWords.begin() + 3);
            return handleGnssBaseCommand(cmdWords);
        }
        Serial.println("[MQTT COMMAND DOWNLINK] Error: Unknown GNSS command: " + cmdWords[2]);
        return CMD_ACTION_NONE;
    }    
    if (cmdWords[1] == "ESP") {
        cmdWords.erase(cmdWords.begin(), cmdWords.begin() + 2);
        return handleEspCommand(cmdWords);
    }
    if (cmdWords[1] == "MQTT") {
        cmdWords.erase(cmdWords.begin(), cmdWords.begin() + 2);
        return handleMqttCommand(cmdWords);
    }
    if (cmdWords[1] == "NTRIP") {
        cmdWords.erase(cmdWords.begin(), cmdWords.begin() + 2);
        return handleNtripCommand(cmdWords);
    }
    if (cmdWords[1] == "CONFIG") {
        if (cmdWords[2] == "RESET") {
            Serial.println("[MQTT COMMAND DOWNLINK] Lenh yeu cau reset ve cau hinh mac dinh");
            prefs.begin("myPrefs", false);
            prefs.putBool("NEED_RESET", true);
            prefs.end();
            return CMD_ACTION_ESP_RESTART;
        }
    }
    Serial.println("[MQTT COMMAND DOWNLINK] Unknown command: " + cmdWords[1]);
    return CMD_ACTION_NONE;
}
#include "functions/cmd_handler.h"
#include <vector>

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
        Serial.println("This command is not yet implemented.");
        return CMD_ACTION_NONE;
    }
    if (cmdWords[2] == "MQTT") {
        if (cmdWords[3] == "SET" && cmdWords[4] == "SERVER") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[3] == "SET" && cmdWords[4] == "PUBTPCHEALTH") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[3] == "SET" && cmdWords[4] == "PUBTPCRAW") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[3] == "SET" && cmdWords[4] == "SUBTPCCMD") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
    }
    if (cmdWords[2] == "NTRIP") {
        if (cmdWords[3] == "SET" && cmdWords[4] == "CSTRADDR") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[3] == "SET" && cmdWords[4] == "CSTRPORT") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[3] == "SET" && cmdWords[4] == "MNTPNT") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
        if (cmdWords[3] == "SET" && cmdWords[4] == "CSTRAUTH") {
            // Will be implemented later
            Serial.println("This command is not yet implemented.");
            return CMD_ACTION_NONE;
        }
    }
    Serial.println("Unknown command: " + cmdWords[1]);
    return CMD_ACTION_NONE;
}
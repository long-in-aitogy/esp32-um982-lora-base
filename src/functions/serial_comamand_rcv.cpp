#include "functions/serial_comamand_rcv.h"
#include "functions/cmd_handler.h"
#include "helper.h"

__attribute__((noreturn)) void serial_listener(void* parameter) {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(20));
        if (Serial.available()) {
            String command = Serial.readStringUntil('\n');
            command.trim(); // Loại bỏ khoảng trắng ở đầu và cuối
            if (command.isEmpty()) {
                continue;
            }
            vTaskDelay(pdMS_TO_TICKS(20));
            Serial.println("[SERIAL COMMAND] Nhận lệnh: " + command);
            std::vector<String> cmdWords = splitCommand(command);
            cmd_action_t action = handleCommand(cmdWords);
            if (action == CMD_ACTION_ESP_RESTART) {
                Serial.println("[SERIAL COMMAND] Khởi động lại ESP32...");
                shutdownTcpTransportBeforeRestart();
                ESP.restart();
            }
        }
    }
}
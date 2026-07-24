#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#include <Arduino.h>
#include <vector>

typedef enum cmd_action {
    CMD_ACTION_NONE = 0,
    CMD_ACTION_PASS_TO_GNSS_MODULE = 1,
    CMD_ACTION_ESP_RESTART = 2
} cmd_action_t;

std::vector<String> splitCommand(const String &command);

cmd_action_t handleCommand(std::vector<String> &cmdWords);

#endif // CMD_HANDLER_H
#include "functions/cmd_handler.h"

static String commandWords[10];

void handleCommand(const String &command) {
    // Split the command into words
    std::string delimiter = "+";
    size_t last = 0;
    size_t next = 0;
}
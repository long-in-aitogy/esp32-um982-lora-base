#ifndef CONNECTION_TYPE_H
#define CONNECTION_TYPE_H

#include <Arduino.h>

enum class ConnectionType : uint8_t {
    WIFI,
    GSM,
};

// Loaded once during setup from Preferences key CONNECTION_TYPE.
extern ConnectionType connectionType;

void loadConnectionTypeFromPrefs();

inline bool isWifiConnection() {
    return connectionType == ConnectionType::WIFI;
}

inline bool isGsmConnection() {
    return connectionType == ConnectionType::GSM;
}

#endif
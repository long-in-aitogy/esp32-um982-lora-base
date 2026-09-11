#include "Connection_Type.h"
#include "Prog_Config.h"

ConnectionType connectionType = ConnectionType::GSM;

void loadConnectionTypeFromPrefs() {
    const String configuredType = prefs.getString("CONNECTION_TYPE", "4G");

    if (configuredType.equalsIgnoreCase("WIFI")) {
        connectionType = ConnectionType::WIFI;
        return;
    }

    if (!configuredType.equalsIgnoreCase("4G")) {
        Serial.println("[SETUP][WARN] CONNECTION_TYPE khong hop le: " + configuredType + ". Su dung 4G.");
    }
    connectionType = ConnectionType::GSM;
}

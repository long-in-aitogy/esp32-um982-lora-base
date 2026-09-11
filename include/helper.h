#ifndef HELPER_H
#define HELPER_H

#include "Top_Lvl_Config.h"
#include "Prog_Config.h"
#include "Connection_Type.h"
#include <Arduino.h>

#include "hardware/Wifi_handler.h"
#include "hardware/Sim_handler.h"
extern TinyGsm modem;

#include "functions/MQTT_Manager.h"

#if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
#include "functions/NTRIP_Handler_IP.h"
#else
#include "hardware/Lora_handler.h"
#include "functions/RTCM_Receiver.h"
#endif

// ================= ĐỊNH NGHĨA CÁC BIẾN TOÀN CỤC =================
extern String latestGGA;
extern bool mqttHealthMode;

// ================= ĐỊNH NGHĨA CÁC HÀM =================
String formDeviceHealthString(int32_t signalQualityDbm);

inline void shutdownTcpTransportBeforeRestart() {
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

#endif

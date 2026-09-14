#ifndef HELPER_H
#define HELPER_H

#include "Top_Lvl_Config.h"
#include "Prog_Config.h"
#include "hardware/Connection_type.h"
#include <Arduino.h>

#include "hardware/Wifi_handler.h"
#include "hardware/Sim_handler.h"
#include "functions/MQTT_Manager.h"

#if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
#include "functions/NTRIP_Handler_IP.h"
#else
#include "hardware/Lora_handler.h"
#include "functions/RTCM_Receiver.h"
#endif

// ================= ĐỊNH NGHĨA CÁC BIẾN TOÀN CỤC =================
extern TinyGsm modem;

// ================= ĐỊNH NGHĨA CÁC HÀM =================
String formDeviceHealthString(int32_t signalQualityDbm, bool gnssDataOk);
void processPendingSerialCommands();

void shutdownTcpTransportBeforeRestart();

#endif

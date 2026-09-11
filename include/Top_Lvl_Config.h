/*
    Cấu hình cho thiết bị ĐỂ SỬ DỤNG TẠI MAIN.CPP
*/
#ifndef TOP_LVL_CONFIG_H
#define TOP_LVL_CONFIG_H

// ================= CẤU HÌNH KHỞI TẠO =================
// #define PROGRAM_DEBUG 1

#ifndef WIFI_LORA_32_V4
#define WIFI_LORA_32_V4
#endif

#ifndef USE_KCT8103L_PA
#define USE_KCT8103L_PA
#endif

#ifndef LORAWAN_DEBUG_LEVEL
#define LORAWAN_DEBUG_LEVEL 1
#endif

#define TCP_IP 0
#define LORA_SERIAL 1

#ifndef RTCM_COMMUNICATION_PROTOCOL
#define RTCM_COMMUNICATION_PROTOCOL TCP_IP // Chọn giữa TCP_IP hoặc LORA_SERIAL
#endif

#define GNSS_MODULE_TYPE_UBLOX 0
#define GNSS_MODULE_TYPE_UNICORE 1

#ifndef GNSS_MODULE_TYPE
#define GNSS_MODULE_TYPE GNSS_MODULE_TYPE_UBLOX // Chọn giữa GNSS_MODULE_TYPE_UBLOX hoặc GNSS_MODULE_TYPE_UNICORE
#endif

#endif

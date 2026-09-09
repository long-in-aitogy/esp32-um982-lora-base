#include "helper.h"
#include "functions/RTCM_Receiver.h"
#include "functions/cmd_handler.h"

// ================= ĐỊNH NGHĨA CÁC BIẾN TOÀN CỤC =================
Preferences prefs;

extern PubSubClient mqtt;
#if CONNECT_USING_4G && RTCM_COMMUNICATION_PROTOCOL==TCP_IP
extern TinyGsmClient ntripClient;
extern TinyGsm modem;
#endif

#if CONNECT_USING_WIFI && RTCM_COMMUNICATION_PROTOCOL==TCP_IP
extern WiFiClient ntripClient;
#endif

#if RTCM_COMMUNICATION_PROTOCOL==LORA_SERIAL
String rtcmBuffer = ""; // Bộ đệm đọc RTCM từ UM980 để gửi lên Caster qua NTRIP
#endif

String latestRtcm = "";

namespace {
    inline constexpr uint8_t CONNECTION_FAIL_LIMIT = 5;

    class deviceHealth {
    public:
        static unsigned long lastHealthCheck;
        static uint8_t mqttDisconnectCount;
        static uint8_t ntripDisconnectCount;
        static uint8_t gsmDisconnectCount;
    };
}

// Semaphore
SemaphoreHandle_t rtcmBufferMutex = nullptr;
SemaphoreHandle_t tcpStreamMutex = nullptr;

/* ===================== NGUYÊN MẪU HÀM ======================== */

void initPrefs();
static void serviceMqtt(const bool reconnect);
#if CONNECT_USING_4G && RTCM_COMMUNICATION_PROTOCOL == TCP_IP
static void settleModemBeforeNtrip();
#endif

#if RTCM_COMMUNICATION_PROTOCOL == LORA_SERIAL
__attribute__((noreturn)) void taskLora([[maybe_unused]] void* const parameter);
__attribute__((noreturn)) void taskRtcm([[maybe_unused]] void* const parameter);
#else
__attribute__((noreturn)) void taskNtrip([[maybe_unused]] void* const parameter);
#endif
__attribute__((noreturn)) void healthCheckTask([[maybe_unused]] void* const parameter);
__attribute__((noreturn)) void taskMQTT([[maybe_unused]] void* const parameter);

/* ==================SETUP VÀ LOOP======================== */

void setup()
{
    Serial.begin(115200);
    #if BOARD_HELTEC
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
    #endif
    delay(100);

    // Debug marker: confirm Serial is working immediately after begin()
    Serial.println("[DEBUG] Serial initialized");
    Serial.println("\n=========================================");
    Serial.println("     ESP32 GNSS GATEWAY KHOI DONG        ");
    Serial.println("=========================================");

    delay(1000);

    // Khởi tạo Preferences
    prefs.begin("myPrefs"); // false: read/write mode
    bool needReset = prefs.getBool("NEED_RESET", true);
    int restartCount = prefs.getInt("RSTRT_COUNT", 0);
    prefs.putInt("RSTRT_COUNT", restartCount + 1);
    if (needReset) {
        Serial.println("[SETUP] Khoi tao Preferences lan dau tien...");
        initPrefs();
    }
    else {
        Serial.println("[SETUP] Preferences da duoc khoi tao truoc do, khong can khoi tao lai.");
    }

    int gnssTX = prefs.getInt("GNSS_TX", TX_GNSS);
    int gnssRX = prefs.getInt("GNSS_RX", RX_GNSS);
    #if CONNECT_USING_4G
    int rx2ModemTX = prefs.getInt("RX_TO_MODEM_TX", RX_TO_MODEM_TX);
    int tx2ModemRX = prefs.getInt("TX_TO_MODEM_RX", TX_TO_MODEM_RX);
    #endif
    prefs.end();

    // Khởi tạo giao tiếp với UM980
    #if PROGRAM_DEBUG
    Serial.println("[SETUP] Khoi dong giao tiep voi UM980: TX=" + String(gnssTX) + ", RX=" + String(gnssRX));
    #endif
    
    Serial1.begin(GNSS_BAUD, SERIAL_8N1, (uint8_t)gnssRX, (uint8_t)gnssTX);
    Serial1.setTimeout(100);

    if (needReset) {
        Serial.println("[SETUP] Cau hinh UM980 lan dau tien...");
        bootstrapUM980();
        Serial.println("[SETUP] Da cau hinh lai tat ca ve mac dinh, khoi dong lai de ap dung cau hinh.");
        ESP.restart(); // Khởi động lại để áp dụng cấu hình mới
    } else {
        Serial.println("[SETUP] Da cau hinh GNSS chip, khong can cau hinh lai.");
    }

    // Khởi động mạng
    bool networkConnected = false;

    #ifndef NATIVE_BUILD
    #if CONNECT_USING_4G
    SerialAT.begin(115200, SERIAL_8N1, (uint8_t)rx2ModemTX, (uint8_t)tx2ModemRX);
    delay(500);
    #endif
    #endif

    #if RTCM_COMMUNICATION_PROTOCOL == LORA_SERIAL
    int loraSetupResult = loraSetup();
    if (loraSetupResult != 0) {
        Serial.println("[SETUP][ERROR] Khoi dong LoRa that bai! Vui long kiem tra cau hinh va thu lai.");
    } else {
        Serial.println("[SETUP] Khoi dong LoRa thanh cong!");
    }
    #endif

    while (!networkConnected) {
#if CONNECT_USING_WIFI
        Serial.println("[SETUP] Su dung ket noi WIFI");
        networkConnected = setupWiFi();
#endif
#if CONNECT_USING_4G
        Serial.println("[SETUP] Su dung ket noi SIM/GSM");
        deviceHealth::gsmDisconnectCount = 0;
        if (startSIM()) {
            if (connectGSM()) {
                networkConnected = true;
            }
        }
#endif
        if (networkConnected) {
            Serial.println("[SETUP] Ket noi mang thanh cong!");
            #if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
            setupNTRIP();
            deviceHealth::ntripDisconnectCount = 0;
            #if CONNECT_USING_4G
            settleModemBeforeNtrip();
            #endif
            connectNTRIP();
            #endif
            deviceHealth::mqttDisconnectCount = 0;
            setupMQTT();
        } else {
            Serial.println("[ERROR] Khong the ket noi mang. Vui long kiem tra cau hinh va thu lai.");
        }
    }

    Serial.println("[SETUP] Khoi dong cac task...");

    Serial.println("[Setup] Tao mutex de dong bo hoa tai nguyen chung");
    
    rtcmBufferMutex = xSemaphoreCreateMutex();
    while (rtcmBufferMutex == nullptr) {
        Serial.println("[ERROR] Tao mutex rtcmDataMutex that bai! Dang thu lai...");
        rtcmBufferMutex = xSemaphoreCreateMutex();
    }
    Serial.println("[SETUP] Tao mutex rtcmDataMutex thanh cong!");

    #if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
    tcpStreamMutex = xSemaphoreCreateMutex();
    while (tcpStreamMutex == nullptr) {
        Serial.println("[ERROR] Tao mutex tcpStreamMutex that bai! Dang thu lai...");
        tcpStreamMutex = xSemaphoreCreateMutex();
    }
    Serial.println("[SETUP] Tao mutex tcpStreamMutex thanh cong!");
    #endif

    Serial.println("[SETUP] Task MQTT: Quan ly ket noi MQTT va callback.");
    xTaskCreatePinnedToCore(taskMQTT, "MQTT Task", 4096, nullptr, 3, nullptr, 1);
    Serial.println("[SETUP] Da khoi dong Task MQTT!");

    #if RTCM_COMMUNICATION_PROTOCOL == LORA_SERIAL
    Serial.println("[SETUP] Task LoRa: Truyen du lieu RTCM qua LoRa.");
    xTaskCreatePinnedToCore(taskLora, "LoRa Task", 4096, nullptr, 1, nullptr, 0);
    Serial.println("[SETUP] Da khoi dong Task LoRa!");

    Serial.println("[SETUP] Task RTCM: Doc du lieu RTCM tu UM980.");
    xTaskCreatePinnedToCore(taskRtcm, "RTCM Task", 4096, nullptr, 2, nullptr, 1);
    Serial.println("[SETUP] Da khoi dong Task RTCM!");
    #elif RTCM_COMMUNICATION_PROTOCOL == TCP_IP
    Serial.println("[SETUP] Task NTRIP: Gui du lieu RTCM qua NTRIP.");
    xTaskCreatePinnedToCore(taskNtrip, "NTRIP Task", 4096, nullptr, 2, nullptr, 1);
    Serial.println("[SETUP] Da khoi dong Task NTRIP!");
    #endif


    Serial.println("[SETUP] Task Health: Gui thong tin suc khoe thiet bi len MQTT moi 30s");
    xTaskCreatePinnedToCore(healthCheckTask, "Health Task", 4096, nullptr, 1, nullptr, 1);
    Serial.println("[SETUP] Da khoi dong Task Health!");

    Serial.println("=========================================");
    Serial.println("        KHOI DONG HOAN TAT               ");
    Serial.println("=========================================\n");

    digitalWrite(LED_PIN, HIGH);

    delay(1000);

    digitalWrite(LED_PIN, LOW);
}

#if RTCM_COMMUNICATION_PROTOCOL == LORA_SERIAL
/* ================= TRIỂN KHAI HÀM TASK ====================== */
__attribute__((noreturn)) void taskRtcm([[maybe_unused]] void* const parameter) {
    // Sử dụng chung rtcmBuffer với taskLora, cần mutex
    while (true) {
        if (xSemaphoreTake(rtcmBufferMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)))
        {
            if (!rtcmBuffer.isEmpty()) {
                #if PROGRAM_DEBUG
                Serial.println("[RTCM TASK] LoRa chua kip gui xong du lieu RTCM truoc do, dang doi de gui tiep...");
                #endif
                goto giveUpMutexRtcm;
            }

            rtcmBuffer = receiveRtcmFromGnss();
            if (!rtcmBuffer.isEmpty()) {
                Serial.println("[RTCM TASK] Da nhan du lieu RTCM tu mach RTK. So byte: " + String(rtcmBuffer.length()));
            } else {
                Serial.println("[RTCM TASK] Du lieu RTCM rong.");

            }
            Serial.println();

            giveUpMutexRtcm:
            xSemaphoreGive(rtcmBufferMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
#else
__attribute__((noreturn)) void taskNtrip([[maybe_unused]] void* const parameter) {
    Serial.println("[NTRIP TASK] Bat dau task NTRIP...");
    int loopStatus = 0;
    String rtcmRead = "";
    while (true) {
        rtcmRead = receiveRtcmFromGnss();
        if (xSemaphoreTake(rtcmBufferMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)))
        {
            latestRtcm = rtcmRead;
            xSemaphoreGive(rtcmBufferMutex);
        }

        #if CONNECT_USING_4G
        bool gprsConnected = false;
        if (xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
            gprsConnected = modem.isGprsConnected();
            xSemaphoreGive(tcpStreamMutex);
        }
        if (!gprsConnected) {
            if (++deviceHealth::gsmDisconnectCount >= CONNECTION_FAIL_LIMIT) {
                Serial.println("[GSM TASK][ERROR] GPRS mat ket noi qua 5 lan, khoi dong lai ESP32...");
                shutdownTcpTransportBeforeRestart();
                ESP.restart();
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        deviceHealth::gsmDisconnectCount = 0;
        #endif
        if (xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
            if (loopStatus == 504) {
                Serial.println("[NTRIP TASK] Dang thu ket noi lai NTRIP...");
                connectNTRIP();
            }
            loopStatus = loopNTRIP(rtcmRead);
            xSemaphoreGive(tcpStreamMutex);
            if (loopStatus == 500 || loopStatus == 504 || !isNtripConnected()) {
                if (++deviceHealth::ntripDisconnectCount >= CONNECTION_FAIL_LIMIT) {
                    Serial.println("[NTRIP TASK][ERROR] NTRIP mat ket noi qua 5 lan, khoi dong lai ESP32...");
                    shutdownTcpTransportBeforeRestart();
                    ESP.restart();
                }
            } else {
                deviceHealth::ntripDisconnectCount = 0;
            }
            #if PROGRAM_DEBUG
            Serial.println("[NTRIP TASK] loopNTRIP() tra ve: " + String(loopStatus));
            #endif
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
#endif

#if RTCM_COMMUNICATION_PROTOCOL == LORA_SERIAL
__attribute__((noreturn))void taskLora([[maybe_unused]] void* const parameter) {
    // Sử dụng chung rtcmBuffer với taskRtcm, cần mutex
    String tempRtcm = "";
    bool lastStateWasEmpty = true;
    while (true) {
        if (xSemaphoreTake(rtcmBufferMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
            #if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
            loopNTRIP(latestGGA);
            #else
            if (rtcmBuffer.isEmpty()) {
                #if PROGRAM_DEBUG
                Serial.println("[LORA TASK] Chua co du lieu RTCM de truyen qua LoRa.");
                #endif
                lastStateWasEmpty = true;
                goto giveUpMutexLora;
            }

            if (lastStateWasEmpty) {
                tempRtcm = rtcmBuffer;
                lastStateWasEmpty = false;
            }

            #if PROGRAM_DEBUG
            Serial.println("[LORA TASK] Chuan bi truyen du lieu RTCM qua LoRA...");

            Serial.println("[LORA TASK] Noi dung con lai trong buffer duoc in ra theo hexa:");

            for (int i = 0; i < rtcmBuffer.length(); i++) {
                Serial.printf("%02X ", static_cast<uint8_t>(rtcmBuffer[i]));

                if ((i + 1) % 16 == 0) {
                    Serial.println();
                }
            }

            Serial.println();
            #endif // PROGRAM_DEBUG

            lora_packet_process(rtcmBuffer);

            if (rtcmBuffer.isEmpty() && !lastStateWasEmpty) {
                latestRtcm = tempRtcm;
                lastStateWasEmpty = true;
            }

            #if PROGRAM_DEBUG
            Serial.println("[LORA TASK] Da xoa du lieu RTCM trong buffer sau khi gui.");
            #endif // PROGRAM_DEBUG
            #endif

            giveUpMutexLora:
            xSemaphoreGive(rtcmBufferMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
#endif

__attribute__((noreturn)) void healthCheckTask([[maybe_unused]] void* const parameter) {
    // Có tranh chấp tài nguyên với task RTCM và NTRIP publish
    String healthPayload = "";
    uint32_t loopStartTime = 0;
    uint32_t remainingWait = 0;
    while (true) {
        loopStartTime = millis();
        int32_t signalQualityDbm = -1;
        #if RTCM_COMMUNICATION_PROTOCOL == TCP_IP
        if (xSemaphoreTake(rtcmBufferMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)))
        {
            #if CONNECT_USING_4G
            if (xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
                signalQualityDbm = modem.getSignalQuality();
                xSemaphoreGive(tcpStreamMutex);
            }
            #endif
            healthPayload = formDeviceHealthString(signalQualityDbm);
            xSemaphoreGive(rtcmBufferMutex);
        }
        #else
            healthPayload = formDeviceHealthString(signalQualityDbm);
        #endif
        vTaskDelay(1);
        Serial.print("[HEALTH CHECK] ");
        Serial.println(healthPayload);

        if (healthPayload.isEmpty())
        {
            if (HEALTH_INTERVAL > (millis() - loopStartTime)) {
                remainingWait = HEALTH_INTERVAL - (millis() - loopStartTime);
            } else {
                remainingWait = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(remainingWait));
            continue;
        }

        #if PROGRAM_DEBUG
        Serial.println("[HEALTH CHECK] Kiem tra ket noi MQTT de gui thong tin suc khoe...");
        #endif

        // Publish health and any latest RTCM via thread-safe helpers.
        // The helpers will wait for MQTT connection and take the tcpStreamMutex.
        publishHealth(healthPayload);
        if (!latestRtcm.isEmpty()) {
            #if PROGRAM_DEBUG
            Serial.println("[GNSS PUBLISH] Dang gui du lieu NMEA len MQTT...");
            #endif
            publishRaw(latestRtcm); // publishRaw accepts String&
        }
        vTaskDelay(1);
        if (HEALTH_INTERVAL > (millis() - loopStartTime)) {
            remainingWait = HEALTH_INTERVAL - (millis() - loopStartTime);
        } else {
            remainingWait = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(remainingWait));
    }
}

__attribute__((noreturn)) void taskMQTT([[maybe_unused]] void* const parameter) {
    Serial.println("[MQTT TASK] Bat dau task MQTT...");
    while (true) {
        // Prefer to take tcpStreamMutex when available to serialize network operations
        if (tcpStreamMutex != nullptr) {
            if (xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
                serviceMqtt(true);
                xSemaphoreGive(tcpStreamMutex);
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // no tcpStreamMutex available, just keep the loop running
        serviceMqtt(false);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void loop() {
    #if CONNECT_USING_4G
    if (xSemaphoreTake(tcpStreamMutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS))) {
        if (!modem.isGprsConnected()) {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("[LOOP] GPRS mat ket noi, dang thu ket noi lai...");
            ntripClient.stop(0);
            if (++deviceHealth::gsmDisconnectCount >= CONNECTION_FAIL_LIMIT) {
                Serial.println("[LOOP][ERROR] GPRS mat ket noi qua 5 lan, khoi dong lai ESP32...");
                shutdownTcpTransportBeforeRestart();
                ESP.restart();
            }
            connectGSM();
        } else {
            deviceHealth::gsmDisconnectCount = 0;
            digitalWrite(LED_PIN, LOW);
        }
        xSemaphoreGive(tcpStreamMutex);
    }
    #endif
    #if CONNECT_USING_WIFI
    if (WiFiClass::status() != WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("[LOOP] WiFi mat ket noi, dang thu ket noi lai...");
        setupWiFi();
        digitalWrite(LED_PIN, LOW);
    }
    #endif
    // MQTT loop handled in dedicated task `taskMQTT`
    vTaskDelay(pdMS_TO_TICKS(1000)); // loop trống, tất cả logic đã được xử lý trong các task
}

#if CONNECT_USING_4G && RTCM_COMMUNICATION_PROTOCOL == TCP_IP
static void settleModemBeforeNtrip() {
    Serial.println("[SETUP][NTRIP] Cho modem on dinh truoc khi bat tay NTRIP...");
    const uint32_t settleStart = millis();
    const uint32_t settleDurationMs = 600;

    while (millis() - settleStart < settleDurationMs) {
        modem.maintain();
        delay(20);
    }

    Serial.println("[SETUP][NTRIP] Modem da on dinh, bat dau ket noi NTRIP.");
}
#endif

void initPrefs() {
    prefs.clear();
    prefs.putBool("NEED_RESET", false);
    prefs.putInt("RSTRT_COUNT", 0); // chưa cấu hình được, lấy được
    prefs.putUChar("TX_TO_MODEM_RX", 17); // chưa cấu hình được, lấy được
    prefs.putUChar("RX_TO_MODEM_TX", 16); // chưa cấu hình được, lấy được
    prefs.putUChar("MODEM_DC_PIN", 15); // chưa cấu hình đc, chưa lấy đc
    prefs.putUChar("MODEM_DTR_PIN", 4); // chưa cấu hình đc, chưa lấy đc
    prefs.putString("APN", "v-internet"); // cấu hình đc, chưa lấy đc
    prefs.putString("GPRS_USER", ""); // cấu hình đc, chưa lấy đc
    prefs.putString("GPRS_PASS", ""); // cấu hình đc, chưa lấy đc
    prefs.putInt("GNSS_RX", RX_GNSS); // cấu hình được, lấy được
    prefs.putInt("GNSS_TX", TX_GNSS); // cấu hình được, lấy được
    prefs.putString("NTRIP_SERVER", NTRIP_CASTER_IP); // cấu hình được, lấy được
    prefs.putUShort("NTRIP_PORT", 2101); // cấu hình được, lấy được
    prefs.putString("NTRIP_MPT", "/test"); // cấu hình được, lấy được
    prefs.putString("NT_AUTH_BS", "12345"); // cấu hình được, lấy được
    prefs.putString("MQTT_SERVER", "aitogy.asia"); // cấu hình được, lấy được
    prefs.putUShort("MQTT_PORT", 1883); // cấu hình được, lấy được
    prefs.putString("MQTT_USER", "mqttUser"); // cấu hình đc, lấy được
    prefs.putString("MQTT_PASS", "MqttPassword123$%^"); // cấu hình đc, lấy được
    prefs.putString("TPC_SUB_CMD", "tdm2402/um980_base_001/cmd"); // cấu hình được, lấy được
    prefs.putString("TPC_RAW_RTCM", "tdm2402/um980_base_001/raw/last_rtcm"); // cấu hình đc, lấy được
    prefs.putString("TPC_HEALTH", "tdm2402/um980_base_001/health"); // cấu hình đc, lấy được
}

static void serviceMqtt(const bool reconnect) {
    if (mqtt.connected()) {
        deviceHealth::mqttDisconnectCount = 0;
        mqtt.loop();
        return;
    }

    if (reconnect) {
        Serial.println("[MQTT TASK] MQTT mat ket noi, dang ket noi lai...");
    }
    
    if (
        const bool restartRequired 
            = ++deviceHealth::mqttDisconnectCount >= CONNECTION_FAIL_LIMIT;
        restartRequired
    ) {
        Serial.println("[MQTT TASK][ERROR] MQTT mat ket noi qua 5 lan, khoi dong lai ESP32...");
        shutdownTcpTransportBeforeRestart();
        ESP.restart();
    }
    if (reconnect) {
        connectMQTT();
    }
}
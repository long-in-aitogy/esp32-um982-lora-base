#if RTCM_COMMUNICATION_PROTOCOL == TCP_IP || defined(UNIT_TEST)
#define NTRIP_HANDLER_IP_CODE

#include "functions/NTRIP_Handler_IP.h"
#include "Prog_Config.h"
#include <Preferences.h>

// ================= BIẾN TOÀN CỤC =================
extern Preferences prefs;
static bool isIcyOk = false;
static unsigned long lastReconnect = 0;
static bool isNmeaSent = false; // Cờ kiểm tra xem đã gửi NMEA xác thực chưa

// ================= CÁC ĐỐI TƯỢNG KẾT NỐI =================
#if CONNECT_USING_WIFI
#include "hardware/Wifi_handler.h"
WiFiClient ntripClient;
#endif
#if CONNECT_USING_4G
#include "hardware/Sim_handler.h"
extern TinyGsm modem;
TinyGsmClient ntripClient(modem, 0);
#endif

extern String latestRtcm;
extern SemaphoreHandle_t rtcmBufferMutex;
extern SemaphoreHandle_t tcpStreamMutex;

// ================= ĐỊNH NGHĨA HÀM =================

int setupNTRIP() {
  isIcyOk = false;
  isNmeaSent = false;

  char nmeaCmdUnlog[] = "UNLOG\r\n";
  // char nmeaCmdSetBase[] = "MODE BASE -1618563.4772 5730003.6935 2278811.0631\r\n";
  char nmeaCmdSetBase[] = "MODE BASE TIME 120 2.5\r\n";
  char rtcm1084SetOutputCom2[] = "RTCM1084 COM1 1\r\n";
  char rtcm1074SetOutputCom2[] = "RTCM1074 COM1 1\r\n";
  char rtcm1006SetOutputCom2[] = "RTCM1006 COM1 1\r\n";
  char rtcm1124SetOutputCom2[] = "RTCM1124 COM1 1\r\n";
  char rtcm1087SetOutputCom2[] = "RTCM1087 COM1 1\r\n";
  char rtcm1077SetOutputCom2[] = "RTCM1077 COM1 1\r\n";
  char rtcm1127SetOutputCom2[] = "RTCM1127 COM1 1\r\n";
  char rtcmSetBaudCom2[] = "CONFIG COM1 115200\r\n";

  char cmdSaveConfig[] = "saveconfig\r\n";

  // tắt tính năng log trước khi bật lại
  unsigned long WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(nmeaCmdUnlog, strlen(nmeaCmdUnlog)); // Gửi lệnh UNLOG

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  String nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // cài baud rate cho COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcmSetBaudCom2, strlen(rtcmSetBaudCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  // thiết lập chế độ base
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(nmeaCmdSetBase, strlen(nmeaCmdSetBase)); // Gửi lệnh NMEA để thiết lập chế độ base station

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1084 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1084SetOutputCom2, strlen(rtcm1084SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1074 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1074SetOutputCom2, strlen(rtcm1074SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1006 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1006SetOutputCom2, strlen(rtcm1006SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1124 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1124SetOutputCom2, strlen(rtcm1124SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1087 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1087SetOutputCom2, strlen(rtcm1087SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1077 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1077SetOutputCom2, strlen(rtcm1077SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // gửi rtcm 1127 ra COM2
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(rtcm1127SetOutputCom2, strlen(rtcm1127SetOutputCom2)); // Gửi lệnh RTCM để thiết lập đầu ra COM2

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  // lưu cấu hình
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  Serial1.write(cmdSaveConfig, strlen(cmdSaveConfig)); // Gửi lệnh lưu cấu hình

  #ifdef PROGRAM_DEBUG
  WaitStartTime = millis();
  while (!Serial1.available() && millis() - WaitStartTime < 2000) {
    delay(10);
  }
  nmeaResponse = Serial1.readStringUntil('\n'); // Đọc phản hồi từ GNSS
  Serial.println("[NMEA CMD] Response: " + nmeaResponse);
  #endif
  delay(20);

  return 0;
}

bool isNtripConnected() {
  return isIcyOk; // Trả về true nếu đã xác thực thành công với Caster
}

int connectNTRIP() {
  Serial.print("\n[NTRIP] Dang mo TCP den: ");
  Serial.println(NTRIP_CASTER_IP);

  ntripClient.stop();
  
  prefs.begin("myPrefs", true);
  String ntripMountpoint = prefs.getString("NTRIP_MOUNTPOINT", NTRIP_MOUNTPOINT);
  prefs.end();

  if (ntripClient.connect(NTRIP_CASTER_IP, NTRIP_CASTER_PORT)) {
    delay(1000); // Đợi một chút để đảm bảo kết nối ổn định
    Serial.println("[NTRIP] Da ket noi TCP! Dang gui Header...");
    
    sendRequest:
    String request = "SOURCE " + String(NTRIP_AUTH_BASE_STATION) + " " + String(ntripMountpoint) + " \r\n"
          + "Source-Agent: NTRIP NtripServerCMD/1.0\r\n\r\n";
    ntripClient.print(request);

    #if PROGRAM_DEBUG
    Serial.println("[NTRIP] Da gui request header len Caster:\n" + request);
    #endif
    
    // Đợi server trả lời ICY OK
    unsigned long timeout = millis();
    while (ntripClient.connected() && millis() - timeout < 10000L) {
      if (ntripClient.available()) {
        String response = ntripClient.readStringUntil('\n');
        response.trim();
        #if PROGRAM_DEBUG
        Serial.print("[CASTER RESP]: ");
        Serial.println(response);
        #endif
        
        if (response.indexOf("ICY 200 OK") != -1 || response.indexOf("ICY OK") != -1) {
          isIcyOk = true;
          isNmeaSent = false;
          Serial.println("[NTRIP] Xac thuc THANH CONG (ICY OK)!");
          break;
        }
      }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (!isIcyOk) {
      Serial.println("[NTRIP] Khong nhan duoc ICY OK tu Caster!");
    }
    // goto sendRequest; // Thử gửi lại request nếu không nhận được phản hồi
  } else {
    Serial.println("[NTRIP] Loi ket noi TCP socket!");
    return -1;
  }
  return 0;
}

int loopNTRIP(String& rtcmData) {
  // không sử dụng tài nguyên chung, không cần mutex
  int returnCode = NTRIP_MODE; // returnCode = NTRIP_MODE + ntripClient.available() * 4
  // 1. Quản lý mất kết nối
  if (!ntripClient.connected()) {
    ntripClient.stop();
    isIcyOk = false;
    if (millis() - lastReconnect > 5000) { // Thử lại sau 7 giây
      lastReconnect = millis();
      return 504; // chuẩn bị kết nối lại
    }
    return 500; // Chưa kết nối, sẽ quay lại ở vòng tiếp theo của loop()
  }

  // 2. Xử lý sau khi kết nối thành công / cảnh báo nếu không kết nối thành công
  #if PROGRAM_DEBUG
  if (!isIcyOk) {
    Serial.println("[NTRIP][WARN] Chua xac thuc voi Caster, du lieu van se duoc gui nhung khong dam bao se toi duoc caster...");
  }
  #endif

  // 3. Đẩy RTCM lên Caster nếu có dữ liệu
  if (!rtcmData.isEmpty()) {
    ntripClient.print(rtcmData); // Gửi dữ liệu RTCM lên Caster
    #if PROGRAM_DEBUG
    Serial.println("[NTRIP TASK] Da gui du lieu RTCM len Caster!");
    #endif
    returnCode += 4;
  }
  #if PROGRAM_DEBUG
  else {
    Serial.println("[NTRIP TASK] Khong co du lieu RTCM de gui len Caster.");
  }
  #endif
  return returnCode;
}
#endif // NTRIP_HANDLER_IP_CODE

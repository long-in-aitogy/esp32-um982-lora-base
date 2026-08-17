#include "functions/RTCM_Receiver.h"
#include "HardwareSerial.h"

String receiveRtcmFromGnss() {
    #ifdef PROGRAM_TEST
    String rtcmData = "THIS IS NOT A REAL RTCM DATA. THIS IS A TEST STRING FOR UNIT TESTING PURPOSES.";
    #else
    String rtcmData = "";
    while (Serial1.available() > 0) {
        int nextByte = Serial1.read();
        if (nextByte < 0) {
            break;
        }
        rtcmData += static_cast<char>(nextByte);
    }
    #endif
    if (!rtcmData.isEmpty()) {
        Serial.println("[UM980] Da nhan du lieu RTCM tu mach RTK. So byte: " + String(rtcmData.length()));
        #if PROGRAM_DEBUG
        Serial.println("[UM980] Du lieu nhan duoc: ");
        Serial.println(rtcmData);
        #endif
    } else {
        Serial.println("[UM980] Khong co du lieu RTCM hop le.");
    }
    return rtcmData;
}

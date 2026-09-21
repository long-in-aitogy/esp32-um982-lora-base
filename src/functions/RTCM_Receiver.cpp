#include "functions/RTCM_Receiver.h"
#include "HardwareSerial.h"

String receiveRtcmFromGnss() {
    #ifdef PROGRAM_TEST
    String rtcmData = "THIS IS NOT A REAL RTCM DATA. THIS IS A TEST STRING FOR UNIT TESTING PURPOSES.";
    #else
    String rtcmData;
    uint8_t buf[128];

    while (Serial1.available() > 0) {
        const size_t bytesAvailable = static_cast<size_t>(Serial1.available());
        const size_t bytesToRead = min(bytesAvailable, sizeof(buf));

        if (bytesToRead == 0) {
            break;
        }

        const size_t bytesRead = Serial1.readBytes(reinterpret_cast<char*>(buf), bytesToRead);
        if (bytesRead == 0) {
            break;
        }

        rtcmData.concat(reinterpret_cast<char*>(buf), bytesRead);
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

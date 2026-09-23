#ifndef RTCM_RECEIVER_H
#define RTCM_RECEIVER_H

#include "Top_Lvl_Config.h"
#include "Prog_Config.h"
#include "WString.h"

inline constexpr size_t RTCM_SUPPORTED_MESSAGE_TYPE_COUNT = 10;

struct RtcmMessageCounts {
	uint32_t values[RTCM_SUPPORTED_MESSAGE_TYPE_COUNT] = {};
};

String receiveRtcmFromGnss();
RtcmMessageCounts consumeRtcmMessageCounts();

#endif
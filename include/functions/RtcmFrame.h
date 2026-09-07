#ifndef RTCM_FRAME_H
#define RTCM_FRAME_H

#include <cstddef>
#include <cstdint>

// Must accommodate the largest RTCM batch read from Serial1 between task runs.
inline constexpr size_t RTCM_FRAME_MAX_SIZE = 2048;

struct RtcmFrame {
    uint8_t data[RTCM_FRAME_MAX_SIZE] = {};
    size_t length = 0;

    bool isEmpty() const { return length == 0; }
};

#endif

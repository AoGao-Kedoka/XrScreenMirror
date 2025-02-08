#pragma once

#include <cstdint>
#include <vector>

class ICapture {
   public:
    virtual ~ICapture() = default;
    virtual std::vector<uint8_t> CaptureScreen() = 0;
    int Width{0};
    int Height{0};
};

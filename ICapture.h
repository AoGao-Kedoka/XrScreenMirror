#pragma once

#include <vector>
#include <cstdint>

class ICapture{
    public:
        virtual ~ICapture() = default;
        virtual std::vector<uint8_t> CaptureScreen() = 0;
        int Width{0};
        int Height{0};
};

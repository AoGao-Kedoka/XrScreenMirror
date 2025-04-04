#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include "ICapture.h"
#include <cstdint>
#include <windows.h>

class WindowsCapture : public ICapture {
   public:
    WindowsCapture();
    std::vector<uint8_t> CaptureScreen() override;
    ~WindowsCapture() = default;

   private:
};
#endif

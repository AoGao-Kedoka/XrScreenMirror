#pragma once

#if defined(__linux__)
#include "ICapture.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "stdio.h"
#include <X11/extensions/Xrandr.h>

class LinuxCapture : public ICapture {
   public:
    LinuxCapture();
    std::vector<uint8_t> CaptureScreen() override;
    ~LinuxCapture();

   private:
    bool GetPrimaryDisplayDimensions();

   private:
    XImage* prevImage{nullptr};
    Display* m_display{nullptr};
    std::vector<uint8_t> data;
    int x = 0, y = 0;
};
#endif

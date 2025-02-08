#include "LinuxCapture.h"

#if defined(__linux__)
LinuxCapture::LinuxCapture() {
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
    }
    m_display = display;
    if (!GetPrimaryDisplayDimensions()) {
        XCloseDisplay(display);
        return;
    }
}

LinuxCapture::~LinuxCapture() {
    XCloseDisplay(m_display);
}

bool LinuxCapture::GetPrimaryDisplayDimensions() {
    Window root = DefaultRootWindow(m_display);
    XRRScreenResources* screenResources = XRRGetScreenResources(m_display, root);
    if (!screenResources) {
        fprintf(stderr, "Unable to get screen resources\n");
        return false;
    }

    RROutput primaryOutput = XRRGetOutputPrimary(m_display, root);
    if (!primaryOutput) {
        fprintf(stderr, "No primary output found\n");
        XRRFreeScreenResources(screenResources);
        return false;
    }

    XRROutputInfo* outputInfo = XRRGetOutputInfo(m_display, screenResources, primaryOutput);
    if (!outputInfo || outputInfo->connection == RR_Disconnected) {
        fprintf(stderr, "Unable to get primary output info or it is disconnected\n");
        XRRFreeOutputInfo(outputInfo);
        XRRFreeScreenResources(screenResources);
        return false;
    }

    bool success = false;
    if (outputInfo->ncrtc > 0) {
        XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(m_display, screenResources, outputInfo->crtc);
        if (crtcInfo) {
            x = crtcInfo->x;
            y = crtcInfo->y;
            Width = crtcInfo->width;
            Height = crtcInfo->height;
            success = true;
            XRRFreeCrtcInfo(crtcInfo);
        }
    }

    XRRFreeOutputInfo(outputInfo);
    XRRFreeScreenResources(screenResources);
    return success;
}
std::vector<uint8_t> LinuxCapture::CaptureScreen() {
    Window root = DefaultRootWindow(m_display);
    XImage* image = XGetImage(m_display, root, x, y, Width, Height, AllPlanes, ZPixmap);
    if (!image) {
        fprintf(stderr, "Failed to capture screen\n");
    }

    size_t imageSize = image->width * image->height * 4;
    data.clear();
    data.resize(imageSize);
    uint8_t* imageData = reinterpret_cast<uint8_t*>(image->data);

    // Correct BGRA to RGBA
    for (size_t i = 0; i < imageSize; i += 4) {
        data[i + 0] = imageData[i + 2];    // Red   <- Blue
        data[i + 1] = imageData[i + 1];    // Green <- Green
        data[i + 2] = imageData[i + 0];    // Blue  <- Red
        data[i + 3] = imageData[i + 3];    // Alpha <- Alpha
    }
    XDestroyImage(image);
    return data;
}
#endif

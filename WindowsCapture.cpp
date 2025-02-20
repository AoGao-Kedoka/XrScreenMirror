#include "WindowsCapture.h"

#if defined(_WIN32) || defined(_WIN64)
WindowsCapture::WindowsCapture() {
    SetProcessDPIAware();
    HDC screenDC = GetDC(NULL);
    Width = GetDeviceCaps(screenDC, HORZRES);
    Height = GetDeviceCaps(screenDC, VERTRES);
    ReleaseDC(NULL, screenDC);
}

std::vector<uint8_t> WindowsCapture::CaptureScreen() {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Create a bitmap compatible with the screen DC.
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, Width, Height);
    if (!hBitmap) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return {};
    }

    // Select the bitmap into the memory DC.
    HGDIOBJ oldObj = SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, Width, Height, hScreenDC, 0, 0, SRCCOPY);

    // Prepare bitmap info
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = Width;
    bmi.bmiHeader.biHeight = -Height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Allocate buffer for the pixel data.
    std::vector<uint8_t> data(Width * Height * 4);
    if (!GetDIBits(hMemoryDC, hBitmap, 0, Height, data.data(), &bmi, DIB_RGB_COLORS)) {
        data.clear();
    }

    SelectObject(hMemoryDC, oldObj);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    // Return pixel data (format is BGRA; convert to RGBA if needed)
    for (size_t i = 0; i < data.size(); i += 4) {
        uint8_t temp = data[i];    // Blue
        data[i] = data[i + 2];     // Red
        data[i + 2] = temp;        // Blue
    }

    return data;
}
#endif

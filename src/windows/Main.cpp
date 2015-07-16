#define NOMINMAX
#include <windows.h>
#include "stdafx.h"

#include "..\common\trace_math.h"
#include "..\common\Render.h"
#include "..\common\defaults.h"

HWND hWnd = NULL;
Render * render = NULL;
HFONT font = NULL;
HDC memDC = NULL;
HBITMAP bitmap = NULL;
DWORD * pixels = NULL;
unsigned int bmWidth = 0;
unsigned int bmHeight = 0;
bool imageReady = false;
float frameChunkTime = 0.0f;
float frameTime = 0.0f;
char exeFullPath[MAX_PATH];
int scrnshotWidth = 0;
int scrnshotHeight = 0;
int scrnshotSamples = 0;
unsigned int scrnshotChunkPixelCount = 1;
float scrnshotProgress = -1;
DWORD scrnshotStartTicks = 0;
std::string scrnshotFileName;
LARGE_INTEGER perfFreq = { 0, 0 };
bool initPerfSuccess = (QueryPerformanceFrequency(&perfFreq) && perfFreq.QuadPart > 0);
bool quitMessage = false;
int controlFlags = 0;
enum STATE {
  stCameraControl,
  stScreenshotResolutionSelection,
  stScreenshotSamplingSelection,
  stScreenshotRenderBegin,
  stScreenshotRenderProceed,
  stScreenshotRenderEnd,
  stScreenshotRenderSave,
  stScreenshotRenderCancelRequested
} state;

void print(const HDC hdc, const int x, const int y, const char* format, ...)
{
  static std::vector<char> buf(1024);
  va_list args;
  va_start(args, format);
  size_t str_size = 1 + vsnprintf(0, 0, format, args);
  if (str_size > buf.size())
    buf.resize(str_size);
  char * buffer = &buf.front();
  vsnprintf(buffer, str_size, format, args);
  va_end(args);

  SetTextColor(hdc, 0xAAAAAA);
  TextOutA(hdc, x - 1, y - 1, buffer, (int)strlen(buffer));
  TextOutA(hdc, x + 1, y - 1, buffer, (int)strlen(buffer));
  TextOutA(hdc, x - 1, y + 1, buffer, (int)strlen(buffer));
  TextOutA(hdc, x + 1, y + 1, buffer, (int)strlen(buffer));
  TextOutA(hdc, x, y - 1, buffer, (int)strlen(buffer));
  TextOutA(hdc, x + 1, y, buffer, (int)strlen(buffer));
  TextOutA(hdc, x, y + 1, buffer, (int)strlen(buffer));
  TextOutA(hdc, x + 1, y, buffer, (int)strlen(buffer));
  SetTextColor(hdc, 0);
  TextOutA(hdc, x, y, buffer, (int)strlen(buffer));
}

void DrawImage(const HWND hWnd, const HDC hdc)
{
  if (imageReady)
  {
    if (bmWidth != render->imageWidth || bmHeight != render->imageHeight)
    {
      if (memDC)
      {
        DeleteDC(memDC);
        memDC = NULL;
      }

      if (bitmap)
      {
        DeleteObject(bitmap);
        bitmap = NULL;
        pixels = NULL;
      }

      BITMAPINFO bmi;
      bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
      bmi.bmiHeader.biWidth = max(render->imageWidth, 1u);
      bmi.bmiHeader.biHeight = max(render->imageHeight, 1u);
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biCompression = BI_RGB;
      bmi.bmiHeader.biSizeImage = 0;
      bmi.bmiHeader.biXPelsPerMeter = 0;
      bmi.bmiHeader.biYPelsPerMeter = 0;
      bmi.bmiHeader.biClrUsed = 0;
      bmi.bmiHeader.biClrImportant = 0;

      memDC = CreateCompatibleDC(NULL);
      bitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);

      if (memDC && bitmap)
      {
        SelectObject(memDC, bitmap);
        bmWidth = render->imageWidth;
        bmHeight = render->imageHeight;
      }
      else
      {
        bmWidth = 0;
        bmHeight = 0;
      }
    }

    if (pixels)
    {
      for (unsigned int y = 0; y < bmHeight; ++y)
      for (unsigned int x = 0; x < bmWidth; ++x)
        pixels[x + y * bmWidth] = render->imagePixel(x, y).argb();
    }

    imageReady = false;
  }

  if (memDC && pixels)
  {
    RECT clientRect;
    if (GetClientRect(hWnd, &clientRect) && clientRect.right && clientRect.bottom)
    {
      float clientAspect = float(clientRect.right) / clientRect.bottom;
      const int yGap = int(bmWidth / clientAspect) - bmHeight;
      SetStretchBltMode(hdc, HALFTONE);
      StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, -yGap / 2, bmWidth, bmHeight + yGap, SRCCOPY);
    }
  }
}

void DrawTextTips(const HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, 0);
  const int lineHeight = tm.tmHeight + 1;
  const int x = 3;
  int y = 3;

  switch (state)
  {
  case stCameraControl:
    if (frameTime < 10)
      print(hdc, x, y, "Frame time: %.0f ms", frameTime * 1000);
    else
      print(hdc, x, y, "Frame time: %.03f s", frameTime);
    y += lineHeight;
    print(hdc, x, y, "Blended frames : %i", render->additiveCounter);
    y += lineHeight * 2;
    print(hdc, x, y, "WSAD : moving");
    y += lineHeight;
    print(hdc, x, y, "Cursor keys: turning");
    y += lineHeight;
    print(hdc, x, y, "Space : ascenting");
    y += lineHeight;
    print(hdc, x, y, "Ctrl : descenting");
    y += lineHeight * 2;
    print(hdc, x, y, "F2 : save screenshot");
    break;
  case stScreenshotResolutionSelection:
    y += lineHeight;
    print(hdc, x, y, "Select screenshot resolution (keys 1-9)");
    y += lineHeight * 2;
    print(hdc, x, y, "1 : 640x480 (4:3)");
    y += lineHeight;
    print(hdc, x, y, "2 : 800x600 (4:3)");
    y += lineHeight;
    print(hdc, x, y, "3 : 1024x768 (4:3)");
    y += lineHeight;
    print(hdc, x, y, "4 : 1280x960 (4:3)");
    y += lineHeight;
    print(hdc, x, y, "5 : 1280x800 (16:10)");
    y += lineHeight;
    print(hdc, x, y, "6 : 1680x1050 (16:10)");
    y += lineHeight;
    print(hdc, x, y, "7 : 1920x1200 (16:10)");
    y += lineHeight;
    print(hdc, x, y, "8 : 1280x720 (HD)");
    y += lineHeight;
    print(hdc, x, y, "9 : 1920x1080 (HD)");
    y += lineHeight * 2;
    print(hdc, x, y, "ESC : cancel");
    break;
  case stScreenshotSamplingSelection:
    y += lineHeight;
    print(hdc, x, y, "Select supersampling rate (keys 1-9)");
    y += lineHeight * 2;
    print(hdc, x, y, "1 : 1x (fast but rough)");
    y += lineHeight;
    print(hdc, x, y, "2 : 2x");
    y += lineHeight;
    print(hdc, x, y, "3 : 4x");
    y += lineHeight;
    print(hdc, x, y, "4 : 8x");
    y += lineHeight;
    print(hdc, x, y, "5 : 16x");
    y += lineHeight;
    print(hdc, x, y, "6 : 32x");
    y += lineHeight;
    print(hdc, x, y, "7 : 64x");
    y += lineHeight;
    print(hdc, x, y, "8 : 128x");
    y += lineHeight;
    print(hdc, x, y, "9 : 256x (slow but smooth)");
    y += lineHeight * 2;
    print(hdc, x, y, "ESC : cancel");
    break;
  case stScreenshotRenderProceed:
  case stScreenshotRenderCancelRequested:
    print(hdc, x, y, "Saving screenshot:");
    y += lineHeight;
    print(hdc, x, y, scrnshotFileName.c_str());
    y += lineHeight;
    print(hdc, x, y, "Resolution: %ix%i", scrnshotWidth, scrnshotHeight);
    y += lineHeight;
    print(hdc, x, y, "SSAA: %ix", scrnshotSamples);
    y += lineHeight * 2;
    print(hdc, x, y, "Progress: %.2f %%", scrnshotProgress);
    if (scrnshotProgress > VERY_SMALL_NUMBER)
    {
      const DWORD ticksPassed = GetTickCount() - scrnshotStartTicks;
      DWORD ticksLeft = DWORD(ticksPassed * 100 / scrnshotProgress) - ticksPassed;

      int hr = ticksLeft / 3600000;
      ticksLeft = ticksLeft % 3600000;
      int min = ticksLeft / 60000;
      ticksLeft = ticksLeft % 60000;
      int sec = ticksLeft / 1000;
      y += lineHeight;
      print(hdc, x, y, "Estimated time left: %i h %02i m %02i s", hr, min, sec);
      y += lineHeight * 2;

      if (state == stScreenshotRenderCancelRequested)
        print(hdc, x, y, "Do you want to cancel ? ( Y / N ) ");
      else
        print(hdc, x, y, "Press ESC to cancel");
    }
    break;
  }
}

void ProceedControl()
{
  static LARGE_INTEGER prevCounter, counter;
  static const int prevSuccess = QueryPerformanceCounter(&prevCounter);

  float timePassedSec = 0;

  if (initPerfSuccess && prevSuccess && QueryPerformanceCounter(&counter))
    timePassedSec = float(counter.QuadPart - prevCounter.QuadPart) / perfFreq.QuadPart;

  render->camera.proceedControl(controlFlags, timePassedSec);
  prevCounter = counter;
}

void RenderImage()
{
  static int motionDynSamples = Default::motionMinSamples;
  static int prevSamples = 0;
  static bool prevInMotion = false;

  const bool inMotion = controlFlags || render->camera.inMotion();

  if (!imageReady)
  {
    if (!render->inProgress || (inMotion && motionDynSamples != prevSamples))
    {
      if (inMotion)
      {
        if (frameTime > Default::maxMotionFrameTime)
          motionDynSamples = max(motionDynSamples - 1, Default::motionMaxSamples);
        else if (frameTime < Default::minMotionFrameTime)
          motionDynSamples = min(motionDynSamples + 1, Default::motionMinSamples);
      }

      const int reflNum = (inMotion || prevInMotion) ? Default::motionReflections : Default::staticReflections;
      const int sampleNum = (inMotion || prevInMotion) ? motionDynSamples : Default::staticSamples;

      render->renderBegin(reflNum, sampleNum, !(inMotion || prevInMotion));
      scrnshotChunkPixelCount = 1;
      prevSamples = sampleNum;
      prevInMotion = inMotion;
    }

    LARGE_INTEGER cnt0, cnt1;
    const bool cnt0Success = QueryPerformanceCounter(&cnt0) != 0;

    DWORD lastTicks = GetTickCount();
    const bool isComplete = !render->renderNext(scrnshotChunkPixelCount);
    int timePassedMs = int(GetTickCount() - lastTicks);

    if (!isComplete)
    {
      if (timePassedMs < 5)
        scrnshotChunkPixelCount = min(scrnshotChunkPixelCount * 2, render->imageHeight * render->imageWidth);
      else if (timePassedMs > 20)
        scrnshotChunkPixelCount = max(scrnshotChunkPixelCount / 2, 1u);
    }

    if (initPerfSuccess &&
      cnt0Success &&
      QueryPerformanceCounter(&cnt1))
    {
      frameChunkTime += float(cnt1.QuadPart - cnt0.QuadPart) / perfFreq.QuadPart;
    }

    if (isComplete)
    {
      frameTime = frameChunkTime;
      frameChunkTime = 0;
      if (!IsIconic(hWnd))
        imageReady = true;
      InvalidateRect(hWnd, NULL, false);
    }
  }
}

void ScrnshotRenderBegin()
{
  FILETIME ft;
  const int bufSize = 256;
  char name[bufSize];
  GetSystemTimeAsFileTime(&ft);

#ifdef _MSC_VER
  _snprintf(name, bufSize, "scrnshoot_%08X%08X.bmp", ft.dwHighDateTime, ft.dwLowDateTime);
#else
  snprintf(name, bufSize, "scrnshoot_%08X%08X.bmp", ft.dwHighDateTime, ft.dwLowDateTime);
#endif

  scrnshotFileName = exeFullPath;
  scrnshotFileName += name;

  render->setImageSize(scrnshotWidth, scrnshotHeight);
  scrnshotStartTicks = GetTickCount();
  render->renderBegin(Default::scrnshotRefections, scrnshotSamples, false);
  scrnshotChunkPixelCount = 1;
}

bool ScreenshotRenderProceed()
{
  DWORD lastTicks = GetTickCount();
  if (render->renderNext(scrnshotChunkPixelCount))
  {
    int timePassedMs = int(GetTickCount() - lastTicks);

    if (timePassedMs < 10)
      scrnshotChunkPixelCount = min(scrnshotChunkPixelCount * 2, render->imageHeight * render->imageWidth);
    else if (timePassedMs > 100)
      scrnshotChunkPixelCount = max(scrnshotChunkPixelCount / 2, 1u);
    
    scrnshotProgress = render->getRenderProgress();
    InvalidateRect(hWnd, NULL, false);
    return false;
  }
  else
    return true;
}

void ScreenshotRenderSave()
{
  Texture imageTexture(render->imageWidth, render->imageHeight);
  render->copyImage(imageTexture);
  imageTexture.saveToFile(scrnshotFileName.c_str());
}

void ScreenshotRenderEnd()
{
  scrnshotProgress = 0;
  scrnshotWidth = 0;
  scrnshotHeight = 0;
  scrnshotSamples = 0;

  RECT clientRect;
  if (GetClientRect(hWnd, &clientRect) && clientRect.right && clientRect.bottom)
    render->setImageSize(clientRect.right, clientRect.bottom);
  else
    render->setImageSize(1, 1);
}

void Pulse()
{
  switch (state)
  {
  case stCameraControl:
    ProceedControl();
    RenderImage();
    break;
  case stScreenshotResolutionSelection:
  case stScreenshotSamplingSelection:
    Sleep(10);
    break;
  case stScreenshotRenderBegin:
    ScrnshotRenderBegin();
    state = stScreenshotRenderProceed;
    break;
  case stScreenshotRenderProceed:
  case stScreenshotRenderCancelRequested:
    if (ScreenshotRenderProceed())
      state = stScreenshotRenderSave;
    break;
  case stScreenshotRenderSave:
    ScreenshotRenderSave();
    state = stScreenshotRenderEnd;
    break;
  case stScreenshotRenderEnd:
    ScreenshotRenderEnd();
    state = stCameraControl;
    break;
    break;
  }
}

// -----------------------------------------------------------------------------
//                                 Events
// -----------------------------------------------------------------------------

void OnResize(const HWND hWnd, const int width, const int height)
{
  #ifdef _MSC_VER
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
  #endif

  if (state == stCameraControl)
  {
    RECT clientRect;
    if (GetClientRect(hWnd, &clientRect))
    {
      const unsigned int width = clientRect.right;
      const unsigned int height = clientRect.bottom;
      if (width && height && (width != render->imageWidth || height != render->imageHeight))
        render->setImageSize(width, height);
    }
  }
}

void OnPaint(const HWND hWnd, const HDC hdc)
{
  RECT clientRect;
  if (GetClientRect(hWnd, &clientRect) && clientRect.right && clientRect.bottom)
  {
    const int width = clientRect.right;
    const int height = clientRect.bottom;
    const HBITMAP bm = CreateCompatibleBitmap(hdc, width, height);
    const HDC dc = CreateCompatibleDC(hdc);
    SelectObject(dc, bm);

    DrawImage(hWnd, dc);
    DrawTextTips(dc);

    BitBlt(hdc, 0, 0, width, height, dc, 0, 0, SRCCOPY);
    DeleteDC(dc);
    DeleteObject(bm);
  }
}

void OnKeyDown(const int Key)
{
  switch (Key)
  {
  case VK_LEFT:
    controlFlags |= turnLeftMask;
    break;
  case VK_RIGHT:
    controlFlags |= turnRightMask;
    break;
  case VK_UP:
    controlFlags |= turnDownMask;
    break;
  case VK_DOWN:
    controlFlags |= turnUpMask;
    break;
  case 'W':
    controlFlags |= shiftForwardMask;
    break;
  case 'S':
    controlFlags |= shiftBackMask;
    break;
  case 'A':
    controlFlags |= shiftLeftMask;
    break;
  case 'D':
    controlFlags |= shiftRightMask;
    break;
  case ' ':
    controlFlags |= shiftUpMask;
    break;
  case VK_CONTROL:
    controlFlags |= shiftDownMask;
    break;
  case VK_F2:
    if (state == stCameraControl)
      state = stScreenshotResolutionSelection;
    break;
  case '1':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 640;
      scrnshotHeight = 480;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 1;
      state = stScreenshotRenderBegin;
    }
    break;
  case '2':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 800;
      scrnshotHeight = 600;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 2;
      state = stScreenshotRenderBegin;
    }
    break;
  case '3':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1024;
      scrnshotHeight = 768;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 4;
      state = stScreenshotRenderBegin;
    }
    break;
  case '4':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1280;
      scrnshotHeight = 960;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 8;
      state = stScreenshotRenderBegin;
    }
    break;
  case '5':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1280;
      scrnshotHeight = 800;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 16;
      state = stScreenshotRenderBegin;
    }
    break;
  case '6':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1680;
      scrnshotHeight = 1050;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 32;
      state = stScreenshotRenderBegin;
    }
    break;
  case '7':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1920;
      scrnshotHeight = 1200;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 64;
      state = stScreenshotRenderBegin;
    }
    break;
  case '8':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1280;
      scrnshotHeight = 720;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 128;
      state = stScreenshotRenderBegin;
    }
    break;
  case '9':
    if (state == stScreenshotResolutionSelection)
    {
      scrnshotWidth = 1920;
      scrnshotHeight = 1080;
      state = stScreenshotSamplingSelection;
    }
    else if (state == stScreenshotSamplingSelection)
    {
      scrnshotSamples = 256;
      state = stScreenshotRenderBegin;
    }
    break;
  case VK_ESCAPE:
    if (state == stScreenshotResolutionSelection || state == stScreenshotSamplingSelection)
      state = stCameraControl;
    else if (state == stScreenshotRenderProceed)
      state = stScreenshotRenderCancelRequested;
    break;
  case 'Y':
    if (state == stScreenshotRenderCancelRequested)
      state = stCameraControl;
  case 'N':
    if (state == stScreenshotRenderCancelRequested)
      state = stScreenshotRenderProceed;
  }
}

void OnKeyUp(const int Key)
{
  switch (Key)
  {
  case VK_LEFT:
    controlFlags &= ~turnLeftMask;
    break;
  case VK_RIGHT:
    controlFlags &= ~turnRightMask;
    break;
  case VK_UP:
    controlFlags &= ~turnDownMask;
    break;
  case VK_DOWN:
    controlFlags &= ~turnUpMask;
    break;
  case 'W':
    controlFlags &= ~shiftForwardMask;
    break;
  case 'S':
    controlFlags &= ~shiftBackMask;
    break;
  case 'A':
    controlFlags &= ~shiftLeftMask;
    break;
  case 'D':
    controlFlags &= ~shiftRightMask;
    break;
  case ' ':
    controlFlags &= ~shiftUpMask;
    break;
  case VK_CONTROL:
    controlFlags &= ~shiftDownMask;
    break;
  }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;

  switch (message)
  {
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);

    if (hdc) 
      OnPaint(hWnd, hdc);

    EndPaint(hWnd, &ps);
    break;
  case WM_ERASEBKGND:
    return 1;
  case WM_DESTROY:
    quitMessage = true;
    PostQuitMessage(0);
    break;
  case WM_SIZE:
    OnResize(hWnd, LOWORD(lParam), HIWORD(lParam));
    break;
  case WM_KEYDOWN:
    OnKeyDown(int(wParam));
    break;
  case WM_KEYUP:
    OnKeyUp(int(wParam));
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  #ifdef _MSC_VER
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
  #endif

  WNDCLASSEXA wcex;
  wcex.cbSize = sizeof(wcex);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WindowProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = NULL;
  wcex.hCursor = NULL;
  wcex.hbrBackground = (HBRUSH)CreateSolidBrush(0xFF80FF);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = "clReflaxWindow";
  wcex.hIconSm = NULL;

  RegisterClassExA(&wcex);

  hWnd = CreateWindowA("clReflaxWindow", "Reflax", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
    640, 480, NULL, NULL, hInstance, NULL);
  font = CreateFontA(12, -4, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, 
    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Aria");

  char exeFullName[MAX_PATH];
  char * lpExeName;
  GetModuleFileNameA(NULL, exeFullName, MAX_PATH);
  GetFullPathNameA(exeFullName, MAX_PATH, exeFullPath, &lpExeName);
  *lpExeName = '\0';

  render = new Render(exeFullPath);

  if (hWnd)
  {
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    while (!quitMessage)
    {
      STATE lastState = state;

      while (!quitMessage && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      Pulse();

      if (state != lastState)
        InvalidateRect(hWnd, NULL, false);
    }

    if (memDC)
      DeleteDC(memDC);

    if (bitmap)
      DeleteObject(bitmap);

    if (font)
      DeleteObject(font);

    if (render)
      delete render;

    return (int)msg.wParam;
  }
  else
    return FALSE;

}

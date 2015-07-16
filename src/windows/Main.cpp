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
int bmWidth = 0;
int bmHeight = 0;
bool imageReady = false;
float frameChunkTime = 0.0f;
float frameTime = 0.0f;
char exeFullPath[MAX_PATH];
bool scrnshotSaving = false;
float scrnshotProgress = -1;
DWORD scrnshotStartTicks = 0;
std::string scrnshotFileName;
bool scrnshotCancelRequested = false;
bool scrnshotCancelConfirmed = false;
LARGE_INTEGER perfFreq = { 0, 0 };
bool initPerfSuccess = (QueryPerformanceFrequency(&perfFreq) && perfFreq.QuadPart > 0);
bool quitMessage = false;
int controlFlags = 0;

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

void ProceedControl()
{
  static DWORD prevTicks = GetTickCount();
  const DWORD curTicks = GetTickCount();
  if (int(curTicks - prevTicks) > 20)
  {
    render->camera.proceedControl(controlFlags, int(curTicks - prevTicks));
    prevTicks = curTicks;
  }
}

void DrawScreenshotStats(const HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, 0);
  const int lineHeight = tm.tmHeight + 1;
  const int x = 3;
  int y = 3;

  print(hdc, x, y, "Saving screenshot:");
  
  y += lineHeight;
  print(hdc, x, y, scrnshotFileName.c_str());

  y += lineHeight;
  print(hdc, x, y, "Resolution: %ix%i", Default::scrnshotWidth, Default::scrnshotHeight);

  y += lineHeight;
  print(hdc, x, y, "SSAA: %ix", Default::scrnshotSamples);

  

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
  }

  if (scrnshotCancelRequested)
  {
    y += lineHeight * 2;
    print(hdc, x, y, "Do you want to cancel ? ( Y / N ) ");
  }
  else
  {
    y += lineHeight * 2;
    print(hdc, x, y, "Press ESC to cancel");
  }
}

void DrawSceneStats(const HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, 0);
  const int lineHeight = tm.tmHeight + 1;
  const int x = 3;
  int y = 3;

  if (frameTime < 10)
    print(hdc, x, y, "Frame time: %.0f ms", frameTime * 1000);
  else
    print(hdc, x, y, "Frame time: %.03f s", frameTime);

  y += lineHeight;
  print(hdc, x, y, "Blended frames : %i", render->additiveCounter);

  y += lineHeight * 2;
  print(hdc, x, y, "WSAD : moving");

  y += lineHeight;
  print(hdc, x, y, "Cursor : turning");

  y += lineHeight;
  print(hdc, x, y, "Space : ascenting");

  y += lineHeight;
  print(hdc, x, y, "Ctrl : descenting");

  y += lineHeight * 2;
  print(hdc, x, y, "F2 : save screenshot");
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
      bmi.bmiHeader.biWidth = max(render->imageWidth, 1);
      bmi.bmiHeader.biHeight = max(render->imageHeight, 1);
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
      for (int y = 0; y < bmHeight; ++y)
      for (int x = 0; x < bmWidth; ++x)
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

void ScrnshotSavingPulse()
{
  if (scrnshotProgress < 0)
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

    render->setImageSize(Default::scrnshotWidth, Default::scrnshotHeight);
    scrnshotStartTicks = GetTickCount();
    render->renderBegin(Default::scrnshotRefections, Default::scrnshotSamples, false);
  }

  if (!scrnshotCancelConfirmed && render->renderNext(1))
  {
    scrnshotProgress = render->getRenderProgress();
    InvalidateRect(hWnd, NULL, false);
  }
  else
  {
    if (!scrnshotCancelConfirmed)
    {
      Texture imageTexture(render->imageWidth, render->imageHeight);
      render->copyImage(imageTexture);
      imageTexture.saveToFile(scrnshotFileName.c_str());
    }
    scrnshotProgress = -1;
    scrnshotSaving = false;
    scrnshotCancelConfirmed = false;

    RECT clientRect;
    if (GetClientRect(hWnd, &clientRect) && clientRect.right && clientRect.bottom)
      render->setImageSize(clientRect.right, clientRect.bottom);
    else
      render->setImageSize(1, 1);
  }
}

void ImageRenderPulse()
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
      prevSamples = sampleNum;
      prevInMotion = inMotion;
    }

    LARGE_INTEGER cnt0, cnt1;
    const bool cnt0Success = QueryPerformanceCounter(&cnt0) != 0;

    const int linesLeft = render->renderNext(1);

    if (initPerfSuccess &&
      cnt0Success &&
      QueryPerformanceCounter(&cnt1))
    {
      frameChunkTime += float(cnt1.QuadPart - cnt0.QuadPart) / perfFreq.QuadPart;
    }

    if (!linesLeft)
    {
      frameTime = frameChunkTime;
      frameChunkTime = 0;
      if (!IsIconic(hWnd))
        imageReady = true;
      InvalidateRect(hWnd, NULL, false);
    }
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

  if (!scrnshotSaving)
  {
    RECT clientRect;
    if (GetClientRect(hWnd, &clientRect))
    {
      const int width = clientRect.right;
      const int height = clientRect.bottom;
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
    if (scrnshotProgress >= 0)
      DrawScreenshotStats(dc);
    else
      DrawSceneStats(dc);

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
    scrnshotSaving = true;
    break;
  case VK_ESCAPE:
    if (scrnshotSaving)
      scrnshotCancelRequested = true;
    break;
  case 'Y':
    if (scrnshotCancelRequested)
    {
      scrnshotCancelRequested = false;
      scrnshotCancelConfirmed = true;
    }
  case 'N':
    if (scrnshotCancelRequested)
      scrnshotCancelRequested = false;
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
      while (!quitMessage && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      if (scrnshotSaving)
      {
        ScrnshotSavingPulse();
      }
      else
      {
        ProceedControl();
        ImageRenderPulse();
      }
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

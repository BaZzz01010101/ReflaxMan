#define NOMINMAX
#include <windows.h>
#include "stdafx.h"
#include "..\airly\airly.h"
#include "..\airly\Render.h"

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
bool scrnshotRequested = false;
float scrnshotProgress = -1;
DWORD scrnshotStartTicks = 0;
LARGE_INTEGER perfFreq = { 0, 0 };
bool initPerfSuccess = (QueryPerformanceFrequency(&perfFreq) && perfFreq.QuadPart > 0);
bool quitMessage = false;
int controlFlags = 0;

void print(HDC hdc, int x, int y, const char* format, ...)
{
  const int buf_len = 256;
  char buffer[buf_len];

  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  SetTextColor(hdc, 0xAAAAAA);
  TextOutA(hdc, x-1, y-1, buffer, strlen(buffer));
  TextOutA(hdc, x+1, y-1, buffer, strlen(buffer));
  TextOutA(hdc, x-1, y+1, buffer, strlen(buffer));
  TextOutA(hdc, x+1, y+1, buffer, strlen(buffer));
  SetTextColor(hdc, 0);
  TextOutA(hdc, x, y, buffer, strlen(buffer));
}

void ProceedControl()
{
  static DWORD prevTicks = GetTickCount();
  DWORD curTicks = GetTickCount();
  render->camera.proceedControl(controlFlags, int(curTicks - prevTicks));
  prevTicks = curTicks;
}

void DrawScreenshotStats(HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, 0);
  int y = 0;

  print(hdc, 0, y, "Saving screenshot");
  y += tm.tmHeight;

  print(hdc, 0, y, "Progress: %.2f %%", scrnshotProgress);
  y += tm.tmHeight;

  DWORD ticksPassed = GetTickCount() - scrnshotStartTicks;
  DWORD ticksLeft = DWORD(ticksPassed * 100 / scrnshotProgress) - ticksPassed;

  int hr = ticksLeft / 3600000;
  ticksLeft = ticksLeft % 3600000;
  int min = ticksLeft / 60000;
  ticksLeft = ticksLeft % 60000;
  int sec = ticksLeft / 1000;
  print(hdc, 0, y, "Estimated time left: %i h %02i m %02i s", hr, min, sec);
}

void DrawSceneStats(HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, 0);
  int y = 0;

  if (frameTime < 10)
    print(hdc, 0, y, "frame time: %.0f ms", frameTime * 1000);
  else
    print(hdc, 0, y, "frame time: %.03f s", frameTime);

  y += tm.tmHeight;
  print(hdc, 0, y, "camera eye: [%.03f, %.03f, %.03f]", render->camera.eye.x, render->camera.eye.y, render->camera.eye.z);

  y += tm.tmHeight;
  Vector3 at = render->camera.eye + render->camera.view.getCol(2);
  print(hdc, 0, y, "camera at: [%.03f, %.03f, %.03f]", at.x, at.y, at.z);

  y += tm.tmHeight;
  Vector3 up = render->camera.view.getCol(1);
  print(hdc, 0, y, "camera up: [%.03f, %.03f, %.03f]", up.x, up.y, up.z);

  y += tm.tmHeight;
  print(hdc, 0, y, "yaw / pitch: %.03f / %.03f", render->camera.yaw, render->camera.pitch);

  y += tm.tmHeight;
  print(hdc, 0, y, "turnSpeed: RL:%.03f UD:%.03f", render->camera.turnRLSpeed, render->camera.turnUDSpeed);

  y += tm.tmHeight;
  print(hdc, 0, y, "shiftSpeed: RL:%.03f UD:%.03f FB:%.03f", render->camera.shiftRLSpeed, render->camera.shiftUDSpeed, render->camera.shiftFBSpeed);
}

void DrawImage(HWND hWnd, HDC hdc)
{
  BITMAPINFO bmi;

  if (imageReady)
  {
    if (bmWidth != render->image.getWidth() || bmHeight != render->image.getHeight())
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

      bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
      bmi.bmiHeader.biWidth = max(render->image.getWidth(), 1);
      bmi.bmiHeader.biHeight = max(render->image.getHeight(), 1);
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
        bmWidth = render->image.getWidth();
        bmHeight = render->image.getHeight();
      }
      else
      {
        bmWidth = 0;
        bmHeight = 0;
      }
    }

    for (int y = 0; y < bmHeight; ++y)
    for (int x = 0; x < bmWidth; ++x)
      pixels[x + y * bmWidth] = render->getImagePixel(x, y).argb();
    
    imageReady = false;
  }

  if (memDC && pixels)
  {
    RECT clientRect;
    if (!GetClientRect(hWnd, &clientRect))
      clientRect.left = clientRect.right = clientRect.top = clientRect.bottom = 0;

    SetStretchBltMode(hdc, HALFTONE);
    StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, bmWidth, bmHeight, SRCCOPY);
  }
}

// -----------------------------------------------------------------------------
//                                 Events
// -----------------------------------------------------------------------------

void OnResize(HWND hWnd, int width, int height)
{
  UNREFERENCED_PARAMETER(width);
  UNREFERENCED_PARAMETER(height);

  RECT clientRect;
  if (GetClientRect(hWnd, &clientRect) && clientRect.right && clientRect.bottom)
    render->setImageSize(clientRect.right, clientRect.bottom);
  else
    render->setImageSize(1, 1);
}

void OnPaint(HWND hWnd, HDC hdc)
{
  if (scrnshotProgress >= 0)
  {
    RECT r;
    GetClientRect(hWnd, &r);
    FillRect(hdc, &r, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
    DrawScreenshotStats(hdc);
  }
  else
  {
    DrawImage(hWnd, hdc);
    DrawSceneStats(hdc);
  }
}

void OnKeyDown(int Key)
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
    scrnshotRequested = true;
    break;
  }
}

void OnKeyUp(int Key)
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
    OnKeyDown(wParam);
    break;
  case WM_KEYUP:
    OnKeyUp(wParam);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  font = CreateFontA(-9, -4, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Aria");

  char exeFullName[MAX_PATH];
  char* lpExeName;
  GetModuleFileNameA(NULL, exeFullName, MAX_PATH);
  GetFullPathNameA(exeFullName, MAX_PATH, exeFullPath, &lpExeName);
  *lpExeName = '\0';

  render = new Render(exeFullPath);

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

  HWND hWnd = CreateWindowA("clReflaxWindow", "Reflax", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

  if (hWnd)
  {
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    int movingSampleNum = -4;
    int normalSampleNum = 1;
    int sampleNum = normalSampleNum;

    while (!quitMessage)
    {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      if (scrnshotRequested)
      {
        if (scrnshotProgress < 0)
        {
          const int scrnshotWidth = 1680;
          const int scrnshotHeight = 1050;
          const int scrnshotRefections = 7;
          const int scrnshotSampleNumber = 2;

          render->setImageSize(scrnshotWidth, scrnshotHeight);

          scrnshotStartTicks = GetTickCount();
          render->renderBegin(scrnshotRefections, scrnshotSampleNumber, false);
        }

        if (render->renderNext(1))
        {
          scrnshotProgress = render->getRenderProgress();
          InvalidateRect(hWnd, NULL, false);
        }
        else
        {
          FILETIME ft;
          const int bufSize = 256;
          char name[bufSize];
          GetSystemTimeAsFileTime(&ft);
          sprintf(name, "scrnshoot_%08X%08X.bmp", ft.dwHighDateTime, ft.dwLowDateTime);
          std::string str = std::string(exeFullPath) + name;
          render->image.saveToFile(str.c_str());
          scrnshotProgress = -1;
          scrnshotRequested = false;

          RECT clientRect;
          if (GetClientRect(hWnd, &clientRect) && clientRect.right && clientRect.bottom)
            render->setImageSize(clientRect.right, clientRect.bottom);
          else
            render->setImageSize(1, 1);
        }
      }
      else if (GetForegroundWindow() == hWnd)
      {
        ProceedControl();
        bool inMotion = controlFlags || render->camera.inMotion();

        if (!render->inProgress() || (inMotion && sampleNum != movingSampleNum))
        {
          int prevSampleNum = sampleNum;
          sampleNum = inMotion ? movingSampleNum : normalSampleNum;
          bool renderAdditive = (!inMotion && sampleNum == prevSampleNum);
          render->renderBegin(7, sampleNum, renderAdditive);
        }

        if (!imageReady)
        {
          LARGE_INTEGER cnt0, cnt1;
          bool cnt0Success = QueryPerformanceCounter(&cnt0) != 0;

          int linesLeft = render->renderNext(1);

          if (initPerfSuccess &&
            cnt0Success &&
            QueryPerformanceCounter(&cnt1) &&
            cnt1.QuadPart != cnt0.QuadPart)
          {
            frameChunkTime += float(cnt1.QuadPart - cnt0.QuadPart) / perfFreq.QuadPart;
          }

          if (!linesLeft)
          {
            frameTime = frameChunkTime;
            frameChunkTime = 0;
            imageReady = true;
            InvalidateRect(hWnd, NULL, false);
          }
        }
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

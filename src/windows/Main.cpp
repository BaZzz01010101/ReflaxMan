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
float scrnshotProgress = -1;
DWORD scrnshotStartTicks = 0;
LARGE_INTEGER perfFreq = { 0, 0 };
bool initPerfSuccess = (QueryPerformanceFrequency(&perfFreq) && perfFreq.QuadPart > 0);
bool quitMessage = false;
int controlFlags = 0;
const int turnLeftMask = 1 << 0;
const int turnRightMask = 1 << 1;
const int turnUpMask = 1 << 2;
const int turnDownMask = 1 << 3;
const int turnCwizeMask = 1 << 4;
const int turnCcwizeMask = 1 << 5;
const int shiftLeftMask = 1 << 6;
const int shiftRightMask = 1 << 7;
const int shiftUpMask = 1 << 8;
const int shiftDownMask = 1 << 9;
const int shiftForwardMask = 1 << 10;
const int shiftBackMask = 1 << 11;

void print(HDC dc, int x, int y, const char* format, ...)
{
  const int buf_len = 256;
  char buffer[buf_len];

  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);

  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, 0xFFFFFF);
  TextOutA(dc, x, y, buffer, strlen(buffer));
}

void ProceedControl()
{
  if (controlFlags & turnLeftMask)
    render->camera.rotate(Camera::Rotation::crLeft);
  if (controlFlags & turnRightMask)
    render->camera.rotate(Camera::Rotation::crRight);
  if (controlFlags & turnUpMask)
    render->camera.rotate(Camera::Rotation::crUp);
  if (controlFlags & turnDownMask)
    render->camera.rotate(Camera::Rotation::crDown);
  if (controlFlags & turnCwizeMask)
    render->camera.rotate(Camera::Rotation::crCwise);
  if (controlFlags & turnCcwizeMask)
    render->camera.rotate(Camera::Rotation::crCcwise);
  if (controlFlags & shiftLeftMask)
    render->camera.move(Camera::Movement::cmLeft);
  if (controlFlags & shiftRightMask)
    render->camera.move(Camera::Movement::cmRight);
  if (controlFlags & shiftUpMask)
    render->camera.move(Camera::Movement::cmUp);
  if (controlFlags & shiftDownMask)
    render->camera.move(Camera::Movement::cmDown);
  if (controlFlags & shiftForwardMask)
    render->camera.move(Camera::Movement::cmForward);
  if (controlFlags & shiftBackMask)
    render->camera.move(Camera::Movement::cmBack);
}

void DrawImage(HWND hWnd, HDC hdc)
{
  RECT clientRect;
  BITMAPINFO bmi;
  if (!GetClientRect(hWnd, &clientRect))
    clientRect.left = clientRect.right = clientRect.top = clientRect.bottom = 0;

  if (imageReady)
  {
    if (clientRect.right != bmWidth || clientRect.bottom != bmHeight)
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
      bmi.bmiHeader.biWidth = max(clientRect.right, 1);
      bmi.bmiHeader.biHeight = max(clientRect.bottom, 1);
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
        bmWidth = clientRect.right;
        bmHeight = clientRect.bottom;
      }
      else
      {
        bmWidth = 0;
        bmHeight = 0;
      }
    }

    for (int y = 0; y < bmHeight; ++y)
    for (int x = 0; x < bmWidth; ++x)
      pixels[x + y * bmWidth] =
      //pixels[x + y * render->screenWidth] ?
      //((Color(pixels[x + y * render->screenWidth]) * 10 + render->tracePixel(x, y, 1)) / 11).argb() :
      render->getImagePixel(x, y).argb();

    imageReady = false;
  }

  if (memDC && pixels)
  {
    SetStretchBltMode(hdc, HALFTONE);
    StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, bmWidth, bmHeight, SRCCOPY);
  }
}

void DrawStats(HWND hWnd, HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);
  int y = 0;

  if (scrnshotProgress >= 0)
  {
    RECT r;
    GetClientRect(hWnd, &r);
    FillRect(hdc, &r, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

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
  else
  {
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
  }
}

// -----------------------------------------------------------------------------
//                                 Events
// -----------------------------------------------------------------------------

void OnResize(int width, int height)
{
  render->setImageSize(width, height);
}

void OnPaint(HWND hWnd, HDC hdc)
{
  DrawImage(hWnd, hdc);
  DrawStats(hWnd, hdc);
}

void OnKeyDown(int Key)
{
  switch (Key)
  {
  case 'A':
    controlFlags |= shiftForwardMask;
    break;
  case 'Z':
    controlFlags |= shiftBackMask;
    break;
  case VK_LEFT:
    controlFlags |= turnLeftMask;
    break;
  case VK_RIGHT:
    controlFlags |= turnRightMask;
    break;
  case VK_UP:
    controlFlags |= turnUpMask;
    break;
  case VK_DOWN:
    controlFlags |= turnDownMask;
    break;
  case VK_INSERT:
    controlFlags |= turnCcwizeMask;
    break;
  case VK_PRIOR:
    controlFlags |= turnCwizeMask;
    break;
  case VK_DELETE:
    controlFlags |= shiftLeftMask;
    break;
  case VK_NEXT:
    controlFlags |= shiftRightMask;
    break;
  case VK_HOME:
    controlFlags |= shiftUpMask;
    break;
  case VK_END:
    controlFlags |= shiftDownMask;
    break;
  }
}

void OnKeyUp(int Key)
{
  switch (Key)
  {
  case 'A':
    controlFlags &= ~shiftForwardMask;
    break;
  case 'Z':
    controlFlags &= ~shiftBackMask;
    break;
  case VK_LEFT:
    controlFlags &= ~turnLeftMask;
    break;
  case VK_RIGHT:
    controlFlags &= ~turnRightMask;
    break;
  case VK_UP:
    controlFlags &= ~turnUpMask;
    break;
  case VK_DOWN:
    controlFlags &= ~turnDownMask;
    break;
  case VK_INSERT:
    controlFlags &= ~turnCcwizeMask;
    break;
  case VK_PRIOR:
    controlFlags &= ~turnCwizeMask;
    break;
  case VK_DELETE:
    controlFlags &= ~shiftLeftMask;
    break;
  case VK_NEXT:
    controlFlags &= ~shiftRightMask;
    break;
  case VK_HOME:
    controlFlags &= ~shiftUpMask;
    break;
  case VK_END:
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
    OnResize(LOWORD(lParam), HIWORD(lParam));
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

      if (GetForegroundWindow() == hWnd)
      {

        ProceedControl();

        if (!render->inProgress() || (controlFlags && sampleNum != movingSampleNum))
        {
          sampleNum = controlFlags ? movingSampleNum : normalSampleNum;
          render->renderBegin(7, sampleNum);
        }

        if (!imageReady)
        {
          LARGE_INTEGER cnt0, cnt1;
          bool cnt0Success = QueryPerformanceCounter(&cnt0) != 0;

          int linesLeft = render->renderNext(10);

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

        if (GetKeyState(VK_F2) & 0x8000)
        {
          Render shotRender(exeFullPath);
          shotRender.camera = render->camera;
          const int w = 1680;
          const int h = 1050;
          const int refls = 7;
          const int aan = 4;

          shotRender.setImageSize(w, h);

          scrnshotStartTicks = GetTickCount();
          DWORD lastTicks = scrnshotStartTicks;
          shotRender.renderBegin(refls, aan);
          while (!shotRender.renderNext(10))
          {
            if (int(GetTickCount() - lastTicks) > 100)
            {
              lastTicks = GetTickCount();
              scrnshotProgress = shotRender.getRenderProgress();
              InvalidateRect(hWnd, NULL, false);
            }

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
              if (msg.message == WM_QUIT)
                goto EXIT;

              TranslateMessage(&msg);
              DispatchMessage(&msg);
            }
          }

          FILETIME ft;
          const int bufSize = 256;
          char name[bufSize];
          GetSystemTimeAsFileTime(&ft);
          sprintf(name, "scrnshoot_%08X%08X.bmp", ft.dwHighDateTime, ft.dwLowDateTime);
          std::string str = std::string(exeFullPath) + name;
          shotRender.image.saveToFile(str.c_str());

          scrnshotProgress = -1;
        }
      }
    }

  EXIT:
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

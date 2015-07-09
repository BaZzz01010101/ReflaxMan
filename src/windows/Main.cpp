#include <windows.h>
#include "stdafx.h"
#include "..\airly\airly.h"
#include "..\airly\Scene.h"

Scene * scene = NULL;
HFONT font = NULL;
HDC memDC = NULL;
HBITMAP bitmap = NULL;
DWORD * pixels = NULL;
wchar_t exeFullPath[MAX_PATH];
float scrnshotProgress = -1;
DWORD scrnshotStartTicks = 0;

void print(HDC dc, int x, int y, const char* format, ...)
{
  const int buf_len = 256;
  char buffer[buf_len];

  va_list args;
  va_start(args, format);
  vsprintf_s(buffer, buf_len, format, args);
  perror(buffer);
  va_end(args);

  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, 0xFFFFFF);
  TextOutA(dc, x, y, buffer, strlen(buffer));
}

bool DrawScene(HWND hWnd, HDC hdc)
{
  for (int y = 0; y < scene->screenHeight; ++y)
  for (int x = 0; x < scene->screenWidth; ++x)
    pixels[x + y * scene->screenWidth] =
    //pixels[x + y * scene->screenWidth] ?
    //((Color(pixels[x + y * scene->screenWidth]) * 10 + scene->tracePixel(x, y, 1)) / 11).argb() :
    scene->tracePixel(x, y, 1).argb();

  RECT clientRect;

  if (memDC &&
      GetClientRect(hWnd, &clientRect) &&
      BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY))
    return true;
  else
    return false;
}

void OnResize(int width, int height)
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

  scene->screenWidth = width;
  scene->screenHeight = height;

  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = (width >= 1 ? width : 1);
  bmi.bmiHeader.biHeight = (height >= 1 ? height : 1);
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
      SelectObject(memDC, bitmap);
}

void OnPaint(HWND hWnd, HDC hdc)
{
  TEXTMETRICA tm;
  SelectObject(hdc, font);
  GetTextMetricsA(hdc, &tm);

  if (scrnshotProgress >= 0)
  {
    RECT r;
    GetClientRect(hWnd, &r);
    FillRect(hdc, &r, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

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
  else
  {
    static LARGE_INTEGER freq = { 0, 0 };
    static bool initSuccess = (QueryPerformanceFrequency(&freq) && 
                               freq.QuadPart > 0);

    LARGE_INTEGER cnt0, cnt1;
    bool cnt0Success = QueryPerformanceCounter(&cnt0) != 0;

    DrawScene(hWnd, hdc);

    if (initSuccess && 
      cnt0Success && 
      QueryPerformanceCounter(&cnt1) && 
      cnt1.QuadPart != cnt0.QuadPart)
    {
      float frameTime = float(cnt1.QuadPart - cnt0.QuadPart) / freq.QuadPart;
      int y = 0;

      if (frameTime < 10)
        print(hdc, 0, y, "frame time: %.0f ms", frameTime * 1000);
      else
        print(hdc, 0, y, "frame time: %.03f s", frameTime);

      y += tm.tmHeight;
      print(hdc, 0, y, "camera eye: [%.03f, %.03f, %.03f]", scene->camera.eye.x, scene->camera.eye.y, scene->camera.eye.z);

      y += tm.tmHeight;
      Vector3 at = scene->camera.eye + scene->camera.view.getCol(2);
      print(hdc, 0, y, "camera at: [%.03f, %.03f, %.03f]", at.x, at.y, at.z);

      y += tm.tmHeight;
      Vector3 up = scene->camera.view.getCol(1);
      print(hdc, 0, y, "camera up: [%.03f, %.03f, %.03f]", up.x, up.y, up.z);
    }
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
    PostQuitMessage(0);
    break;
  case WM_SIZE:
    OnResize(LOWORD(lParam), HIWORD(lParam));
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  font = CreateFontW(-9, -4, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, L"Arial");

  wchar_t exeFullName[MAX_PATH];
  wchar_t* lpExeName;
  GetModuleFileNameW(NULL, exeFullName, MAX_PATH);
  GetFullPathNameW(exeFullName, MAX_PATH, exeFullPath, &lpExeName);
  *lpExeName = '\0';

  scene = new Scene(exeFullPath);

  WNDCLASSEXW wcex;
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
  wcex.lpszClassName = L"clReflaxWindow";
  wcex.hIconSm = NULL;

  RegisterClassExW(&wcex);

  HWND hWnd = CreateWindowW(L"clReflaxWindow", L"Reflax", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

  if (hWnd)
  {
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);

      if (GetForegroundWindow() == hWnd)
      {
        if (GetKeyState('A') & 0x8000)
          scene->camera.move(Camera::Movement::cmForward);
        if (GetKeyState('Z') & 0x8000)
          scene->camera.move(Camera::Movement::cmBack);
        if (GetKeyState(VK_LEFT) & 0x8000)
          scene->camera.rotate(Camera::Rotation::crLeft);
        if (GetKeyState(VK_RIGHT) & 0x8000)
          scene->camera.rotate(Camera::Rotation::crRight);
        if (GetKeyState(VK_UP) & 0x8000)
          scene->camera.rotate(Camera::Rotation::crUp);
        if (GetKeyState(VK_DOWN) & 0x8000)
          scene->camera.rotate(Camera::Rotation::crDown);
        if (GetKeyState(VK_INSERT) & 0x8000)
          scene->camera.rotate(Camera::Rotation::crCcwise);
        if (GetKeyState(VK_PRIOR) & 0x8000)
          scene->camera.rotate(Camera::Rotation::crCwise);
        if (GetKeyState(VK_DELETE) & 0x8000)
          scene->camera.move(Camera::Movement::cmLeft);
        if (GetKeyState(VK_NEXT) & 0x8000)
          scene->camera.move(Camera::Movement::cmRight);
        if (GetKeyState(VK_HOME) & 0x8000)
          scene->camera.move(Camera::Movement::cmUp);
        if (GetKeyState(VK_END) & 0x8000)
          scene->camera.move(Camera::Movement::cmDown);

        if (GetKeyState(VK_F2) & 0x8000)
        {
          Scene shotScene(exeFullPath);
          shotScene.camera = scene->camera;
          const int w = 1680;
          const int h = 1050;
          Texture shot(w, h);
          const int aan = 64;

          shotScene.screenWidth = w;
          shotScene.screenHeight = h;
          ARGB * pixels = shot.getColorBuffer();

          scrnshotStartTicks = GetTickCount();
          DWORD lastTicks = scrnshotStartTicks;

          for (int y = 0; y < h; ++y)
          {
            for (int x = 0; x < w; ++x)
            {
              pixels[x + y * w] = shotScene.tracePixel(x, y, aan).argb();

              if (int(GetTickCount() - lastTicks) > 100)
              {
                lastTicks = GetTickCount();
                scrnshotProgress = (x + y * w) * 100.0f / w / h;
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

          }

          FILETIME ft;
          const int bufSize = 256;
          wchar_t name[bufSize];
          GetSystemTimeAsFileTime(&ft);
          swprintf_s(name, bufSize, L"scrnshoot_%08X%08X.bmp", ft.dwHighDateTime, ft.dwLowDateTime);
          std::wstring str = std::wstring(exeFullPath) + name;
          shot.saveToFile(str.c_str());

          scrnshotProgress = -1;
        }
        InvalidateRect(hWnd, NULL, false);
      }
    }

  EXIT:
    if (memDC)
      DeleteDC(memDC);

    if (bitmap)
      DeleteObject(bitmap);

    if (font)
      DeleteObject(font);

    if (scene)
      delete scene;

    return (int)msg.wParam;
  }
  else
    return FALSE;

}

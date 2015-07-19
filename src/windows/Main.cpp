#define NOMINMAX
#include <windows.h>
#include "stdafx.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4701)
#endif

#include "WindowsPlatformInterface.h"
#include "..\common\Pulse.h"
#include "..\common\trace_math.h"

WindowsPlatformInterface * plint = NULL;
Pulse * pulse = NULL;

HWND hWnd = NULL;
HFONT font = NULL;
HDC memDC = NULL;
HBITMAP bitmap = NULL;
DWORD * pixels = NULL;
unsigned int clientWidth = 0;
unsigned int clientHeight = 0;
unsigned int bmWidth = 0;
unsigned int bmHeight = 0;
bool quitMessage = false;

void OutlinePrint(const HDC hdc, const int x, const int y, const char* str)
{
  SetTextColor(hdc, 0xAAAAAA);
  TextOutA(hdc, x - 1, y - 1, str, (int)strlen(str));
  TextOutA(hdc, x + 1, y - 1, str, (int)strlen(str));
  TextOutA(hdc, x - 1, y + 1, str, (int)strlen(str));
  TextOutA(hdc, x + 1, y + 1, str, (int)strlen(str));
  TextOutA(hdc, x, y - 1, str, (int)strlen(str));
  TextOutA(hdc, x + 1, y, str, (int)strlen(str));
  TextOutA(hdc, x, y + 1, str, (int)strlen(str));
  TextOutA(hdc, x + 1, y, str, (int)strlen(str));
  SetTextColor(hdc, 0);
  TextOutA(hdc, x, y, str, (int)strlen(str));
}

void DrawImage(const HWND hWnd, const HDC hdc)
{
  if (pulse->imageReady)
  {
    if (bmWidth != clientWidth || bmHeight != clientHeight)
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
      bmi.bmiHeader.biWidth = max(clientWidth, 1u);
      bmi.bmiHeader.biHeight = max(clientHeight, 1u);
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
        bmWidth = clientWidth;
        bmHeight = clientHeight;
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
        pixels[x + y * bmWidth] = pulse->getRenderImagePixel(x, y);
    }

    pulse->imageReady = false;
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
  std::vector<std::string> & text = *pulse->getCurrentScreenText();
  const int lineHeight = tm.tmHeight + 1;
  const int x = 3;
  int y = 3;

  for (int i = 0, sz = (int)text.size(); i < sz; i++)
  {
    OutlinePrint(hdc, x, y, text[i].c_str());
    y += lineHeight;
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

  RECT clientRect;
  if (GetClientRect(hWnd, &clientRect))
  {
    const unsigned int width = clientRect.right;
    const unsigned int height = clientRect.bottom;
    if (width && height)
    {
      pulse->onResize(width, height);
      clientWidth = width;
      clientHeight = height;
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

void OnKeyMessage(const int Key, const bool isDown)
{
  switch (Key)
  {
  case VK_LEFT:     pulse->onKeyEvent(KEY_LEFT, isDown);    break;
  case VK_RIGHT:    pulse->onKeyEvent(KEY_RIGHT, isDown);   break;
  case VK_UP:       pulse->onKeyEvent(KEY_UP, isDown);      break;
  case VK_DOWN:     pulse->onKeyEvent(KEY_DOWN, isDown);    break;
  case 'W':         pulse->onKeyEvent(KEY_W, isDown);       break;
  case 'S':         pulse->onKeyEvent(KEY_S, isDown);       break;
  case 'A':         pulse->onKeyEvent(KEY_A, isDown);       break;
  case 'D':         pulse->onKeyEvent(KEY_D, isDown);       break;
  case ' ':         pulse->onKeyEvent(KEY_SPACE, isDown);   break;
  case VK_CONTROL:  pulse->onKeyEvent(KEY_CONTROL, isDown); break;
  case VK_F2:       pulse->onKeyEvent(KEY_F2, isDown);      break;
  case '1':         pulse->onKeyEvent(KEY_1, isDown);       break;
  case '2':         pulse->onKeyEvent(KEY_2, isDown);       break;
  case '3':         pulse->onKeyEvent(KEY_3, isDown);       break;
  case '4':         pulse->onKeyEvent(KEY_4, isDown);       break;
  case '5':         pulse->onKeyEvent(KEY_5, isDown);       break;
  case '6':         pulse->onKeyEvent(KEY_6, isDown);       break;
  case '7':         pulse->onKeyEvent(KEY_7, isDown);       break;
  case '8':         pulse->onKeyEvent(KEY_8, isDown);       break;
  case '9':         pulse->onKeyEvent(KEY_9, isDown);       break;
  case VK_ESCAPE:   pulse->onKeyEvent(KEY_ESCAPE, isDown);  break;
  case 'Y':         pulse->onKeyEvent(KEY_Y, isDown);       break;
  case 'N':         pulse->onKeyEvent(KEY_N, isDown);       break;
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
    OnKeyMessage(int(wParam), true);
    break;
  case WM_KEYUP:
    OnKeyMessage(int(wParam), false);
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

  if (hWnd)
  {
    font = CreateFontA(12, -4, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET,
      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Aria");

    plint = new WindowsPlatformInterface(hWnd);
    pulse = new Pulse(plint);

    RECT crect;
    GetClientRect(hWnd, &crect);
    SetWindowPos(hWnd, 0, 0, 0, 640 + (640 - crect.right), 480 + (480 - crect.bottom), SWP_NOZORDER | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);

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

      pulse->exec();
    }

    if (memDC)
      DeleteDC(memDC);

    if (bitmap)
      DeleteObject(bitmap);

    if (font)
      DeleteObject(font);

    delete pulse;
    delete plint;

    return (int)msg.wParam;

  }
  else
    return FALSE;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

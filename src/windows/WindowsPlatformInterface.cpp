#include <windows.h>
#include "WindowsPlatformInterface.h"


WindowsPlatformInterface::WindowsPlatformInterface(HWND hWnd)
{
  this->hWnd = hWnd;
}


WindowsPlatformInterface::~WindowsPlatformInterface()
{
}

std::string WindowsPlatformInterface::getExePath()
{
  char exeFullName[MAX_PATH + 1];
  char exeFullPath[MAX_PATH + 1];
  char * lpExeName;
  GetModuleFileNameA(NULL, exeFullName, MAX_PATH + 1);
  GetFullPathNameA(exeFullName, MAX_PATH + 1, exeFullPath, &lpExeName);
  *lpExeName = '\0';

  return exeFullPath;
}

uint64_t WindowsPlatformInterface::getPerformanceCounter()
{
  LARGE_INTEGER counter = { 0, 0 };
  QueryPerformanceCounter(&counter);

  return counter.QuadPart;
}

uint64_t WindowsPlatformInterface::getPerformanceFrequency()
{
  LARGE_INTEGER perfFreq = { 0, 0 };
  QueryPerformanceFrequency(&perfFreq);
  
  return perfFreq.QuadPart;
}

uint64_t WindowsPlatformInterface::getSystemTime()
{
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);

  return uint64_t(ft.dwHighDateTime) << 32 | uint64_t(ft.dwLowDateTime);
}

void WindowsPlatformInterface::getMainWindowClientSize(unsigned int * width, unsigned int * const height)
{
  RECT rect = { 0, 0, 0, 0 };
  GetClientRect(hWnd, &rect);
  *width = rect.right;
  *height = rect.bottom;
}

void WindowsPlatformInterface::invalidateMainWindow()
{
  InvalidateRect(hWnd, NULL, false);
}

void WindowsPlatformInterface::sleep(unsigned int ms)
{
  Sleep(ms);
}


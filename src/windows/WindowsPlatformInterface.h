#pragma once
#include "..\common\BasePlatformInterface.h"

class WindowsPlatformInterface : public BasePlatformInterface
{
private:
  HWND hWnd;
public:
  WindowsPlatformInterface(HWND hWnd);
  ~WindowsPlatformInterface();

  std::string getExePath();
  uint64_t getPerformanceCounter();
  uint64_t getPerformanceFrequency();
  uint64_t getSystemTime();
  void getMainWindowClientSize(unsigned int * const width, unsigned int * const height);
  void invalidateMainWindow();
  void sleep(unsigned int timeMs);
};


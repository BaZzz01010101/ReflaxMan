#pragma once
#include "../common/BasePlatformInterface.h"

class LinuxPlatformInterface : public BasePlatformInterface
{
private:
  Display * dsp;
  Window win;

public:
  LinuxPlatformInterface(Display * dsp, Window win);
  ~LinuxPlatformInterface();

  std::string getExePath();
  uint64_t getPerformanceCounter();
  uint64_t getPerformanceFrequency();
  uint64_t getSystemTime();
  void getMainWindowClientSize(unsigned int * const width, unsigned int * const height);
  void invalidateMainWindow();
  void sleep(unsigned int timeMs);
};


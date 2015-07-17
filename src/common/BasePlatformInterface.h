#pragma once
#include <string>
#include <stdint.h>

class BasePlatformInterface
{
private:

public:
  BasePlatformInterface();
  virtual ~BasePlatformInterface();

  virtual std::string getExePath() = 0;
  virtual uint64_t getPerformanceCounter() = 0;
  virtual uint64_t getPerformanceFrequency() = 0;
  virtual uint64_t getSystemTime() = 0;
  virtual void getMainWindowClientSize(unsigned int * const width, unsigned int * const height) = 0;
  virtual void invalidateMainWindow() = 0;
  virtual void sleep(unsigned int ms) = 0;
};


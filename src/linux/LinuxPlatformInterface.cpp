#include <X11/Xlib.h>
#include <X11/X.h>
#include <limits.h>
#include <vector>
#include <unistd.h>
#include <string.h>

#include "LinuxPlatformInterface.h"


LinuxPlatformInterface::LinuxPlatformInterface(Display * dsp, Window win)
{
  this->dsp = dsp;
  this->win = win;
}


LinuxPlatformInterface::~LinuxPlatformInterface()
{
}

std::string LinuxPlatformInterface::getExePath()
{
    std::string exeFullPath = "./";
    std::vector<char> buffer(PATH_MAX + 1);
    char * pathStr = &buffer.front();
    int pathLen = readlink("/proc/self/exe", pathStr, PATH_MAX);

    if(pathLen > 0)
    {
      pathStr[pathLen] = '\0';
      char * exeNamePtr = strrchr(pathStr, '/');
      if(exeNamePtr)
      {
        *++exeNamePtr = '\0';
        exeFullPath = std::string(pathStr);
      }
    }

  return exeFullPath;
}

uint64_t LinuxPlatformInterface::getPerformanceCounter()
{
  timespec ts = { 0, 0 };
  clock_gettime(CLOCK_MONOTONIC, &ts);

  return uint64_t(ts.tv_sec) * 1000000000 + uint64_t(ts.tv_nsec);
}

uint64_t LinuxPlatformInterface::getPerformanceFrequency()
{
  return 1000000000;
}

uint64_t LinuxPlatformInterface::getSystemTime()
{
  timespec ts = { 0, 0 };
  clock_gettime(CLOCK_REALTIME, &ts);

  return (uint64_t)(ts.tv_sec) * 1000000000 + (uint64_t)(ts.tv_nsec);
}

void LinuxPlatformInterface::getMainWindowClientSize(unsigned int * width, unsigned int * const height)
{
  Window root;
  int x, y;
  unsigned int border, depth;
  XGetGeometry(dsp, win, &root, &x, &y, width, height, &border, &depth);
}

void LinuxPlatformInterface::invalidateMainWindow()
{
  XClearArea(dsp, win, 0, 0, 0, 0, true);
}

void LinuxPlatformInterface::sleep(unsigned int ms)
{
  usleep(ms * 1000);
}


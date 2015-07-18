#pragma once
#include <string>
#include "BasePlatformInterface.h"
#include "Render.h"

enum PULSE_STATE
{
  stInit,
  stCameraControl,
  stScreenshotResolutionSelection,
  stScreenshotSamplingSelection,
  stScreenshotRenderBegin,
  stScreenshotRenderProceed,
  stScreenshotRenderEnd,
  stScreenshotRenderSave,
  stScreenshotRenderCancelRequested
};

enum KEY_CODE
{
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_0,
  KEY_F2,
  KEY_Y,
  KEY_N,
  KEY_W,
  KEY_S,
  KEY_A,
  KEY_D,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_UP,
  KEY_DOWN,
  KEY_SPACE,
  KEY_CONTROL,
  KEY_ESCAPE,
};

struct Resolution
{
  int w;
  int h;
  const char * tip;
};

struct SsRate
{
  int rate;
  const char * tip;
};

class Pulse
{
private:
  BasePlatformInterface * plint;
  std::string exeFullPath;
  Render render;
  uint64_t perfFreq;
  PULSE_STATE state;
  unsigned int controlFlags;
  float frameTimeAccumulator;
  float frameTime;
  unsigned int renderChunkInPixels;
  std::string scrnshotFileName;
  unsigned int scrnshotWidth;
  unsigned int scrnshotHeight;
  int scrnshotSamples;
  float scrnshotProgress;
  uint64_t scrnshotStartTicks;
  bool quitMessage;

  std::string format(const char* format, ...);
  void setState(PULSE_STATE state);
  void proceedControl();
  void renderImage();
  void scrnshotRenderBegin();
  bool screenshotRenderProceed();
  void screenshotRenderSave();
  void screenshotRenderEnd();

public:
  bool imageReady;

  Pulse(BasePlatformInterface * platform);
  ~Pulse();

  void exec();
  std::vector<std::string> * getCurrentScreenText();
  void onKeyEvent(KEY_CODE key, bool isPressed);
  void onResize(unsigned int width, unsigned int height);
  uint32_t getRenderImagePixel(unsigned int x, unsigned int y);
};


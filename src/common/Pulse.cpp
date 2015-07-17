#include "trace_math.h"
#include "Pulse.h"
#include "defaults.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

static Resolution res[9] =
{
  { 640, 480, "(4:3)", },
  { 800, 600, "(4:3)", },
  { 1024, 768, "(4:3)", },
  { 1280, 960, "(4:3)", },
  { 1280, 800, "(16:10)", },
  { 1680, 1050, "(16:10)", },
  { 1920, 1200, "(16:10)", },
  { 1280, 720, "(HD)", },
  { 1920, 1080, "(Full HD)", },
};

static SsRate ss[9] =
{
  { 1, "(fast but rough)" },
  { 2, "" },
  { 4, "" },
  { 8, "" },
  { 16, "" },
  { 32, "" },
  { 64, "" },
  { 128, "" },
  { 256, "(slow but smooth)" },
};

Pulse::Pulse(BasePlatformInterface * platformInterface) :
  exeFullPath(platformInterface->getExePath().c_str()),
  render(exeFullPath.c_str())
{
  plint = platformInterface;
  perfFreq = plint->getPerformanceFrequency();
  setState(stCameraControl);
  controlFlags = 0;
  imageReady = false;
  frameTimeAccumulator = 0.0f;
  frameTime = 0.0f;
  renderChunkInPixels = 1;
  scrnshotFileName;
  scrnshotWidth = 0;
  scrnshotHeight = 0;
  scrnshotSamples = 0;
  scrnshotProgress = 0;
  scrnshotStartTicks = 0;
  quitMessage = false;
}


Pulse::~Pulse()
{
}

std::string Pulse::format(const char* format, ...)
{
  static std::vector<char> buf(1024);
  va_list args;
  va_start(args, format);
  size_t str_size = 1 + vsnprintf(0, 0, format, args);
  if (str_size > buf.size())
    buf.resize(str_size);
  char * buffer = &buf.front();
  vsnprintf(buffer, str_size, format, args);
  va_end(args);
  
  return buffer;
}

void Pulse::setState(PULSE_STATE state)
{
  this->state = state;
  plint->invalidateMainWindow();
}

void Pulse::proceedControl()
{
  const uint64_t counter = plint->getPerformanceCounter();

  if (counter)
  {
    static uint64_t prevCounter = 0;
    if (prevCounter)
    {
      float timePassedSec = float(counter - prevCounter) / perfFreq;
      render.camera.proceedControl(controlFlags, timePassedSec);
    }
    prevCounter = counter;
  }
}

void Pulse::renderImage()
{
  if (!imageReady)
  {
    static int motionDynSamples = Default::motionMinSamples;
    static int prevSamples = 0;
    const bool inMotion = controlFlags || render.camera.inMotion();

    if (!render.inProgress || (inMotion && motionDynSamples != prevSamples))
    {
      if (inMotion)
      {
        if (frameTime > Default::maxMotionFrameTime)
          motionDynSamples = max(motionDynSamples - 1, Default::motionMaxSamples);
        else if (frameTime < Default::minMotionFrameTime)
          motionDynSamples = min(motionDynSamples + 1, Default::motionMinSamples);
      }

      static bool prevInMotion = false;
      const int reflNum = (inMotion || prevInMotion) ? Default::motionReflections : Default::staticReflections;
      const int sampleNum = (inMotion || prevInMotion) ? motionDynSamples : Default::staticSamples;

      render.renderBegin(reflNum, sampleNum, !(inMotion || prevInMotion));
      renderChunkInPixels = 1;
      prevSamples = sampleNum;
      prevInMotion = inMotion;
    }

    const uint64_t cnt0 = plint->getPerformanceCounter();
    const bool isComplete = !render.renderNext(renderChunkInPixels);
    const uint64_t cnt1 = plint->getPerformanceCounter();

    float timePassed = float(cnt1 - cnt0) / perfFreq;
    frameTimeAccumulator += timePassed;
    const float minChunkRenderTime = 0.005f;
    const float maxChunkRenderTime = 0.020f;

    if (!isComplete)
    {
      if (timePassed < minChunkRenderTime)
        renderChunkInPixels = min(renderChunkInPixels * 2, render.imageHeight * render.imageWidth);
      else if (timePassed > maxChunkRenderTime)
        renderChunkInPixels = max(renderChunkInPixels / 2, 1u);
    }
    else
    {
      frameTime = frameTimeAccumulator;
      frameTimeAccumulator = 0;
      imageReady = true;
      plint->invalidateMainWindow();
    }
  }
}

void Pulse::scrnshotRenderBegin()
{
  const int bufSize = 256;
  char name[bufSize];

  uint64_t systemTime = plint->getSystemTime();
  unsigned int hiPart = (unsigned int)(systemTime >> 32);
  unsigned int loPart = (unsigned int)(systemTime & 0xFFFFFFFF);

#ifdef _MSC_VER
  _snprintf(name, bufSize, "scrnshoot_%08X%08X.bmp", hiPart, loPart);
#else
  snprintf(name, bufSize, "scrnshoot_%08X%08X.bmp", hiPart, loPart);
#endif

  scrnshotFileName = exeFullPath;
  scrnshotFileName += name;

  render.setImageSize(scrnshotWidth, scrnshotHeight);
  scrnshotStartTicks = plint->getPerformanceCounter();
  render.renderBegin(Default::scrnshotRefections, scrnshotSamples, false);
  renderChunkInPixels = 1;
}

bool Pulse::screenshotRenderProceed()
{
  const uint64_t cnt0 = plint->getPerformanceCounter();
  const float minChunkRenderTime = 0.010f;
  const float maxChunkRenderTime = 0.100f;

  if (render.renderNext(renderChunkInPixels))
  {
    const uint64_t cnt1 = plint->getPerformanceCounter();
    float timePassed = float(cnt1 - cnt0) / perfFreq;

    if (timePassed < minChunkRenderTime)
      renderChunkInPixels = min(renderChunkInPixels * 2, render.imageHeight * render.imageWidth);
    else if (timePassed > maxChunkRenderTime)
      renderChunkInPixels = max(renderChunkInPixels / 2, 1u);

    scrnshotProgress = render.getRenderProgress();
    plint->invalidateMainWindow();
    return false;
  }
  else
    return true;
}

void Pulse::screenshotRenderSave()
{
  Texture imageTexture(render.imageWidth, render.imageHeight);
  render.copyImage(imageTexture);
  imageTexture.saveToFile(scrnshotFileName.c_str());
}

void Pulse::screenshotRenderEnd()
{
  scrnshotProgress = 0;
  scrnshotWidth = 0;
  scrnshotHeight = 0;
  scrnshotSamples = 0;

  unsigned int clWidth, clHeight;
  plint->getMainWindowClientSize(&clWidth, &clHeight);

  if (clWidth && clHeight)
    render.setImageSize(clWidth, clHeight);
  else
    render.setImageSize(1, 1);

}

void Pulse::exec()
{
  switch (state)
  {
  case stCameraControl:
    proceedControl();
    renderImage();
    break;
  case stScreenshotResolutionSelection:
  case stScreenshotSamplingSelection:
    plint->sleep(10);
    break;
  case stScreenshotRenderBegin:
    scrnshotRenderBegin();
    setState(stScreenshotRenderProceed);
    break;
  case stScreenshotRenderProceed:
  case stScreenshotRenderCancelRequested:
    if (screenshotRenderProceed())
      setState(stScreenshotRenderSave);
    break;
  case stScreenshotRenderSave:
    screenshotRenderSave();
    setState(stScreenshotRenderEnd);
    break;
  case stScreenshotRenderEnd:
    screenshotRenderEnd();
    setState(stCameraControl);
    break;
  default:
    assert(0);
  }
}

std::vector<std::string> * Pulse::getCurrentScreenText()
{
  static std::vector<std::string> screenText;
  screenText.clear();

  switch (state)
  {
  case stCameraControl:
    if (frameTime < 10)
      screenText.push_back(format("Frame time: %.0f ms", frameTime * 1000));
    else
      screenText.push_back(format("Frame time: %.03f s", frameTime));
    screenText.push_back(format("Blended frames : %i", render.additiveCounter));
    screenText.push_back("");
    screenText.push_back("WSAD : moving");
    screenText.push_back("Cursor keys: turning");
    screenText.push_back("Space : ascenting");
    screenText.push_back("Ctrl : descenting");
    screenText.push_back("");
    screenText.push_back("F2 : save screenshot");
    break;
  case stScreenshotResolutionSelection:
    screenText.push_back("Select screenshot resolution (keys 1-9)");
    screenText.push_back("");

    for (int i = 0; i < sizeof(res) / sizeof(*res); i++)
      screenText.push_back(format("%i : %ix%i %s", i + 1, res[i].w, res[i].h, res[i].tip));

    screenText.push_back("");
    screenText.push_back("ESC : cancel");
    break;
  case stScreenshotSamplingSelection:
    screenText.push_back("Select supersampling rate (keys 1-9)");
    screenText.push_back("");

    for (int i = 0; i < sizeof(ss) / sizeof(*ss); i++)
      screenText.push_back(format("%i : %ix%i %s", i + 1, ss[i].rate, ss[i].rate, ss[i].tip));

    screenText.push_back("");
    screenText.push_back("ESC : cancel");
    break;
  case stScreenshotRenderProceed:
  case stScreenshotRenderCancelRequested:
    screenText.push_back("Saving screenshot:");
    screenText.push_back(scrnshotFileName.c_str());
    screenText.push_back(format("Resolution: %ix%i", scrnshotWidth, scrnshotHeight));
    screenText.push_back(format("SSAA: %ix%i", scrnshotSamples, scrnshotSamples));
    screenText.push_back("");
    screenText.push_back(format("Progress: %.2f %%", scrnshotProgress));
    if (scrnshotProgress > VERY_SMALL_NUMBER)
    {
      int timePassed = int((plint->getPerformanceCounter() - scrnshotStartTicks) / perfFreq);
      int timeLeft = int(timePassed * 100 / scrnshotProgress) - timePassed;

      int hr = timeLeft / 3600;
      timeLeft = timeLeft % 3600;
      int min = timeLeft / 60;
      timeLeft = timeLeft % 60;
      int sec = timeLeft;
  
      screenText.push_back(format("Estimated time left: %i h %02i m %02i s", hr, min, sec));
      screenText.push_back("");

      if (state == stScreenshotRenderCancelRequested)
        screenText.push_back("Do you want to cancel ? ( Y / N ) ");
      else
        screenText.push_back("Press ESC to cancel");
    }
    break;
  }

  return &screenText;
}

void Pulse::onKeyEvent(KEY_CODE key, bool isPressed)
{
  if (state == stCameraControl)
  {
    unsigned int mask = 0;

    switch (key)
    {
      case KEY_LEFT:    mask = turnLeftMask; break;
      case KEY_RIGHT:   mask = turnRightMask; break;
      case KEY_UP:      mask = turnDownMask; break;
      case KEY_DOWN:    mask = turnUpMask; break;
      case KEY_W:       mask = shiftForwardMask; break;
      case KEY_S:       mask = shiftBackMask; break;
      case KEY_A:       mask = shiftLeftMask; break;
      case KEY_D:       mask = shiftRightMask; break;
      case KEY_SPACE:   mask = shiftUpMask; break;
      case KEY_CONTROL: mask = shiftDownMask; break;
      case KEY_F2:      
        if(isPressed) 
          setState(stScreenshotResolutionSelection); 
        break;
    }
    controlFlags = isPressed ? controlFlags | mask : controlFlags & ~mask;

  }
  else if (state == stScreenshotResolutionSelection && isPressed)
  {
    int index = -1;

    switch (key)
    {
      case KEY_1: index = 0; break;
      case KEY_2: index = 1; break;
      case KEY_3: index = 2; break;
      case KEY_4: index = 3; break;
      case KEY_5: index = 4; break;
      case KEY_6: index = 5; break;
      case KEY_7: index = 6; break;
      case KEY_8: index = 7; break;
      case KEY_9: index = 8; break;
      case KEY_ESCAPE: setState(stCameraControl); break;
    }

    if (index >= 0)
    {
      scrnshotWidth = res[index].w;
      scrnshotHeight = res[index].h;
      setState(stScreenshotSamplingSelection);
    }
  }
  else if (state == stScreenshotSamplingSelection && isPressed)
  {
    int index = -1;

    switch (key)
    {
      case KEY_1: index = 0; break;
      case KEY_2: index = 1; break;
      case KEY_3: index = 2; break;
      case KEY_4: index = 3; break;
      case KEY_5: index = 4; break;
      case KEY_6: index = 5; break;
      case KEY_7: index = 6; break;
      case KEY_8: index = 7; break;
      case KEY_9: index = 8; break;
      case KEY_ESCAPE: setState(stCameraControl); break;
    }

    if (index >= 0)
    {
      scrnshotSamples = ss[index].rate;
      setState(stScreenshotRenderBegin);
    }
  }
  else if (state == stScreenshotRenderProceed && isPressed)
  {
    if (key == KEY_ESCAPE)
      setState(stScreenshotRenderCancelRequested);
  }
  else if (state == stScreenshotRenderCancelRequested && isPressed)
  {
    switch (key)
    {
      case KEY_Y: setState(stCameraControl); break;
      case KEY_N: setState(stScreenshotRenderProceed); break;
    }
  }
}

void Pulse::onResize(unsigned int width, unsigned int height)
{
  if (state == stCameraControl)
  {
    render.setImageSize(width, height);
  }
}

uint32_t Pulse::getRenderImagePixel(unsigned int x, unsigned int y)
{
  return render.imagePixel(x, y).argb();
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

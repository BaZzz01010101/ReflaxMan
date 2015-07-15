#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <cstdarg>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/kd.h>

#include "../common/trace_math.h"
#include "../common/defaults.h"
#include "../common/Render.h"

Render * render = NULL;
Display * dsp = NULL;
int scr = 0;
Window win = 0;
Visual * vis = NULL;
GC gc = 0;
unsigned int clientWidth = 0;
unsigned int clientHeight = 0;
int fontAscent = 0;
int lineHeight = 0;
Pixmap pixmap = 0;
XImage * image = NULL;
uint32_t * pixels = NULL;
bool imageReady = false;
float frameChunkTime = 0.0f;
float frameTime = 0.0f;
std::string exeFullPath;
std::string scrnshotFileName;
bool scrnshotSaving = false;
float scrnshotProgress = -1;
timespec scrnshotStartTs = { 0, 0 };
bool scrnshotCancelRequested = false;
bool scrnshotCancelConfirmed = false;
bool quitMessage = false;
int controlFlags = 0;

void print(Drawable dr, const int x, const int y, const char* format, ...)
{
  static std::vector<char> buf(1024);
  va_list args;
  va_start(args, format);
  const size_t str_size = 1 + vsnprintf(0, 0, format, args);
  if (str_size > buf.size())
    buf.resize(str_size);
  char * buffer = &buf.front();
  vsnprintf(buffer, str_size, format, args);
  va_end(args);

  XSetForeground(dsp, gc, 0xAAAAAA);
  XDrawString(dsp, dr, gc, x - 1, y - 1, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x, y - 1, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x + 1, y - 1, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x - 1, y, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x + 1, y, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x - 1, y + 1, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x, y + 1, buffer, (int)strlen(buffer));
  XDrawString(dsp, dr, gc, x + 1, y + 1, buffer, (int)strlen(buffer));

  XSetForeground(dsp, gc, 0);
  XDrawString(dsp, dr, gc, x, y, buffer, (int)strlen(buffer));
}

int clock_timediff_ms(const timespec ts1, const timespec ts0)
{
  timespec diff;
  diff.tv_nsec = ts1.tv_nsec - ts0.tv_nsec;
  diff.tv_sec = ts1.tv_sec - ts0.tv_sec;

  if(ts1.tv_nsec < ts0.tv_nsec)
  {
    diff.tv_sec--;
    diff.tv_nsec = 1000000000 + diff.tv_nsec;
  }

  return diff.tv_sec * 1000 + diff.tv_nsec / 1000000;
}

int clock_timediff_mcs(const timespec ts1, const timespec ts0)
{
  timespec diff;
  diff.tv_nsec = ts1.tv_nsec - ts0.tv_nsec;
  diff.tv_sec = ts1.tv_sec - ts0.tv_sec;

  if(ts1.tv_nsec < ts0.tv_nsec)
  {
    diff.tv_sec--;
    diff.tv_nsec = 1000000000 + diff.tv_nsec;
  }

  return diff.tv_sec * 1000000 + diff.tv_nsec / 1000;
}

void ProceedControl()
{
  static timespec prevTs;
  static const bool initSuccess = !clock_gettime(CLOCK_MONOTONIC, &prevTs);
  timespec ts;

  if(initSuccess && !clock_gettime(CLOCK_MONOTONIC, &ts))
  {
    int ms = clock_timediff_ms(ts, prevTs);

    if(ms > 20)
    {
      render->camera.proceedControl(controlFlags, ms);
      prevTs = ts;
    }
  }
}

void DrawScreenshotStats(Drawable dr)
{
  const int x = 3;
  int y = fontAscent + 3;

  print(dr, x, y, "Saving screenshot:");

  y += lineHeight;
  print(dr, x, y, scrnshotFileName.c_str());

  y += lineHeight;
  print(dr, x, y, "Resolution: %ix%i", Default::scrnshotWidth, Default::scrnshotHeight);

  y += lineHeight;
  print(dr, x, y, "SSAA: %ix", Default::scrnshotSamples);

  y += lineHeight * 2;
  print(dr, x, y, "Progress: %.2f %%", scrnshotProgress);

  if (scrnshotProgress > VERY_SMALL_NUMBER)
  {
    timespec ts;
    if(!clock_gettime(CLOCK_MONOTONIC, &ts))
    {
      const int timePassedMs = clock_timediff_ms(ts, scrnshotStartTs);
      int timeLeftMs = int(timePassedMs * 100 / scrnshotProgress) - timePassedMs;

      int h = timeLeftMs / 3600000;
      timeLeftMs = timeLeftMs % 3600000;
      int m = timeLeftMs / 60000;
      timeLeftMs = timeLeftMs % 60000;
      int s = timeLeftMs / 1000;
      y += lineHeight;
      print(dr, x, y, "Estimated time left: %i h %02i m %02i s", h, m, s);
    }
  }

  if (scrnshotCancelRequested)
  {
    y += lineHeight * 2;
    print(dr, x, y, "Do you want to cancel ? ( Y / N ) ");
  }
  else
  {
    y += lineHeight * 2;
    print(dr, x, y, "Press ESC to cancel");
  }
}

void DrawSceneStats(Drawable dr)
{
  const int x = 3;
  int y = fontAscent + 3;

  if (frameTime < 10)
    print(dr, x, y, "Frame time: %.0f ms", frameTime * 1000);
  else
    print(dr, x, y, "Frame time: %.03f s", frameTime);

  y += lineHeight;
  print(dr, x, y, "Blended frames : %i", render->additiveCounter);

  y += lineHeight * 2;
  print(dr, x, y, "WSAD : moving");

  y += lineHeight;
  print(dr, x, y, "Cursor : turning");

  y += lineHeight;
  print(dr, x, y, "Space : ascenting");

  y += lineHeight;
  print(dr, x, y, "Ctrl : descenting");

  y += lineHeight * 2;
  print(dr, x, y, "F2 : save screenshot");
}

void DrawImage(Drawable dr)
{
  if (imageReady)
  {
    const int width = render->imageWidth;
    const int height = render->imageHeight;

    if (!image || image->width != width || image->height != height)
    {
      if(image)
      {
        XDestroyImage(image);
        image = NULL;
        pixels = NULL;
      }
      else if(pixels)
      {
        free(pixels);
        pixels = NULL;
      }

      pixels = (uint32_t*)malloc(width * height * 4);

      if(pixels)
        image = XCreateImage(dsp, vis, 24, ZPixmap, 0, (char*)pixels, width, height, 32, 0);

      assert(pixels);
      assert(image);
    }

    if(pixels)
    {
      for (int y = 0; y < height; ++y)
      for (int x = 0; x < width; ++x)
        pixels[x + (height - y - 1) * width] = render->imagePixel(x, y).argb();
    }

    imageReady = false;
  }

  if (image)
  {
    const int xGap = clientWidth - image->width;
    const int yGap = clientHeight - image->height;

    if(xGap > 0)
    {
      XFillRectangle(dsp, dr, gc, 0, 0, xGap / 2, clientHeight);
      XFillRectangle(dsp, dr, gc, clientWidth - xGap / 2, 0, xGap / 2, clientHeight);
    }

    if(yGap > 0)
    {
      XFillRectangle(dsp, dr, gc, 0, 0, clientWidth, yGap / 2);
      XFillRectangle(dsp, dr, gc, 0, clientHeight - yGap / 2, clientWidth, yGap / 2);
    }
    XPutImage(dsp, dr, gc, image, 0, 0, xGap / 2, yGap / 2, image->width, image->height);
  }
}

void ScrnshotSavingPulse()
{
  if (scrnshotProgress < 0)
  {
    timespec ts;

    if(!clock_gettime(CLOCK_REALTIME, &ts))
    {
      unsigned int highTime = ts.tv_sec / 1000000;
      unsigned int lowTime = (ts.tv_sec % 1000000 * 1000) + ts.tv_nsec / 1000000;
      const int bufSize = 256;
      char name[bufSize];
      snprintf(name, bufSize, "scrnshoot_%08X%08X.bmp", highTime, lowTime);
      scrnshotFileName = exeFullPath + name;

      render->setImageSize(Default::scrnshotWidth, Default::scrnshotHeight);

      if(!clock_gettime(CLOCK_MONOTONIC, &scrnshotStartTs))
        render->renderBegin(Default::scrnshotRefections, Default::scrnshotSamples, false);
    }
  }

  if (!scrnshotCancelConfirmed && render->renderNext(1))
  {
    scrnshotProgress = render->getRenderProgress();
    XClearArea(dsp, win, 0, 0, 0, 0, true);
  }
  else
  {
    if (!scrnshotCancelConfirmed)
    {
      Texture imageTexture(render->imageWidth, render->imageHeight);
      render->copyImage(imageTexture);
      imageTexture.saveToFile(scrnshotFileName.c_str());
    }

    scrnshotProgress = -1;
    scrnshotSaving = false;
    scrnshotCancelConfirmed = false;

    if (clientWidth && clientHeight)
      render->setImageSize(clientWidth, clientHeight);
    else
      render->setImageSize(1, 1);
  }
}

void ImageRenderPulse()
{
  static int motionDynSamples = Default::motionMinSamples;
  static int prevSamples = 0;
  static bool prevInMotion = false;
  const bool inMotion = controlFlags || render->camera.inMotion();

  if (!imageReady)
  {
    if (!render->inProgress || (inMotion && motionDynSamples != prevSamples))
    {
      if (inMotion)
      {
        if (frameTime > Default::maxMotionFrameTime)
          motionDynSamples = max(motionDynSamples - 1, Default::motionMaxSamples);
        else if (frameTime < Default::minMotionFrameTime)
          motionDynSamples = min(motionDynSamples + 1, Default::motionMinSamples);
      }

      const int reflNum = (inMotion || prevInMotion) ? Default::motionReflections : Default::staticReflections;
      const int sampleNum = (inMotion || prevInMotion) ? motionDynSamples : Default::staticSamples;

      render->renderBegin(reflNum, sampleNum, !(inMotion || prevInMotion));
      prevSamples = sampleNum;
      prevInMotion = inMotion;
    }

    timespec ts0, ts1;
    if(!clock_gettime(CLOCK_MONOTONIC, &ts0))
    {
      const int linesLeft = render->renderNext(1);

      if (!clock_gettime(CLOCK_MONOTONIC, &ts1))
        frameChunkTime += float(clock_timediff_mcs(ts1, ts0)) / 1000000.0f;

      if (!linesLeft)
      {
        frameTime = frameChunkTime;
        frameChunkTime = 0;
        imageReady = true;
        XClearArea(dsp, win, 0, 0, 0, 0, true);
        XFlush(dsp);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//                                 Events
// -----------------------------------------------------------------------------

void OnResize(XConfigureEvent & e)
{
  Window root;
  int x, y;
  unsigned int newClientWidth, newClientHeight, border, depth;
  XGetGeometry(dsp, win, &root, &x, &y, &newClientWidth, &newClientHeight, &border, &depth);

  if(!pixmap || newClientWidth != clientWidth || newClientHeight != clientHeight)
  {
    clientWidth = newClientWidth;
    clientHeight = newClientHeight;

    if(pixmap)
    {
      XFreePixmap(dsp, pixmap);
      pixmap = 0;
    }

    pixmap = XCreatePixmap(dsp, win, clientWidth, clientHeight, 24);
    XFillRectangle(dsp, pixmap, gc, 0, 0, clientWidth, clientHeight);
  }

  if (!scrnshotSaving)
    render->setImageSize(clientWidth, clientHeight);
}

void OnExpose(XExposeEvent & e)
{
  if(!e.count)
  {
    DrawImage(pixmap);

    if (scrnshotProgress >= 0)
      DrawScreenshotStats(pixmap);
    else
      DrawSceneStats(pixmap);

    XCopyArea(dsp, pixmap, win, gc, 0, 0, clientWidth, clientHeight, 0, 0);
    XFlush(dsp);
  }
}

void OnKeyPress(XKeyEvent & e)
{
  const int buf_size = 256;
  char buf[buf_size];
  KeySym ks;
  XLookupString(&e, buf, buf_size, &ks, NULL);

  switch(ks)
  {
    case XK_Left:
      controlFlags |= turnLeftMask;
      break;
    case XK_Right:
      controlFlags |= turnRightMask;
      break;
    case XK_Up:
      controlFlags |= turnDownMask;
      break;
    case XK_Down:
      controlFlags |= turnUpMask;
      break;
    case XK_W:
    case XK_w:
      controlFlags |= shiftForwardMask;
      break;
    case XK_S:
    case XK_s:
      controlFlags |= shiftBackMask;
      break;
    case XK_A:
    case XK_a:
      controlFlags |= shiftLeftMask;
      break;
    case XK_D:
    case XK_d:
      controlFlags |= shiftRightMask;
      break;
    case XK_space:
      controlFlags |= shiftUpMask;
      break;
    case XK_Control_L:
    case XK_Control_R:
      controlFlags |= shiftDownMask;
      break;
    case XK_F2:
      scrnshotSaving = true;
      break;
    case XK_Escape:
      if (scrnshotSaving)
        scrnshotCancelRequested = true;
      break;
    case XK_Y:
    case XK_y:
      if (scrnshotCancelRequested)
      {
        scrnshotCancelRequested = false;
        scrnshotCancelConfirmed = true;
      }
    case XK_N:
    case XK_n:
      if (scrnshotCancelRequested)
        scrnshotCancelRequested = false;
  }
}

void OnKeyRelease(XKeyEvent & e)
{
  const int buf_size = 256;
  char buf[buf_size];
  KeySym ks;
  XLookupString(&e, buf, buf_size, &ks, NULL);

  switch(ks)
  {
    case XK_Left:
      controlFlags &= ~turnLeftMask;
      break;
    case XK_Right:
      controlFlags &= ~turnRightMask;
      break;
    case XK_Up:
      controlFlags &= ~turnDownMask;
      break;
    case XK_Down:
      controlFlags &= ~turnUpMask;
      break;
    case XK_W:
    case XK_w:
      controlFlags &= ~shiftForwardMask;
      break;
    case XK_S:
    case XK_s:
      controlFlags &= ~shiftBackMask;
      break;
    case XK_A:
    case XK_a:
      controlFlags &= ~shiftLeftMask;
      break;
    case XK_D:
    case XK_d:
      controlFlags &= ~shiftRightMask;
      break;
    case XK_space:
      controlFlags &= ~shiftUpMask;
      break;
    case XK_Control_L:
    case XK_Control_R:
      controlFlags &= ~shiftDownMask;
      break;
  }
}

void ProcessEvent(XEvent & e)
{
  switch(e.type)
  {
  case Expose:
    OnExpose(e.xexpose);
    break;
  case KeyPress:
    OnKeyPress(e.xkey);
    break;
  case KeyRelease:
    OnKeyRelease(e.xkey);
    break;
  case MappingNotify:
    XRefreshKeyboardMapping(&e.xmapping);
    break;
  case ConfigureNotify:
    OnResize(e.xconfigure);
    break;
  }
}

int main()
{
  dsp = XOpenDisplay(0);

  if (dsp)
  {
    scr = XDefaultScreen(dsp);
    vis = DefaultVisual(dsp, scr);
    win = XCreateWindow(dsp, DefaultRootWindow(dsp), 0, 0, 640, 480, 1, 24, CopyFromParent, CopyFromParent, 0, 0);
    gc = DefaultGC(dsp, scr);

    XSelectInput(dsp, win, ExposureMask | KeyPressMask | KeyReleaseMask | MappingNotify | StructureNotifyMask);

    int direction, fontDescent;
    XCharStruct overall;
    XQueryTextExtents(dsp, XGContextFromGC(gc), "Jj", 2, &direction, &fontAscent, &fontDescent, &overall);
    lineHeight = fontAscent + fontDescent + 1;

    Atom wmDeleteMessage = XInternAtom(dsp, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(dsp, win, &wmDeleteMessage, 1);

    exeFullPath = "./";
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

    render = new Render(exeFullPath.c_str());

    XMapWindow(dsp, win);
    XFlush(dsp);

    XEvent e;

    while (!quitMessage)
    {
      while(XPending(dsp))
      {
        XNextEvent(dsp, &e);
        ProcessEvent(e);

        if (e.type == ClientMessage && e.xclient.data.l[0] == (int)wmDeleteMessage)
          quitMessage = true;
      }

      if (scrnshotSaving)
      {
        ScrnshotSavingPulse();
      }
      else
      {
        ProceedControl();
        ImageRenderPulse();
      }
    }
  }

  if(image)
    XDestroyImage(image);
  else if(pixels)
    free(pixels);

  if(pixmap)
    XFreePixmap(dsp, pixmap);

  delete render;

  return 0;
}

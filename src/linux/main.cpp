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
#include "../airly/airly.h"
#include "../airly/Scene.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

bool exitCmd = false;
Display * dsp = NULL;
int scr;
Window win;
XImage * image = NULL;
Visual * vis;
void * imageBuf = NULL;
char exePath[PATH_MAX + 1];
Scene * scene = NULL;
int controlFlags = 0;
const int turnLeftMask = 1 << 0;
const int turnRightMask = 1 << 1;
const int turnUpMask = 1 << 2;
const int turnDownMask = 1 << 3;
const int turnCwizeMask = 1 << 4;
const int turnCcwizeMask = 1 << 5;
const int shiftLeftMask = 1 << 6;
const int shiftRightMask = 1 << 7;
const int shiftUpMask = 1 << 8;
const int shiftDownMask = 1 << 9;
const int shiftForwardMask = 1 << 10;
const int shiftBackMask = 1 << 11;

Pixmap pm;
GC gc;
XGCValues gcval;

void print(int x, int y, const char * format, ...)
{

  char buffer[256];
  va_list arg;
  va_start(arg, format);
  vsprintf(buffer, format, arg);
  va_end(arg);
  XDrawString(dsp, win, DefaultGC(dsp, scr), x, y, buffer, strlen(buffer));
}

void proceedControls()
{
  if(controlFlags & turnLeftMask)
    scene->camera.rotate(Camera::Rotation::crLeft);
  if(controlFlags & turnRightMask)
    scene->camera.rotate(Camera::Rotation::crRight);
  if(controlFlags & turnUpMask)
    scene->camera.rotate(Camera::Rotation::crUp);
  if(controlFlags & turnDownMask)
    scene->camera.rotate(Camera::Rotation::crDown);
  if(controlFlags & turnCwizeMask)
    scene->camera.rotate(Camera::Rotation::crCwise);
  if(controlFlags & turnCcwizeMask)
    scene->camera.rotate(Camera::Rotation::crCcwise);
  if(controlFlags & shiftLeftMask)
    scene->camera.move(Camera::Movement::cmLeft);
  if(controlFlags & shiftRightMask)
    scene->camera.move(Camera::Movement::cmRight);
  if(controlFlags & shiftUpMask)
    scene->camera.move(Camera::Movement::cmUp);
  if(controlFlags & shiftDownMask)
    scene->camera.move(Camera::Movement::cmDown);
  if(controlFlags & shiftForwardMask )
    scene->camera.move(Camera::Movement::cmForward);
  if(controlFlags & shiftBackMask )
    scene->camera.move(Camera::Movement::cmBack);
}

void DrawScene()
{
  proceedControls();

  timespec ts0, ts1;
  bool ts0_success = !clock_gettime(CLOCK_MONOTONIC, &ts0);

  for(int x = 0; x < scene->screenWidth; x++)
  for(int y = 0; y < scene->screenHeight; y++)
  {
    ARGB argb = scene->tracePixel(x, scene->screenHeight - y, 1).argb();
    ((uint32_t*)(imageBuf))[x + y * scene->screenWidth] = argb;
  }

  XPutImage(dsp, win, gc, image, 0, 0, 0, 0, scene->screenWidth, scene->screenHeight);

  int direction, fontAscent, fontDescent;
  XCharStruct overall;
  XQueryTextExtents(dsp, XGContextFromGC(DefaultGC(dsp, scr)), "Jj", 2, &direction, &fontAscent, &fontDescent, &overall);
  int fontHeight = fontAscent + fontDescent + 2;
  int y = fontHeight;

  if(ts0_success && !clock_gettime(CLOCK_MONOTONIC, &ts1))
  {
    float frameTime = float(ts1.tv_sec) + float(ts1.tv_nsec) / 1000000000.0f - float(ts0.tv_sec) - float(ts0.tv_nsec) / 1000000000.0f;

    if (frameTime < 10)
      print(0, y, "frame time: %.0f ms", frameTime * 1000);
    else
      print(0, y, "frame time: %.03f s", frameTime);
    y += fontHeight;
  }

  print(0, y, "camera eye: [%.03f, %.03f, %.03f]", scene->camera.eye.x, scene->camera.eye.y, scene->camera.eye.z);

  y += fontHeight;
  Vector3 at = scene->camera.eye + scene->camera.view.getCol(2);
  print(0, y, "camera at: [%.03f, %.03f, %.03f]", at.x, at.y, at.z);

  y += fontHeight;
  Vector3 up = scene->camera.view.getCol(1);
  print(0, y, "camera up: [%.03f, %.03f, %.03f]", up.x, up.y, up.z);

  XClearArea(dsp, win, 0, 0, 0, 0, true);
  XFlush(dsp);
}

void OnKeyPress(XKeyEvent & e)
{
  const int buf_size = 256;
  char buf[buf_size];
  KeySym ks;
  XLookupString(&e, buf, buf_size, &ks, NULL);

  switch(ks)
  {
    case XK_F10:
      exitCmd = true;
      break;

    case XK_Left:
      controlFlags |= turnLeftMask;
      break;

    case XK_Right:
      controlFlags |= turnRightMask;
      break;

    case XK_Up:
      controlFlags |= turnUpMask;
      break;

    case XK_Down:
      controlFlags |= turnDownMask;
      break;

    case XK_Page_Up:
      controlFlags |= turnCwizeMask;
      break;

    case XK_Insert:
      controlFlags |= turnCcwizeMask;
      break;

    case XK_Delete:
      controlFlags |= shiftLeftMask;
      break;

    case XK_Page_Down:
      controlFlags |= shiftRightMask;
      break;

    case XK_Home:
      controlFlags |= shiftUpMask;
      break;

    case XK_End:
      controlFlags |= shiftDownMask;
      break;

    case XK_A:
    case XK_a:
      controlFlags |= shiftForwardMask;
      break;

    case XK_Z:
    case XK_z:
      controlFlags |= shiftBackMask;
      break;
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
      controlFlags &= ~turnUpMask;
      break;

    case XK_Down:
      controlFlags &= ~turnDownMask;
      break;

    case XK_Page_Up:
      controlFlags &= ~turnCwizeMask;
      break;

    case XK_Insert:
      controlFlags &= ~turnCcwizeMask;
      break;

    case XK_Delete:
      controlFlags &= ~shiftLeftMask;
      break;

    case XK_Page_Down:
      controlFlags &= ~shiftRightMask;
      break;

    case XK_Home:
      controlFlags &= ~shiftUpMask;
      break;

    case XK_End:
      controlFlags &= ~shiftDownMask;
      break;

    case XK_A:
    case XK_a:
      controlFlags &= ~shiftForwardMask;
      break;

    case XK_Z:
    case XK_z:
      controlFlags &= ~shiftBackMask;
      break;
  }
}

void OnResize(XConfigureEvent & e)
{
  if(image)
  {
    XDestroyImage(image);
    image = NULL;
    imageBuf = NULL;
  }

  if(imageBuf)
  {
    free(imageBuf);
    imageBuf = NULL;
  }

  imageBuf = malloc(e.height * e.width * 4);
  image = XCreateImage(dsp, vis, 24, ZPixmap, 0, (char*)imageBuf, e.width, e.height, 32, 0);
  assert(image);
  scene->screenWidth = e.width;
  scene->screenHeight = e.height;
}

int main()
{
  readlink("/proc/self/exe", exePath, PATH_MAX + 1);
  char * lastSlash = strrchr(exePath, '/');
  if(!lastSlash)
    return 1;

  *(lastSlash + 1) = '\0';

  dsp = XOpenDisplay(0);

  if ( dsp )
  {
    scene = new Scene(exePath);
    scr = XDefaultScreen(dsp);
    vis = DefaultVisual(dsp, scr);
    win = XCreateWindow(dsp, DefaultRootWindow(dsp), 0, 0, 640, 480, 1, CopyFromParent, CopyFromParent, CopyFromParent, 0, 0);
    XSelectInput(dsp, win, ExposureMask | KeyPressMask | KeyReleaseMask | MappingNotify | StructureNotifyMask);
    XMapWindow(dsp, win);
    XFlush(dsp);

    gcval.function = GXcopy;
    gcval.plane_mask = AllPlanes;
    gcval.foreground = BlackPixel(dsp, scr);
    gcval.background = WhitePixel(dsp, scr);
    gc = XCreateGC(dsp, win, GCFunction | GCPlaneMask | GCForeground | GCBackground, &gcval);

    XEvent e;

    while (!exitCmd)
    {
      XNextEvent(dsp, &e);

      switch(e.type)
      {
      case Expose:
        if(!e.xexpose.count)
          DrawScene();
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
    XFreePixmap(dsp, pm);
  }

  if(image)
  {
    XDestroyImage(image);
    image = NULL;
  }

  if(imageBuf)
  {
    free(imageBuf);
    imageBuf = NULL;
  }

  return 0;
}

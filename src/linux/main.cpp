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

#include "LinuxPlatformInterface.h"
#include "../common/Pulse.h"
#include "../common/trace_math.h"

LinuxPlatformInterface * plint = NULL;
Pulse * pulse = NULL;
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
Atom wmDeleteMessage = 0;
bool quitMessage = false;

void OutlinePrint(Drawable dr, const int x, const int y, const char* str)
{
  XSetForeground(dsp, gc, 0xAAAAAA);
  XDrawString(dsp, dr, gc, x - 1, y - 1, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x, y - 1, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x + 1, y - 1, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x - 1, y, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x + 1, y, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x - 1, y + 1, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x, y + 1, str, (int)strlen(str));
  XDrawString(dsp, dr, gc, x + 1, y + 1, str, (int)strlen(str));
  XSetForeground(dsp, gc, 0);
  XDrawString(dsp, dr, gc, x, y, str, (int)strlen(str));
}

void DrawImage(Drawable dr)
{
  if (pulse->imageReady)
  {
    if (!image || image->width != (int)clientWidth || image->height != (int)clientHeight)
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

      pixels = (uint32_t*)malloc(clientWidth * clientHeight * 4);

      if(pixels)
        image = XCreateImage(dsp, vis, 24, ZPixmap, 0, (char*)pixels, clientWidth, clientHeight, 32, 0);

      assert(pixels);
      assert(image);
    }

    if(pixels)
    {
      for (unsigned int y = 0; y < clientHeight; ++y)
      for (unsigned int x = 0; x < clientWidth; ++x)
        pixels[x + (clientHeight - y - 1) * clientWidth] = pulse->getRenderImagePixel(x, y);
    }

    pulse->imageReady = false;
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

void DrawTextTips(Drawable dr)
{
  std::vector<std::string> & text = *pulse->getCurrentScreenText();
  const int x = 3;
  int y = fontAscent + 3;

  for (int i = 0, sz = (int)text.size(); i < sz; i++)
  {
    OutlinePrint(dr, x, y, text[i].c_str());
    y += lineHeight;
  }
}

// -----------------------------------------------------------------------------
//                                 Events
// -----------------------------------------------------------------------------

void OnResize(XConfigureEvent & e)
{
  Window root;
  int x, y;
  unsigned int width, height, border, depth;
  XGetGeometry(dsp, win, &root, &x, &y, &width, &height, &border, &depth);

  if(!pixmap || width != clientWidth || height != clientHeight)
  {
    if (width && height)
    {
      if(pixmap)
      {
        XFreePixmap(dsp, pixmap);
        pixmap = 0;
      }

      pixmap = XCreatePixmap(dsp, win, width, height, 24);
      XFillRectangle(dsp, pixmap, gc, 0, 0, width, height);

      pulse->onResize(width, height);

      clientWidth = width;
      clientHeight = height;
    }
  }
}

void OnExpose(XExposeEvent & e)
{
  if(!e.count)
  {
    DrawImage(pixmap);
    DrawTextTips(pixmap);

    XCopyArea(dsp, pixmap, win, gc, 0, 0, clientWidth, clientHeight, 0, 0);
    XFlush(dsp);
  }
}

void OnKeyEvent(XKeyEvent & e, bool isPressed)
{
  const int buf_size = 256;
  char buf[buf_size];
  KeySym ks;
  XLookupString(&e, buf, buf_size, &ks, NULL);

  switch (ks)
  {
    case XK_Left:     pulse->onKeyEvent(KEY_LEFT, isPressed);    break;
    case XK_Right:    pulse->onKeyEvent(KEY_RIGHT, isPressed);   break;
    case XK_Up:       pulse->onKeyEvent(KEY_UP, isPressed);      break;
    case XK_Down:     pulse->onKeyEvent(KEY_DOWN, isPressed);    break;
    case XK_W:
    case XK_w:        pulse->onKeyEvent(KEY_W, isPressed);       break;
    case XK_S:
    case XK_s:        pulse->onKeyEvent(KEY_S, isPressed);       break;
    case XK_A:
    case XK_a:        pulse->onKeyEvent(KEY_A, isPressed);       break;
    case XK_D:
    case XK_d:        pulse->onKeyEvent(KEY_D, isPressed);       break;
    case XK_space:    pulse->onKeyEvent(KEY_SPACE, isPressed);   break;
    case XK_Control_L:
    case XK_Control_R:pulse->onKeyEvent(KEY_CONTROL, isPressed); break;
    case XK_F2:       pulse->onKeyEvent(KEY_F2, isPressed);      break;
    case XK_1:        pulse->onKeyEvent(KEY_1, isPressed);       break;
    case XK_2:        pulse->onKeyEvent(KEY_2, isPressed);       break;
    case XK_3:        pulse->onKeyEvent(KEY_3, isPressed);       break;
    case XK_4:        pulse->onKeyEvent(KEY_4, isPressed);       break;
    case XK_5:        pulse->onKeyEvent(KEY_5, isPressed);       break;
    case XK_6:        pulse->onKeyEvent(KEY_6, isPressed);       break;
    case XK_7:        pulse->onKeyEvent(KEY_7, isPressed);       break;
    case XK_8:        pulse->onKeyEvent(KEY_8, isPressed);       break;
    case XK_9:        pulse->onKeyEvent(KEY_9, isPressed);       break;
    case XK_Escape:   pulse->onKeyEvent(KEY_ESCAPE, isPressed);  break;
    case XK_Y:
    case XK_y:        pulse->onKeyEvent(KEY_Y, isPressed);       break;
    case XK_N:
    case XK_n:        pulse->onKeyEvent(KEY_N, isPressed);       break;
  }
}

void ProcessEvent(XEvent & e)
{
  if (e.type == ClientMessage && e.xclient.data.l[0] == (int)wmDeleteMessage)
    quitMessage = true;

  switch(e.type)
  {
  case Expose:
    OnExpose(e.xexpose);
    break;
  case KeyPress:
    OnKeyEvent(e.xkey, true);
    break;
  case KeyRelease:
    OnKeyEvent(e.xkey, false);
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

    wmDeleteMessage = XInternAtom(dsp, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(dsp, win, &wmDeleteMessage, 1);

    plint = new LinuxPlatformInterface(dsp, win);
    pulse = new Pulse(plint);

    XMapWindow(dsp, win);
    XFlush(dsp);

    XEvent e;

    while (!quitMessage)
    {
      while(!quitMessage && XPending(dsp))
      {
        XNextEvent(dsp, &e);
        ProcessEvent(e);
      }

      pulse->exec();
    }
  }

  if(image)
    XDestroyImage(image);
  else if(pixels)
    free(pixels);

  if(pixmap)
    XFreePixmap(dsp, pixmap);

    delete pulse;
    delete plint;

  return 0;
}

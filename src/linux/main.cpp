#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "../airly/airly.h"
#include "../airly/Scene.h"

Display * dsp = NULL;
int scr;
Window win;
XImage * image = NULL;
Visual * vis;
void * imageBuf = NULL;
char exePath[PATH_MAX + 1];
Scene * scene = NULL;


Pixmap pm;
GC gc;
XGCValues gcval;

void DrawScene()
{
  DWORD
  for(int x = 0; x < scene->screenWidth; x++)
  for(int y = 0; y < scene->screenHeight; y++)
  {
    ARGB argb = scene->tracePixel(x, scene->screenHeight - y, 1).argb();
    ((uint32_t*)(imageBuf))[x + y * scene->screenWidth] = argb;
  }

  XPutImage(dsp, win, gc, image, 0, 0, 0, 0, scene->screenWidth, scene->screenHeight);

  char str[256];
  sprintf(str, "size %i x %i", w, h);
  XDrawString(dsp, win, DefaultGC(dsp, scr), 50, 50, str, strlen(str));
}

KeySym GetKey(XKeyEvent & e)
{
  const int buf_size = 256;
  char buf[buf_size];
  KeySym ks;
  XLookupString(&e, buf, buf_size, &ks, NULL);
  return ks;
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

  // Open a display.
  dsp = XOpenDisplay(0);

  if ( dsp )
  {
    scene = new Scene(exePath);
    scr = XDefaultScreen(dsp);
    vis = DefaultVisual(dsp, scr);

    // Create the window
    win = XCreateWindow(dsp, DefaultRootWindow(dsp), 0, 0, 640,
                 480, 1, CopyFromParent, CopyFromParent,
                 CopyFromParent, 0, 0);

    XSelectInput(dsp, win, ExposureMask | KeyPressMask | MappingNotify | StructureNotifyMask);

    // Show the window
    XMapWindow(dsp, win);
    XFlush(dsp);

    gcval.function = GXcopy;
    gcval.plane_mask = AllPlanes;
    gcval.foreground = BlackPixel(dsp, scr);
    gcval.background = WhitePixel(dsp, scr);
    gc = XCreateGC(dsp, win, GCFunction|GCPlaneMask|GCForeground|GCBackground, &gcval);

    XEvent e;
    KeySym ks;
    bool exitCmd = false;

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
        ks = GetKey(e.xkey);

        switch(ks)
        {
        case XK_F10:
          if(e.xkey.state & Mod1Mask)
            exitCmd = true;
          break;
        }
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

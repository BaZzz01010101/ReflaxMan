#Ray Tracer

This ray tracer currently support reflections, phong lighting, smooth shadows and super sampling.
The scene can contain textured skybox and any quantity of light sources and objects (spheres, planes and textured triangles supported).

You can interactively control camera with keyboard and save render to the BMP file with selectable resolution and super sampling rate. While moving camera, program will dynamically downgrade tracing resolution to provide comfortable framerate. After camera stops, it will render picture additively making smoother picture with time.

Project support windows and linux platforms. 
Visual Studio and Code::Blocks files included.

#Performance 

(Intel Core2 Duo E8400 @ 3.0 GHz / default camera view / resolution 640 x 480 / no SSAA)

Windows 7 x64 / Visual Studio 2013
  
- Win32 : 462 ms/frame
    
- x64 : 368 ms/frame
    
Windows 7 x64 / g++
  
- x64 : 580 ms/frame
    
Kubuntu-15.04-desktop-amd64 / g++
  
- amd64 : 598 ms/frame
    
    
#Screenshot
1920x1080, 128x128 SSAA:
[<img src="https://raw.githubusercontent.com/BaZzz01010101/ReflaxMan/master/scrnshoot.jpg" />](https://raw.githubusercontent.com/BaZzz01010101/ReflaxMan/master/scrnshoot.jpg)

#pragma once
#include "Scene.h"
#include "Texture.h"
#include "Camera.h"

class Render
{
private:
  int imageWidth;
  int imageHeight;
  int renderReflectNum; 
  int renderSampleNum;
  int scanLinesRendered;
  bool renderInProgress;

public:
  Texture image;
  Camera camera;
  Scene scene;

  Render(const char * exePath);
  ~Render();

  void loadScene(const char * exePath);
  void setImageSize(int width, int height);

  void renderBegin(int reflectNum, int sampleNum);
  int renderNext(int scanLines);
  void renderAll(int reflectNum, int sampleNum);
  
  inline float getRenderProgress() { return float(scanLinesRendered) * 100 / image.getHeight(); }
  inline bool inProgress() { return renderInProgress; }
  inline Color getImagePixel(float u, float v) { return image.getTexelColor(u, v); }
  inline Color getImagePixel(int x, int y) { return image.getTexelColor(x, y); }
};


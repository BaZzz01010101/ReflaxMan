#pragma once
#include <vector>
#include "Scene.h"
#include "Texture.h"
#include "Camera.h"

class Render
{
private:
  std::vector<Color> image;

  // staged render settings
  unsigned int curx;
  unsigned int cury;
  int renderReflectNum;
  int renderSampleNum;
  bool renderAdditive;
  Matrix33 renderCameraView;
  Vector3 renderCameraEye;

public:
  Camera camera;
  Scene scene;
  unsigned int imageWidth;
  unsigned int imageHeight;
  int additiveCounter;
  bool inProgress;

  Render(const char * exePath);
  ~Render();

  void loadScene(const char * exePath);
  void setImageSize(unsigned int width, unsigned int height);
  void copyImage(Texture & texture) const;
  Color imagePixel(int x, int y) const;

  void renderBegin(int reflectNum, int sampleNum, bool additive);
  bool renderNext(unsigned int pixels);
  void renderAll(int reflectNum, int sampleNum, bool additive);
  
  float getRenderProgress() const;
};


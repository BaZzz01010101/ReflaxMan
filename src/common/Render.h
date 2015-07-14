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
  int renderReflectNum;
  int renderSampleNum;
  bool renderAdditive;
  Matrix33 renderCameraView;
  Vector3 renderCameraEye;

  int scanLinesRendered;

public:
  Camera camera;
  Scene scene;
  int imageWidth;
  int imageHeight;
  int additiveCounter;
  bool inProgress;

  Render(const char * exePath);
  ~Render();

  void loadScene(const char * exePath);
  void setImageSize(int width, int height);
  void copyImage(Texture & texture) const;
  Color imagePixel(int x, int y) const;

  void renderBegin(int reflectNum, int sampleNum, bool additive);
  int renderNext(int scanLines);
  void renderAll(int reflectNum, int sampleNum, bool additive);
  
  inline float getRenderProgress() const { return float(scanLinesRendered) * 100 / imageHeight; }
};


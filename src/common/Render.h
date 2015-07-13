#pragma once
#include <vector>
#include "Scene.h"
#include "Texture.h"
#include "Camera.h"

class Render
{
private:
  std::vector<Color> image;
  int renderReflectNum;
  int renderSampleNum;
  bool renderAdditive;
  int additiveCounter;
  int scanLinesRendered;
  bool renderInProgress;
  Matrix33 renderCameraView;
  Vector3 renderCameraEye;

public:
  Camera camera;
  Scene scene;
  int imageWidth;
  int imageHeight;

  Render(const char * exePath);
  ~Render();

  void loadScene(const char * exePath);
  void setImageSize(int width, int height);
  void copyImage(Texture & texture);
  Color imagePixel(int x, int y);

  void renderBegin(int reflectNum, int sampleNum, bool additive);
  void renderRestart();
  int renderNext(int scanLines);
  void renderAll(int reflectNum, int sampleNum, bool additive);
  
  inline float getRenderProgress() { return float(scanLinesRendered) * 100 / imageHeight; }
  inline bool inProgress() { return renderInProgress; }
};


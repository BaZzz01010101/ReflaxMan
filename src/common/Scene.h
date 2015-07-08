#pragma once
#include <vector>
#include "SceneObject.h"
#include "OmniLight.h"
#include "Camera.h"
#include "Skybox.h"
#include "Texture.h"
#include "Color.h"

class Scene
{
private:
  typedef std::vector<SceneObject*> SCENE_OBJECTS;
  typedef std::vector<OmniLight*> SCENE_LIGHTS;
  SCENE_OBJECTS sceneObjects;
  SCENE_LIGHTS sceneLights;
  Color envColor;
  Skybox skybox;
  Texture planeTexture;

public:
  Color diffLightColor;
  float diffLightPower;
  Camera camera;
  int screenWidth;
  int screenHeight;

  Scene(const wchar_t* exePath);
  ~Scene();

  Color tracePixel(const int x, const int y, const int aa) const;
};


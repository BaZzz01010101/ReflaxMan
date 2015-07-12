#pragma once
#include <vector>
#include "SceneObject.h"
#include "OmniLight.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Camera.h"
#include "Skybox.h"
#include "Texture.h"
#include "Color.h"

class Scene
{
private:
  typedef std::vector<SceneObject*> SCENE_OBJECTS;
  typedef std::vector<OmniLight*> SCENE_LIGHTS;
  typedef std::vector<Texture*> SCENE_TEXTURES;

  SCENE_OBJECTS sceneObjects;
  SCENE_LIGHTS sceneLights;
  SCENE_TEXTURES sceneTextures;

  Color envColor;
  Skybox skybox;
  Color diffLightColor;
  float diffLightPower;

public:
  Scene();
  Scene(const Color & diffLightColor, float diffLightPower);
  ~Scene();

  Sphere * addSphere(const Vector3 & center, float radius, const Material & material);
  Triangle * addTriangle(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, const Material & material);
  OmniLight * addLight(const Vector3 & origin, float radius, const Color & color, float power);
  Texture * addTexture(const char * fileName);
  bool setSkyboxTexture(const char * fileName);

  Color trace(Vector3 origin, Vector3 ray, int reflNumber) const;
};


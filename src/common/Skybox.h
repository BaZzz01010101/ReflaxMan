#pragma once
#include "Vector3.h"
#include "Plane.h"
#include "Texture.h"

class Skybox 
{
private:
  float halfTileWidth;
  float halfTileHeight;
  Texture texture;

public:
  Skybox();
  Skybox(const char * textureFileName);
  ~Skybox();

  bool loadTexture(const char* fileName);
  Color getTexelColor(const Vector3 & ray) const;
};


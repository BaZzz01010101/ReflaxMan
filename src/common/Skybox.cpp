#include "airly.h"
#include "Skybox.h"


Skybox::Skybox()
{
  halfTileWidth = 1.0f / 8.0f - FLT_EPSILON;
  halfTileHeight = 1.0f / 6.0f - FLT_EPSILON;
}

Skybox::Skybox(const wchar_t * textureFileName)
{
  bool success = loadTexture(textureFileName);
  assert(success);
}

Skybox::~Skybox()
{
}

bool Skybox::loadTexture(const wchar_t* fileName)
{
  if (texture.loadFromFile(fileName))
  {
    halfTileWidth = 1.0f / 8.0f - 1.0f / texture.getWidth() - FLT_EPSILON;
    halfTileHeight = 1.0f / 6.0f - 1.0f / texture.getHeight() - FLT_EPSILON;

    return true;
  }
  else
  {
    halfTileWidth = 1.0f / 8.0f - FLT_EPSILON;
    halfTileHeight = 1.0f / 6.0f - FLT_EPSILON;

    return false;
  }
}

Color Skybox::getTexelColor(const Vector3 & ray) const
{
  const float uLeft = 1.0f / 8.0f;
  const float vLeft = 3.0f / 6.0f;
  const float uFront = 3.0f / 8.0f;
  const float vFront = 3.0f / 6.0f;
  const float uRight = 5.0f / 8.0f;
  const float vRight = 3.0f / 6.0f;
  const float uBack = 7.0f / 8.0f;
  const float vBack = 3.0f / 6.0f;
  const float uTop = 3.0f / 8.0f;
  const float vTop = 5.0f / 6.0f;
  const float uBottom = 3.0f / 8.0f;
  const float vBottom = 1.0f / 6.0f;

  Vector3 nray = normalize(ray);
  float x = nray.x;
  float y = nray.y;
  float z = nray.z;
  float ax = fabs(x) + VERY_SMALL_NUMBER;
  float ay = fabs(y) + VERY_SMALL_NUMBER;
  float az = fabs(z) + VERY_SMALL_NUMBER;
  float u = 0;
  float v = 0;
  
  if (az >= ax && az >= ay)
  {
    if (z > 0)  
    {
      u = uFront + x / az * halfTileWidth;
      v = vFront + y / az * halfTileHeight;
    }
    else        
    {
      u = uBack - x / az * halfTileWidth;
      v = vBack + y / az * halfTileHeight;
    }
  }
  else if (ax >= ay && ax >= az)
  {
    if (x > 0)  
    {
      u = uRight - z / ax * halfTileWidth;
      v = vRight + y / ax * halfTileHeight;
    }
    else        
    {
      u = uLeft + z / ax * halfTileWidth;
      v = vLeft + y / ax * halfTileHeight;
    }
  }
  else 
  {
    if (y > 0)
    {
      u = uTop + x / ay * halfTileWidth;
      v = vTop - z / ay * halfTileHeight;
    }
    else
    {
      u = uBottom + x / ay * halfTileWidth;
      v = vBottom + z / ay * halfTileHeight;
    }
  }

  return Color(texture.getTexelColor(u, v));
}

#pragma once
#include "Vector3.h"
#include "Color.h"

class OmniLight
{
public:
  Vector3 origin;
  float radius;
  Color color;
  float power;

  OmniLight();
  OmniLight(const Vector3 & origin, const float radius, const Color & color, const float power);
  OmniLight(const OmniLight & l);
  OmniLight & operator = (const OmniLight & l);
  ~OmniLight();
};


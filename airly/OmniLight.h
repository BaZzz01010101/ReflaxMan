#pragma once
#include "Vector3.h"
#include "Color.h"

class OmniLight
{
public:
  Vector3 origin;
  Color color;
  float power;

  OmniLight();
  OmniLight(const OmniLight & l);
  OmniLight & operator = (const OmniLight & l);
  OmniLight(const Vector3 & origin, const Color & color, const float power);
  ~OmniLight();
};


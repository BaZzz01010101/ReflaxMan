#include "trace_math.h"
#include "OmniLight.h"

OmniLight::OmniLight()
{
}

OmniLight::OmniLight(const Vector3 & origin, const float radius, const Color & color, const float power) :
  origin(origin),
  color(color)
{
  this->radius = radius;
  this->power = clamp(power, 0.0f, 1.0f);
}

OmniLight::OmniLight(const OmniLight & l) :
origin(l.origin),
color(l.color)
{
  radius = l.radius;
  power = l.power;
}

OmniLight & OmniLight::operator = (const OmniLight & l)
{
  origin = l.origin;
  radius = l.radius;
  color = l.color;
  power = l.power;

  return *this;
}

OmniLight::~OmniLight()
{
}

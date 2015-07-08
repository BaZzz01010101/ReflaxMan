#include "airly.h"
#include "OmniLight.h"

OmniLight::OmniLight()
{
}

OmniLight::OmniLight(const OmniLight & l)
{
  origin = l.origin;
  radius = l.radius;
  color = l.color;
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

OmniLight::OmniLight(const Vector3 & origin, const float radius, const Color & color, const float power)
{
  this->origin = origin;
  this->radius = radius;
  this->color = color;
  this->power = clamp(power, 0.0f, 1.0f);
}

OmniLight::~OmniLight()
{
}

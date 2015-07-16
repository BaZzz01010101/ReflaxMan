#include "trace_math.h"
#include "Vector3.h"
#include "Sphere.h"

Sphere::Sphere()
{

}

Sphere::Sphere(const Vector3 & center, float radius, const Material & material) 
{
  assert(radius > VERY_SMALL_NUMBER);

  if (radius <= VERY_SMALL_NUMBER)
    radius = VERY_SMALL_NUMBER;

  this->center = center;
  this->radius = radius;
  this->sqRadius = radius * radius;
  this->material = material;
}

Sphere::Sphere(const Sphere & sphere) 
{
  center = sphere.center;
  radius = sphere.radius;
  sqRadius = radius * radius;
  material = sphere.material;
}

Sphere & Sphere::operator =(const Sphere & sphere)
{
  center = sphere.center;
  radius = sphere.radius;
  sqRadius = radius * radius;
  material = sphere.material;

  return *this;
}

Sphere::~Sphere()
{
}

bool Sphere::trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, 
  Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const
{
// tracing intersections only from outside of sphere

  const Vector3 vco = origin - center;
  const float a = ray.sqLength();
  const float b = 2.0f * ray * vco;
  const float c = vco.sqLength() - sqRadius;
  const float d = b * b - 4.0f * a * c;

  if (d >= 0.0f && a > VERY_SMALL_NUMBER) 
  {
    const float t = (-b - sqrtf(d)) / (2.0f * a);

    if (t > VERY_SMALL_NUMBER)
    {
      const Vector3 fullRay = ray * t;
      const float distance = fullRay.length();

      if (distance > DELTA)
      {
        const Vector3 drop = origin + fullRay;
        const Vector3 norm = drop - center;

        if (out_distance)
          *out_distance = distance;
        if (out_drop)
          *out_drop = drop;
        if (out_norm)
          *out_norm = norm;
        if (out_reflected_ray)
          *out_reflected_ray = reflect(fullRay, norm);
        if (out_drop_material)
          *out_drop_material = material;

        return true;
      }
    }
  }
  return false;
}


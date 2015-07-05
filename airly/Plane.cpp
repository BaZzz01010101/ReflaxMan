#include <cmath>
#include "airly.h"
#include "Plane.h"


Plane::Plane()
{
}

Plane::Plane(const Plane & plane) 
{
  material = plane.material;
  pos = plane.pos;
  norm = plane.norm;
}

Plane & Plane::operator =(const Plane & plane)
{
  material = plane.material;
  pos = plane.pos;
  norm = plane.norm;

  return *this;
}

Plane::Plane(const Vector3 & pos, const Vector3 & norm, const Material & material) 
{
  this->material = material;
  this->pos = pos;
  this->norm = norm;
}

Plane::~Plane()
{
}

bool Plane::trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const
{
  Vector3 vop = pos - origin;
  float a = norm * ray;

  if (fabs(a) > VERY_SMALL_NUMBER)
  {
    float t = (norm * vop) / a;

    if (t > VERY_SMALL_NUMBER) 
    {
      Vector3 fullRay = ray * t;
      float sqDistance = fullRay.sqLength();

      if (sqDistance > DELTA * DELTA)
      {
        if (out_drop)
          *out_drop = origin + fullRay;

        if (out_norm)
          *out_norm = norm;

        if (out_reflected_ray)
          *out_reflected_ray = reflect(fullRay, norm);

        if (out_distance)
          *out_distance = sqrtf(sqDistance);

        if (out_drop_material)
          *out_drop_material = material;

        return true;
      }
    }
  }
  return false;
}

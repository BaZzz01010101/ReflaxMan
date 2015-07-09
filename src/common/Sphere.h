#pragma once
#include "Vector3.h"
#include "Material.h"
#include "SceneObject.h"

class Sphere : public SceneObject
{
private:
  Vector3 center;
  float radius;
  float sqRadius;
  Material material;

public:
  Sphere();
  Sphere(const Sphere & sphere);
  Sphere & operator =(const Sphere & sphere);
  Sphere(const Vector3 & center, const float radius, const Material & material);
  ~Sphere();

  bool trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, 
    Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const;
};


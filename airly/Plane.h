#pragma once
#include "SceneObject.h"
#include "Vector3.h"

class Plane : public SceneObject
{
public:
  Vector3 pos;
  Vector3 norm;
  Material material;

  Plane();
  Plane(const Plane & plane);
  Plane & operator =(const Plane & plane);
  Plane(const Vector3 & pos, const Vector3 & norm, const Material & material);
  ~Plane();

  bool trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const;
};


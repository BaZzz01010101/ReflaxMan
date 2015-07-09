#pragma once
#include "Vector3.h"
#include "Material.h"

class SceneObject
{
public:
  SceneObject();
  virtual ~SceneObject();

  virtual bool trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, 
    Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const = 0;
};


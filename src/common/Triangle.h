#pragma once
#include "SceneObject.h"
#include "Vector3.h"
#include "Texture.h"
#include "Matrix33.h"

class Triangle : public SceneObject
{
private:
  Matrix33 axTrans, tuvTrans;
  Vector3 v[3]; // vertexes
  float tu[3];  // vertexes texture u-coords
  float tv[3];  // vertexes texture v-coords
  Vector3 norm;
  Material material;
  const Texture * texture;

public:
  Triangle();
  Triangle(const Triangle & Triangle);
  Triangle & operator =(const Triangle & Triangle);
  Triangle(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, const Material & material);
  ~Triangle();

  void setTexture(const Texture * texture, const float u1, const float v1, const float u2, const float v2, 
    const float u3, const float v3);

  bool trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, 
    Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const;
};

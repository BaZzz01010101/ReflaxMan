#include "airly.h"
#include "Triangle.h"

Triangle::Triangle()
{
  texture = NULL;
}

Triangle::Triangle(const Triangle & triangle)
{
  memcpy(v, triangle.v, sizeof(v));
  memcpy(tu, triangle.tu, sizeof(tu));
  memcpy(tv, triangle.tv, sizeof(tv));
  material = triangle.material;
  norm = triangle.norm;
  texture = triangle.texture;
  axTrans = triangle.axTrans;
  tuvTrans = triangle.axTrans;
}

Triangle & Triangle::operator =(const Triangle & triangle)
{
  memcpy(v, triangle.v, sizeof(v));
  memcpy(tu, triangle.tu, sizeof(tu));
  memcpy(tv, triangle.tv, sizeof(tv));
  material = triangle.material;
  norm = triangle.norm;
  axTrans = triangle.axTrans;
  texture = triangle.texture;
  tuvTrans = triangle.axTrans;

  return *this;
}

Triangle::Triangle(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, const Material & material)
{
  v[0] = v1; v[1] = v2; v[2] = v3;
  this->material = material;
  texture = NULL;
  norm = normalize((v2 - v1) % (v3 - v1));
  axTrans = Matrix33(v3 - v1, v2 - v1, -norm);
  axTrans.invert();
}

Triangle::~Triangle() 
{
}

bool Triangle::trace(const Vector3 & origin, const Vector3 & ray, Vector3 * const out_drop, Vector3 * const out_norm, Vector3 * const out_reflected_ray, float * const out_distance, Material * const out_drop_material) const
{
  Vector3 axOrigin = axTrans * (origin - v[0]);
  Vector3 axRay = axTrans * ray;

  if (fabs(axRay.z) > VERY_SMALL_NUMBER)
  {
    float t = -axOrigin.z / axRay.z;

    if (t > VERY_SMALL_NUMBER)
    {
      float u = axOrigin.x + t * axRay.x;
      float v = axOrigin.y + t * axRay.y;

      if (u >= 0.0f && v >= 0.0f && u + v < 1.0f)
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
          {
            if (texture)
            {
              Vector3 tvec = tuvTrans * Vector3(u, v, 0);
              (*out_drop_material).color = texture->getTexelColor(tu[0] + tvec.x, tv[0] + tvec.y);
              (*out_drop_material).type = material.type;
              (*out_drop_material).reflectivity = material.reflectivity;
              (*out_drop_material).transparency = material.transparency;
            }
            else
              *out_drop_material = material;
          }

          return true;
        }
      }
    }
  }

  return false;
}

void Triangle::setTexture(const Texture * texture, const float tu1, const float tv1, const float tu2, const float tv2, const float tu3, const float tv3)
{
  this->texture = texture;
  tu[0] = tu1; tu[1] = tu2; tu[2] = tu3;
  tv[0] = tv1; tv[1] = tv2; tv[2] = tv3;
  Vector3 v1(tu1, tv1, 0);
  Vector3 v2(tu2, tv2, 0);
  Vector3 v3(tu3, tv3, 0);
  tuvTrans = Matrix33(v3 - v1, v2 - v1, Vector3(0, 0, -1));
}

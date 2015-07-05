#pragma once
#include "Color.h"


class Material
{
public:
  enum Type { mtMetal, mtDielectric };
  Type type;
  Color color;
  float reflectivity;
  float transparency;

  Material();
  Material(const Material & mat);
  Material & operator = (const Material & mat);
  Material(const Type type, const Color color, const float reflectivity, const float transparency);
  ~Material();
};


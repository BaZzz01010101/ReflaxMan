#include "airly.h"
#include "Material.h"

Material::Material()
{
}

Material::Material(const Type type, const Color color, const float reflectivity, const float transparency)
{
  this->type = type;
  this->color = color;
  this->reflectivity = clamp(reflectivity, 0.0f, 1.0f);
  this->transparency = clamp(transparency, 0.0f, 1.0f);
}

Material::Material(const Material & mat)
{
  type = mat.type;
  color = mat.color;
  reflectivity = mat.reflectivity;
  transparency = mat.transparency;
}

Material & Material::operator = (const Material & mat)
{
  type = mat.type;
  color = mat.color;
  reflectivity = mat.reflectivity;
  transparency = mat.transparency;

  return *this;
}

Material::~Material()
{
}

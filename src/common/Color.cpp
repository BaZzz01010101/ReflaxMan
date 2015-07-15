#include "trace_math.h"
#include "Color.h"


Color::Color()
{
}

Color::Color(const ARGB & c)
{
  r = float(ARGB_RED(c)) / 255.0f;
  g = float(ARGB_GREEN(c)) / 255.0f;
  b = float(ARGB_BLUE(c)) / 255.0f;
}

Color::Color(const float r, const float g, const float b)
{
  this->r = r;
  this->g = g;
  this->b = b;
}

Color::Color(const Color & c)
{
  r = c.r;
  g = c.g;
  b = c.b;
}

Color & Color::operator = (const Color & c)
{
  r = c.r;
  g = c.g;
  b = c.b;
  return *this;
}

Color::~Color()
{
}

Color & Color::operator *=(const float & f) 
{
  r *= f;
  g *= f;
  b *= f;
  return *this;
}

Color & Color::operator /=(const float & f) 
{
  assert(fabs(f) > VERY_SMALL_NUMBER);

  if (fabs(f) > VERY_SMALL_NUMBER)
  {
    r /= f;
    g /= f;
    b /= f;
  }
  return *this;
}

Color & Color::operator +=(const Color & c)
{
  r += c.r;
  g += c.g;
  b += c.b;
  return *this;
}

Color & Color::operator *=(const Color & c)
{
  r *= c.r;
  g *= c.g;
  b *= c.b;
  return *this;
}

Color Color::operator +(const Color & c) const
{
  return Color(r + c.r, g + c.g, b + c.b);
}

Color Color::operator -(const Color & c) const
{
  return Color(r - c.r, g - c.g, b - c.b);
}

Color Color::operator *(const float & f) const
{
  return Color(r * f, g * f, b * f);
}

Color operator *(const float & f, const Color & c)
{
  return Color(c.r * f, c.g * f, c.b * f);
}

Color Color::operator /(const float & f) const
{
  assert(fabs(f) > VERY_SMALL_NUMBER);

  if (fabs(f) > VERY_SMALL_NUMBER)
    return Color(r / f, g / f, b / f);
  else
    return *this;
}

Color Color::operator *(const Color & f) const
{
  return Color(r * f.r, g * f.g, b * f.b);
}

ARGB Color::argb() const
{
  return MAKEARGB(0, (r - FLT_EPSILON) * 256.0f, (g - FLT_EPSILON)  * 256.0f, (b - FLT_EPSILON)  * 256.0f);
}

void Color::clamp()
{
  r = Airly::clamp(r, 0.0f, 1.0f);
  g = Airly::clamp(g, 0.0f, 1.0f);
  b = Airly::clamp(b, 0.0f, 1.0f);
}
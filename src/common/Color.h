#pragma once
#include "stdint.h"

typedef uint32_t ARGB;

#define ALPHA_SHIFT 24
#define RED_SHIFT   16
#define GREEN_SHIFT 8
#define BLUE_SHIFT  0
#define ALPHA_MASK  ((ARGB) 0xff << ALPHA_SHIFT)
#define MAKEARGB(a, r, g, b) \
  (((ARGB)(unsigned char(a) & 0xff) << ALPHA_SHIFT) | \
  ((ARGB)(unsigned char(r) & 0xff) << RED_SHIFT) | \
  ((ARGB)(unsigned char(g) & 0xff) << GREEN_SHIFT) | \
  ((ARGB)(unsigned char(b) & 0xff) << BLUE_SHIFT))
#define ARGB_RED(argb) ((argb >> RED_SHIFT) & 0xFF)
#define ARGB_GREEN(argb) ((argb >> GREEN_SHIFT) & 0xFF)
#define ARGB_BLUE(argb) ((argb >> BLUE_SHIFT) & 0xFF)
#define ARGB_ALPHA(argb) ((argb >> ALPHA_SHIFT) & 0xFF)

class Color
{
public:
  float r;
  float g;
  float b;

  Color();
  Color(const Color & c);
  Color & operator =(const Color & c);
  Color(const ARGB & bgra);
  Color(const float r, const float g, const float b);
  ~Color();

  Color & operator *=(const float & f);
  Color & operator /=(const float & f);

  Color & operator +=(const Color & c);
  Color & operator *=(const Color & c);

  Color operator *(const float & f) const;
  friend Color operator *(const float & f, const Color & c);
  Color operator /(const float & f) const;

  Color operator +(const Color & c) const;
  Color operator -(const Color & c) const;
  Color operator *(const Color & c) const;

  ARGB argb() const;
  void clamp();
};


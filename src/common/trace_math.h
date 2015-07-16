#pragma once
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

#include "Vector3.h"

namespace Airly
{
  const float VERY_SMALL_NUMBER = sqrtf(FLT_MIN);
  const float DELTA = 0.0001f;

  Vector3 normalize(const Vector3 & v);
  Vector3 reflect(const Vector3 & v, const Vector3 & norm);

  inline double clamp(double val, double min, double max) { return val < min ? min : val > max ? max : val; }
  inline float clamp(float val, float min, float max) { return val < min ? min : val > max ? max : val; }
  inline int clamp(int val, int min, int max) { return val < min ? min : val > max ? max : val; }

  inline int min(int a, int b) { return a < b ? a : b; };
  inline int max(int a, int b) { return a > b ? a : b; };
  inline int min(unsigned int a, unsigned int b) { return a < b ? a : b; };
  inline int max(unsigned int a, unsigned int b) { return a > b ? a : b; };
  inline float min(float a, float b) { return a < b ? a : b; };
  inline float max(float a, float b) { return a > b ? a : b; };

  static int g_seed = rand();
  static const int FAST_RAND_MAX = 0x7FFF;
  inline int fastrand() {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & FAST_RAND_MAX;
  }
}

#ifndef FORBIDE_USING_AIRLY_NAMESPACE
using namespace Airly;
#endif

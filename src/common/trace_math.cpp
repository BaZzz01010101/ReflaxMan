#include "trace_math.h"

Vector3 Tracemath::normalize(const Vector3 & v)
{
  float len = v.length();
  assert(len > VERY_SMALL_NUMBER);

  if (len > VERY_SMALL_NUMBER)
    return v / len;
  else
    return v;
}

Vector3 Tracemath::reflect(const Vector3 & v, const Vector3 & norm)
{
  float dotNorm = norm * norm;
  assert(dotNorm > VERY_SMALL_NUMBER);

  if (dotNorm > VERY_SMALL_NUMBER)
    return v - 2.0f * norm * ((v * norm) / dotNorm);
  else
    return v;
}

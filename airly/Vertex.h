#pragma once
#include "Vector3.h"

class Vertex
{
public:
  Vector3 v;
  float tu;
  float tv;

  Vertex();
  Vertex(const Vertex & vert);
  Vertex(const Vector3 & v, const float tu, const float tv);
  ~Vertex();

  Vertex & operator =(const Vertex & vert);
};


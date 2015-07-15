#include "trace_math.h"
#include "Vertex.h"


Vertex::Vertex()
{
}

Vertex::Vertex(const Vertex & vert)
{
  v = vert.v;
  tu = vert.tu;
  tv = vert.tv;
}

Vertex::Vertex(const Vector3 & v, const float tu, const float tv)
{
  this->v = v;
  this->tu = tu;
  this->tv = tv;
}

Vertex::~Vertex()
{
}

Vertex & Vertex::operator =(const Vertex & vert)
{
  v = vert.v;
  tu = vert.tu;
  tv = vert.tv;

  return *this;
}


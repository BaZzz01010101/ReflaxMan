#pragma once
class Quaternion
{
public:
  float x;
  float y;
  float z;
  float w;
  Quaternion(const float x, const float y, const float z, const float w);
  ~Quaternion();
};


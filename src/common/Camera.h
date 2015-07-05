#pragma once
#include "Vector3.h"
#include "Matrix33.h"

class Camera
{
private:
  Matrix33 cwQuant, ccwQuant, leftQuant, rightQuant, upQuant, downQuant;
  void initRotationQuants();
public:
  enum Rotation { crLeft, crRight, crUp, crDown, crCwise, crCcwise };
  enum Movement { cmLeft, cmRight, cmUp, cmDown, cmForward, cmBack };
  float fov;
  Vector3 eye;
  Matrix33 view;

  Camera();
  Camera(const Camera & camera);
  Camera & operator =(const Camera & camera);
  Camera(const Vector3 & eye, const Vector3 & at, const Vector3 & up, const float fov);
  ~Camera();

  void rotate(Rotation rot);
  void move(Movement mov);
};


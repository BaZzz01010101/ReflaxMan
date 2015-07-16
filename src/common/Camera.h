#pragma once
#include "Vector3.h"
#include "Matrix33.h"

enum Control
{
  turnLeftMask = 1 << 0,
  turnRightMask = 1 << 1,
  turnUpMask = 1 << 2,
  turnDownMask = 1 << 3,
  shiftLeftMask = 1 << 6,
  shiftRightMask = 1 << 7,
  shiftUpMask = 1 << 8,
  shiftDownMask = 1 << 9,
  shiftForwardMask = 1 << 10,
  shiftBackMask = 1 << 11,
};

namespace Default
{
  const float turnAccel = 2.0f;
  const float turnDecel = 2.0f;
  const float maxTurnSpeed = 0.2f;

  const float shiftAccel = 50.0f;
  const float shiftDecel = 50.0f;
  const float maxShiftSpeed = 10.0f;
}

class Camera
{
private:
  float turnAccel;
  float turnDecel;
  float shiftAccel;
  float shiftDecel;
  float maxTurnSpeed;
  float maxShiftSpeed;

public:
  float turnRLSpeed;
  float turnUDSpeed;
  float shiftRLSpeed;
  float shiftUDSpeed;
  float shiftFBSpeed;

  float yaw;
  float pitch;

  float fov;
  Vector3 eye;
  Matrix33 view;

  Camera();
  Camera(const Vector3 & eye, const Vector3 & at, const float fov);
  Camera(const Camera & camera);
  Camera & operator =(const Camera & camera);
  ~Camera();

  void proceedControl(const int controlFlags, const float timePassedSec);
  bool inMotion() const;
};


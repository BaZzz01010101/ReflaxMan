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

const float defTurnAccel = 5.0f;
const float defTurnDecel = 5.0f;
const float defMaxTurnSpeed = 0.25f;

const float defShiftAccel = 50.0f;
const float defShiftDecel = 50.0f;
const float defMaxShiftSpeed = 10.0f;

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
  Camera(const Vector3 & eye, const Vector3 & at, /*const Vector3 & up, */const float fov);
  Camera(const Camera & camera);
  Camera & operator =(const Camera & camera);
  ~Camera();

  void proceedControl(int controlFlags, int timePassedMs);
  bool inMotion();
};


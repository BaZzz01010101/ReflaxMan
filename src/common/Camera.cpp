#include "airly.h"
#include "Camera.h"

Camera::Camera()
{
  yaw = 0;
  pitch = 0;

  turnAccel = Default::turnAccel;
  turnDecel = Default::turnDecel;
  shiftAccel = Default::shiftAccel;
  shiftDecel = Default::shiftDecel;
  maxTurnSpeed = Default::maxTurnSpeed;
  maxShiftSpeed = Default::maxShiftSpeed;

  turnRLSpeed = 0;
  turnUDSpeed = 0;
  shiftRLSpeed = 0;
  shiftUDSpeed = 0;
  shiftFBSpeed = 0;
}

Camera::Camera(const Vector3 & eye, const Vector3 & at, const float fov)
{
  this->eye = eye;
  this->fov = fov;

  Vector3 up(0.0f, 1.0f, 0.0f);
  Vector3 oz = normalize(at - eye);
  Vector3 ox = normalize(up % oz);
  Vector3 oy = normalize(oz % ox);

  view.setCol(0, ox);
  view.setCol(1, oy);
  view.setCol(2, oz);

  yaw = acos(ox.z);
  if (ox.x < 0)
    yaw = 2.0f * float(M_PI) - yaw;
  yaw -= M_PI_2;
  pitch = asin(oz.y);

  turnAccel = Default::turnAccel;
  turnDecel = Default::turnDecel;
  shiftAccel = Default::shiftAccel;
  shiftDecel = Default::shiftDecel;
  maxTurnSpeed = Default::maxTurnSpeed;
  maxShiftSpeed = Default::maxShiftSpeed;

  turnRLSpeed = 0;
  turnUDSpeed = 0;
  shiftRLSpeed = 0;
  shiftUDSpeed = 0;
  shiftFBSpeed = 0;
}

Camera::Camera(const Camera & camera)
{
  eye = camera.eye;
  fov = camera.fov;
  view = camera.view;

  yaw = camera.yaw;
  pitch = camera.pitch;

  turnAccel = camera.turnAccel;
  turnDecel = camera.turnDecel;
  shiftAccel = camera.shiftAccel;
  shiftDecel = camera.shiftDecel;
  maxTurnSpeed = camera.maxTurnSpeed;
  maxShiftSpeed = camera.maxShiftSpeed;

  turnRLSpeed = 0;
  turnUDSpeed = 0;
  shiftRLSpeed = 0;
  shiftUDSpeed = 0;
  shiftFBSpeed = 0;
}

Camera & Camera::operator =(const Camera & camera)
{
  eye = camera.eye;
  fov = camera.fov;
  view = camera.view;

  yaw = camera.yaw;
  pitch = camera.pitch;

  turnAccel = camera.turnAccel;
  turnDecel = camera.turnDecel;
  shiftAccel = camera.shiftAccel;
  shiftDecel = camera.shiftDecel;
  maxTurnSpeed = camera.maxTurnSpeed;
  maxShiftSpeed = camera.maxShiftSpeed;

  turnRLSpeed = 0;
  turnUDSpeed = 0;
  shiftRLSpeed = 0;
  shiftUDSpeed = 0;
  shiftFBSpeed = 0;

  return *this;
}

Camera::~Camera()
{
}

void Camera::proceedControl(const int controlFlags, const int timePassedMs)
{
  float timePassed = timePassedMs / 1000.0f;

  float prevTurnRLSpeed = turnRLSpeed;
  float prevTurnUDSpeed = turnUDSpeed;
  float prevShiftRLSpeed = shiftRLSpeed;
  float prevShiftUDSpeed = shiftUDSpeed;
  float prevShiftFBSpeed = shiftFBSpeed;

  switch (controlFlags & (turnLeftMask | turnRightMask))
  {
  case turnRightMask:
    turnRLSpeed = clamp(turnRLSpeed + turnAccel * timePassed, -maxTurnSpeed, maxTurnSpeed);
    break;
  case turnLeftMask:
    turnRLSpeed = clamp(turnRLSpeed - turnAccel * timePassed, -maxTurnSpeed, maxTurnSpeed);
    break;
  default:
    if (turnRLSpeed < 0.0f)
      turnRLSpeed = min(0.0f, turnRLSpeed + turnDecel * timePassed);
    else if (turnRLSpeed > 0.0f)
      turnRLSpeed = max(0.0f, turnRLSpeed - turnDecel * timePassed);
    break;
  }

  switch (controlFlags & (turnUpMask | turnDownMask))
  {
  case turnUpMask:
    turnUDSpeed = clamp(turnUDSpeed + turnAccel * timePassed, -maxTurnSpeed, maxTurnSpeed);
    break;
  case turnDownMask:
    turnUDSpeed = clamp(turnUDSpeed - turnAccel * timePassed, -maxTurnSpeed, maxTurnSpeed);
    break;
  default:
    if (turnUDSpeed < 0.0f)
      turnUDSpeed = min(0.0f, turnUDSpeed + turnDecel * timePassed);
    else if (turnUDSpeed > 0.0f)
      turnUDSpeed = max(0.0f, turnUDSpeed - turnDecel * timePassed);
    break;
  }

  switch (controlFlags & (shiftLeftMask | shiftRightMask))
  {
  case shiftRightMask:
    if (shiftRLSpeed < 0.0f)
      shiftRLSpeed = clamp(shiftRLSpeed + (shiftDecel + shiftAccel) * timePassed, -maxShiftSpeed, maxShiftSpeed);
    else
      shiftRLSpeed = clamp(shiftRLSpeed + shiftAccel * timePassed, -maxShiftSpeed, maxShiftSpeed);
    break;
  case shiftLeftMask:
    if (shiftRLSpeed > 0.0f)
      shiftRLSpeed = clamp(shiftRLSpeed - (shiftDecel + shiftAccel) * timePassed, -maxShiftSpeed, maxShiftSpeed);
    else
      shiftRLSpeed = clamp(shiftRLSpeed - shiftAccel * timePassed, -maxShiftSpeed, maxShiftSpeed);
    break;
    break;
  default:
    if (shiftRLSpeed < 0.0f)
      shiftRLSpeed = min(0.0f, shiftRLSpeed + shiftDecel * timePassed);
    else if (shiftRLSpeed > 0.0f)
      shiftRLSpeed = max(0.0f, shiftRLSpeed - shiftDecel * timePassed);
    break;
  }

  switch (controlFlags & (shiftUpMask | shiftDownMask))
  {
  case shiftUpMask:
    shiftUDSpeed = clamp(shiftUDSpeed + shiftAccel * timePassed, -maxShiftSpeed, maxShiftSpeed);
    break;
  case shiftDownMask:
    shiftUDSpeed = clamp(shiftUDSpeed - shiftAccel * timePassed, -maxShiftSpeed, maxShiftSpeed);
    break;
  default:
    if (shiftUDSpeed < 0.0f)
      shiftUDSpeed = min(0.0f, shiftUDSpeed + shiftDecel * timePassed);
    else if (shiftUDSpeed > 0.0f)
      shiftUDSpeed = max(0.0f, shiftUDSpeed - shiftDecel * timePassed);
    break;
  }

  switch (controlFlags & (shiftBackMask | shiftForwardMask))
  {
  case shiftForwardMask:
    shiftFBSpeed = clamp(shiftFBSpeed + shiftAccel * timePassed, -maxShiftSpeed, maxShiftSpeed);
    break;
  case shiftBackMask:
    shiftFBSpeed = clamp(shiftFBSpeed - shiftAccel * timePassed, -maxShiftSpeed, maxShiftSpeed);
    break;
  default:
    if (shiftFBSpeed < 0.0f)
      shiftFBSpeed = min(0.0f, shiftFBSpeed + shiftDecel * timePassed);
    else if (shiftFBSpeed > 0.0f)
      shiftFBSpeed = max(0.0f, shiftFBSpeed - shiftDecel * timePassed);
    break;
  }

  float const M_2PI = 2 * M_PI;
  yaw += timePassed * M_2PI * (turnRLSpeed + prevTurnRLSpeed) / 2.0f;
  pitch = clamp(pitch + timePassed * M_2PI * (turnUDSpeed + prevTurnUDSpeed) / 2.0f, float(-0.95f * M_PI_2), float(0.95f * M_PI_2));

  if (yaw >= M_2PI)
    yaw -= M_2PI;
  else if (yaw <= -M_2PI)
    yaw += M_2PI;

  view = Matrix33::makeRotation(yaw, pitch);
  if (fabs(shiftRLSpeed) > FLT_EPSILON ||
    fabs(shiftUDSpeed) > FLT_EPSILON ||
    fabs(shiftFBSpeed) > FLT_EPSILON)
  {

    Vector3 right = view.getCol(0);
    Vector3 up(0.0f, 1.0f, 0.0f);
    Vector3 front = normalize(right % up);

    float midShiftRLSpeed = 0.5f * (shiftRLSpeed + prevShiftRLSpeed);
    float midShiftUDSpeed = 0.5f * (shiftUDSpeed + prevShiftUDSpeed);
    float midShiftFBSpeed = 0.5f * (shiftFBSpeed + prevShiftFBSpeed);

    Vector3 shift =
      midShiftRLSpeed * view.getCol(0) +
      midShiftUDSpeed * up +
      midShiftFBSpeed * front;

    float shiftSqLength = shift.sqLength();

    if (shiftSqLength > Default::maxShiftSpeed * Default::maxShiftSpeed)
      shift = shift * Default::maxShiftSpeed / sqrtf(shiftSqLength);

    eye += shift * timePassed;
  }
}

bool Camera::inMotion() const
{
  return fabs(turnRLSpeed) > FLT_EPSILON ||
         fabs(turnUDSpeed) > FLT_EPSILON ||
         fabs(shiftRLSpeed) > FLT_EPSILON ||
         fabs(shiftUDSpeed) > FLT_EPSILON ||
         fabs(shiftFBSpeed) > FLT_EPSILON;
}

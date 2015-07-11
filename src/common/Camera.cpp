#include "airly.h"
#include "Camera.h"

Camera::Camera()
{

}

Camera::Camera(const Camera & camera) 
{
  eye = camera.eye;
  fov = camera.fov;
  view = camera.view;
  leftQuant = camera.leftQuant;
  rightQuant = camera.rightQuant;
  upQuant = camera.upQuant;
  downQuant = camera.downQuant;
  ccwQuant = camera.ccwQuant;
  cwQuant = camera.cwQuant;
}

Camera & Camera::operator =(const Camera & camera)
{
  eye = camera.eye;
  fov = camera.fov;
  view = camera.view;
  leftQuant = camera.leftQuant;
  rightQuant = camera.rightQuant;
  upQuant = camera.upQuant;
  downQuant = camera.downQuant;
  ccwQuant = camera.ccwQuant;
  cwQuant = camera.cwQuant;

  return *this;
}

Camera::Camera(const Vector3 & eye, const Vector3 & at, const Vector3 & up, const float fov) 
{
  this->eye = eye;
  this->fov = fov;

  Vector3 oz = normalize(at - eye);
  Vector3 ox = normalize(up % oz);
  Vector3 oy = normalize(oz % ox);

  view.setCol(0, ox);
  view.setCol(1, oy);
  view.setCol(2, oz);

  initRotationQuants();
}


Camera::~Camera()
{
}

void Camera::initRotationQuants()
{
  const float Q = float(M_PI) / 1000.0f;
  const float cosQ = cos(Q);
  const float sinQ = sin(Q);
  const float mcosQ = cos(-Q);
  const float msinQ = sin(-Q);

  leftQuant = Matrix33(mcosQ, 0.0f, msinQ,
    0.0f, 1.0f, 0.0f,
    -msinQ, 0.0f, mcosQ);

  rightQuant = Matrix33(cosQ, 0.0f, sinQ,
    0.0f, 1.0f, 0.0f,
    -sinQ, 0.0f, cosQ);

  cwQuant = Matrix33(mcosQ, -msinQ, 0.0f,
    msinQ, mcosQ, 0.0f,
    0.0f, 0.0f, 1.0f);

  ccwQuant = Matrix33(cosQ, -sinQ, 0.0f,
    sinQ, cosQ, 0.0f,
    0.0f, 0.0f, 1.0f);

  upQuant = Matrix33(1.0f, 0.0f, 0.0f,
    0.0f, cosQ, -sinQ,
    0.0f, sinQ, cosQ);

  downQuant = Matrix33(1.0f, 0.0f, 0.0f,
    0.0f, mcosQ, -msinQ,
    0.0f, msinQ, mcosQ);
}

void Camera::rotate(Rotation rot)
{
  switch (rot)
  {
  case crLeft:
    view = view * leftQuant;
    break;
  case crRight:
    view = view * rightQuant;
    break;
  case crUp:
    view = view * upQuant;
    break;
  case crDown:
    view = view * downQuant;
    break;
  case crCwise:
    view = view * cwQuant;
    break;
  case crCcwise:
    view = view * ccwQuant;
    break;
  }
}

void Camera::move(Movement mov)
{
  const float moveStep = 0.01f;

  switch (mov)
  {
  case cmLeft:
    eye -= view.getCol(0) * moveStep;
    break;
  case cmRight:
    eye += view.getCol(0) * moveStep;
    break;
  case cmUp:
    eye += view.getCol(1) * moveStep;
    break;
  case cmDown:
    eye -= view.getCol(1) * moveStep;
    break;
  case cmForward:
    eye += view.getCol(2) * moveStep;
    break;
  case cmBack:
    eye -= view.getCol(2) * moveStep;
    break;
  }
}

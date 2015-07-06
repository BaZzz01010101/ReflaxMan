#include "airly.h"
#include "Vector3.h"
#include "Matrix33.h"

Vector3::Vector3()
{
}

Vector3::Vector3(const Vector3 & v)
{
  x = v.x;
  y = v.y;
  z = v.z;
}

Vector3 & Vector3::operator = (const Vector3 & v)
{
  x = v.x;
  y = v.y;
  z = v.z;

  return *this;
}

Vector3::Vector3(const float x, const float y, const float z)
{
  this->x = x;
  this->y = y;
  this->z = z;
}

Vector3::~Vector3()
{
}

float Vector3::length() const
{
  return sqrtf(x * x + y * y + z * z);
}

float Vector3::sqLength() const
{
  return x * x + y * y + z * z;
}

void Vector3::normalize()
{
  float l = length();
  assert(l > VERY_SMALL_NUMBER);

  if (l > VERY_SMALL_NUMBER)
    *this /= l;
}

Vector3 Vector3::normalized() const
{
  float l = length();
  assert(l > VERY_SMALL_NUMBER);

  if (l > VERY_SMALL_NUMBER)
    return *this / l;
  else
    return *this;
}

Vector3 & Vector3::operator += (const Vector3 & v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

Vector3 & Vector3::operator -= (const Vector3 & v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

Vector3 & Vector3::operator *= (const float f)
{
  x *= f;
  y *= f;
  z *= f;
  return *this;
}

Vector3 & Vector3::operator /= (const float f)
{
  assert(fabs(f) > VERY_SMALL_NUMBER);

  if (fabs(f) > VERY_SMALL_NUMBER)
  {
    x /= f;
    y /= f;
    z /= f;
  }

  return *this;
}

Vector3 Vector3::operator + (const Vector3 & v) const
{
  return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 Vector3::operator - (const Vector3 & v) const
{
  return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3 Vector3::operator * (const float f) const
{
  return Vector3(x * f, y * f, z * f);
}

Vector3 operator * (const float f, const Vector3 & v)
{
  return Vector3(v.x * f, v.y * f, v.z * f);
}

float Vector3::operator * (const Vector3 & v) const
{
  return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::operator * (const Matrix33 & m) const
{
  return Vector3(x * m._11 + y * m._21 + z * m._31,
                 x * m._12 + y * m._22 + z * m._32,
                 x * m._13 + y * m._23 + z * m._33);
}

Vector3 Vector3::operator % (const Vector3 & v) const
{
  return Vector3(y * v.z - z * v.y, 
                 z * v.x - x * v.z, 
                 x * v.y - y * v.x);
}

Vector3 Vector3::operator / (const float f) const
{
  assert(fabs(f) > VERY_SMALL_NUMBER);

  if (fabs(f) > VERY_SMALL_NUMBER)
    return Vector3(x / f, y / f, z / f);
  else
    return *this;
}

bool Vector3::operator == (const Vector3 & v) const
{
  return x == v.x && y == v.y && z == v.z;
}

bool Vector3::operator != (const Vector3 & v) const
{
  return !operator == (v);
}

Vector3 Vector3::operator - () const
{
  return Vector3(-x, -y, -z);
}

Vector3 Vector3::reflect(const Vector3 & norm) const
{
  float a = norm * norm;
  assert(a > VERY_SMALL_NUMBER);

  return 2 * (*this - (*this * norm / a) * norm) - *this;
}

Vector3 Vector3::randomInsideSphere(float radius)
{
  Vector3 v;

  do
  {
    v.x = float(fastrand()) / (float(FAST_RAND_MAX) / 2) - 1.f;
    v.y = float(fastrand()) / (float(FAST_RAND_MAX) / 2) - 1.f;
    v.z = float(fastrand()) / (float(FAST_RAND_MAX) / 2) - 1.f;
  } while (v.sqLength() > 1.f);

  return v * radius;
}

#include <memory.h>
#include "Matrix44.h"
#include "Vector3.h"


Matrix44::Matrix44()
{
}

void Matrix44::transpose()
{
  for (int i = 0; i < 3; i++)
  for (int j = i + 1; j < 4; j++)
  {
    float f = m[i][j];
    m[i][j] = m[j][i];
    m[j][i] = f;
  }
}

Matrix44 Matrix44::transposed() const 
{
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
  {
    result.m[i][j] = m[j][i];
  }

  return result;
}

float Matrix44::det33() const
{
  return
    _11 * (_22 * _33 - _32 * _23) +
    _21 * (_32 * _13 - _12 * _33) +
    _31 * (_12 * _23 - _13 * _22);
}

Vector3 Matrix44::getCol(const int i) const
{
  return Vector3(m[0][i], m[1][i], m[2][i]);
}

void Matrix44::setCol(const int i, const Vector3 & v)
{
  m[0][i] = v.x;
  m[1][i] = v.y;
  m[2][i] = v.z;
}

Matrix44::Matrix44(const Matrix44 & _m)
{
  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    m[i][j] = _m.m[i][j];
}

Matrix44::Matrix44(float f11, float f12, float f13, float f14,
                   float f21, float f22, float f23, float f24,
                   float f31, float f32, float f33, float f34,
                   float f41, float f42, float f43, float f44)
{
  _11 = f11; _12 = f12; _13 = f13; _14 = f14;
  _21 = f21; _22 = f22; _23 = f23; _24 = f24;
  _31 = f31; _32 = f32; _33 = f33; _34 = f34;
  _41 = f41; _42 = f42; _43 = f43; _44 = f44;
}

Matrix44::~Matrix44()
{
}

Matrix44 & Matrix44::operator = (const Matrix44 & _m)
{
  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    m[i][j] = _m.m[i][j];

  return *this;
}

Matrix44 & Matrix44::operator += (const Matrix44 & _m)
{
  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    m[i][j] += _m.m[i][j];

  return *this;
}

Matrix44 & Matrix44::operator -= (const Matrix44 & _m)
{
  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    m[i][j] -= _m.m[i][j];

  return *this;
}

Matrix44 & Matrix44::operator *= (const float f)
{
  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    m[i][j] *= f;

  return *this;
}

Matrix44 & Matrix44::operator /= (const float f)
{
  float inv = 1 / f;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    m[i][j] *= inv;

  return *this;
}

Matrix44 Matrix44::operator + (const Matrix44 & _m) const
{
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    result.m[i][j] = m[i][j] + _m.m[i][j];

  return result;
}

Matrix44 Matrix44::operator - (const Matrix44 & _m) const
{
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    result.m[i][j] = m[i][j] - _m.m[i][j];

  return result;
}

Matrix44 Matrix44::operator * (const float f) const
{
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    result.m[i][j] = m[i][j] * f;

  return result;
}

Matrix44 Matrix44::operator * (const Matrix44 & _m) const
{
  const float(&a)[4][4] = m;
  const float(&b)[4][4] = _m.m;
  Matrix44 ab;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    ab.m[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];

  return ab;
}

Vector3 Matrix44::operator % (const Vector3 & v) const
{
  return Vector3(v.x * _11 + v.y * _12 + v.z * _13,
                 v.x * _21 + v.y * _22 + v.z * _23,
                 v.x * _31 + v.y * _32 + v.z * _33);
}

//Matrix44 Matrix44::operator % (const Matrix44 & _m) const
//{
//  return Matrix44(y * m.z - z * m.y, z * m.x - x * m.z, x * m.y - y * m.x);
//}

Matrix44 Matrix44::operator / (const float f) const
{
  float inv = 1 / f;
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    result.m[i][j] = m[i][j] * inv;

  return result;
}

Matrix44 operator * (const float f, const Matrix44 & _m)
{
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    result.m[i][j] = _m.m[i][j] * f;

  return result;
}

bool Matrix44::operator == (const Matrix44 & _m) const
{
  return !memcmp(&_m, &_11, sizeof(Matrix44));
}

bool Matrix44::operator != (const Matrix44 & _m) const
{
  return !operator == (_m);
}

Matrix44 Matrix44::operator - () const
{
  Matrix44 result;

  for (int i = 0; i < 4; i++)
  for (int j = 0; j < 4; j++)
    result.m[i][j] = -m[i][j];

  return result;
  //return Matrix44(-_11, -_12, -_13, -_14,
  //                -_21, -_22, -_23, -_24,
  //                -_31, -_32, -_33, -_34,
  //                -_41, -_42, -_43, -_44);
}

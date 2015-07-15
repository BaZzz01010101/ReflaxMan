#include "trace_math.h"
#include "Matrix33.h"
#include "Vector3.h"


Matrix33::Matrix33()
{
}

Matrix33::Matrix33(const Vector3 & u, const Vector3 & v, const Vector3 & n)
{
  _11 = u.x; _12 = v.x; _13 = n.x;
  _21 = u.y; _22 = v.y; _23 = n.y;
  _31 = u.z; _32 = v.z; _33 = n.z;
}

Matrix33::Matrix33(
  float f11, float f12, float f13,
  float f21, float f22, float f23,
  float f31, float f32, float f33)
{
  _11 = f11; _12 = f12; _13 = f13;
  _21 = f21; _22 = f22; _23 = f23;
  _31 = f31; _32 = f32; _33 = f33;
}

Matrix33::Matrix33(const Matrix33 & mat)
{
  //for (int i = 0; i < 3; i++)
  //for (int j = 0; j < 3; j++)
  //  m[i][j] = mat.m[i][j];
  memcpy(m, mat.m, sizeof(m));
}

Matrix33 & Matrix33::operator = (const Matrix33 & mat)
{
  //for (int i = 0; i < 3; i++)
  //for (int j = 0; j < 3; j++)
  //  m[i][j] = mat.m[i][j];
  memcpy(m, mat.m, sizeof(m));

  return *this;
}

Matrix33::~Matrix33()
{
}


float Matrix33::det() const
{
  return 
    _11 * (_22 * _33 - _32 * _23) +
    _21 * (_32 * _13 - _12 * _33) +
    _31 * (_12 * _23 - _13 * _22);
}

void Matrix33::invert()
{
  const float d = det();
  assert(fabs(d) > VERY_SMALL_NUMBER);


  if (fabs(d) > VERY_SMALL_NUMBER)
  {
    *this = Matrix33(
      (_22 * _33 - _23 * _32) / d,
      (_13 * _32 - _12 * _33) / d,
      (_12 * _23 - _13 * _22) / d,
      (_23 * _31 - _21 * _33) / d,
      (_11 * _33 - _13 * _31) / d,
      (_13 * _21 - _11 * _23) / d,
      (_21 * _32 - _22 * _31) / d,
      (_12 * _31 - _11 * _32) / d,
      (_11 * _22 - _12 * _21) / d);
  }
  else
    *this = Matrix33(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

Matrix33 Matrix33::inverted() const
{
  const float d = det();
  assert(fabs(d) > VERY_SMALL_NUMBER);

  if (fabs(d) > VERY_SMALL_NUMBER)
    return Matrix33(
      (_22 * _33 - _23 * _32) / d,
      (_13 * _32 - _12 * _33) / d,
      (_12 * _23 - _13 * _22) / d,
      (_23 * _31 - _21 * _33) / d,
      (_11 * _33 - _13 * _31) / d,
      (_13 * _21 - _11 * _23) / d,
      (_21 * _32 - _22 * _31) / d,
      (_12 * _31 - _11 * _32) / d,
      (_11 * _22 - _12 * _21) / d);
  else
    return Matrix33(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

void Matrix33::transpose()
{
  for (int i = 0; i < 2; i++)
  for (int j = i + 1; j < 3; j++)
  {
    const float f = m[i][j];
    m[i][j] = m[j][i];
    m[j][i] = f;
  }
}

Matrix33 Matrix33::transposed() const
{
  Matrix33 result;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
  {
    result.m[i][j] = m[j][i];
  }

  return result;
}

Vector3 Matrix33::getCol(const int i) const
{
  return Vector3(m[0][i], m[1][i], m[2][i]);
}

void Matrix33::setCol(const int i, const Vector3 & v)
{
  m[0][i] = v.x;
  m[1][i] = v.y;
  m[2][i] = v.z;
}

Matrix33 & Matrix33::operator += (const Matrix33 & mat)
{
  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    m[i][j] += mat.m[i][j];

  return *this;
}

Matrix33 & Matrix33::operator -= (const Matrix33 & mat)
{
  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    m[i][j] -= mat.m[i][j];

  return *this;
}

Matrix33 & Matrix33::operator *= (const float f)
{
  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    m[i][j] *= f;

  return *this;
}

Matrix33 & Matrix33::operator /= (const float f)
{
  assert(fabs(f) > VERY_SMALL_NUMBER);

  if (fabs(f) > VERY_SMALL_NUMBER)
  {
    const float inv = 1 / f;

    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      m[i][j] *= inv;
  }
  return *this;
}

Matrix33 Matrix33::operator + (const Matrix33 & mat) const
{
  Matrix33 result;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    result.m[i][j] = m[i][j] + mat.m[i][j];

  return result;
}

Matrix33 Matrix33::operator - (const Matrix33 & mat) const
{
  Matrix33 result;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    result.m[i][j] = m[i][j] - mat.m[i][j];

  return result;
}

Matrix33 Matrix33::operator * (const float f) const
{
  Matrix33 result;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    result.m[i][j] = m[i][j] * f;

  return result;
}

Matrix33 operator * (const float f, const Matrix33 & mat)
{
  return mat * f;
}

Matrix33 Matrix33::operator * (const Matrix33 & mat) const
{
  const float(&a)[3][3] = m;
  const float(&b)[3][3] = mat.m;
  Matrix33 ab;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    ab.m[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j];

  return ab;
}

Vector3 Matrix33::operator * (const Vector3 & v) const
{
  return Vector3(v.x * _11 + v.y * _12 + v.z * _13,
                 v.x * _21 + v.y * _22 + v.z * _23,
                 v.x * _31 + v.y * _32 + v.z * _33);
}

Matrix33 Matrix33::operator / (const float f) const
{
  assert(fabs(f) > VERY_SMALL_NUMBER);

  if (fabs(f) > VERY_SMALL_NUMBER)
  {
    const float inv = 1 / f;
    return *this * inv;
  }
  return *this;
}

bool Matrix33::operator == (const Matrix33 & mat) const
{
  return
    //_11 == mat._11 && _12 == mat._12 && _13 == mat._13 &&
    //_21 == mat._21 && _22 == mat._22 && _23 == mat._23 &&
    //_31 == mat._31 && _32 == mat._32 && _33 == mat._33;
  !memcmp(m, mat.m, sizeof(m));
}

bool Matrix33::operator != (const Matrix33 & mat) const
{
  return !operator == (mat);
}

Matrix33 Matrix33::operator - () const
{
  return Matrix33(-_11, -_12, -_13,
                  -_21, -_22, -_23,
                  -_31, -_32, -_33);
}

Matrix33 Matrix33::makeRotation(const float yaw, const float pitch)
{
  const float yawSin = sin(yaw);
  const float yawCos = cos(yaw);
  const float pitchSin = sin(pitch);
  const float pitchCos = cos(pitch);

  const Vector3 front(yawSin * pitchCos, pitchSin, yawCos * pitchCos);
  const Vector3 right = Vector3(0.0f, 1.0f, 0.0f) % front;
  const Vector3 up = front % right;

  return Matrix33(right.normalized(), up.normalized(), front.normalized());
}

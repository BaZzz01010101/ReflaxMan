#pragma once

class Matrix33;

class Vector3
{
public:
  float x;
  float y;
  float z;

  Vector3();
  Vector3(const float x, const float y, const float z);
  Vector3(const Vector3 & v);
  Vector3 & operator = (const Vector3 & v);
  ~Vector3();

  float length() const;
  float sqLength() const;
  void normalize();
  Vector3 normalized() const;

  Vector3 & operator += (const Vector3 & v);
  Vector3 & operator -= (const Vector3 & v);
  Vector3 & operator *= (const float f);
  Vector3 & operator /= (const float f);

  Vector3 operator + (const Vector3 & v) const;
  Vector3 operator - (const Vector3 & v) const;
  Vector3 operator * (const float f) const;
  friend Vector3 operator * (const float s, const Vector3 & v);
  float operator * (const Vector3 & v) const;
  Vector3 operator * (const Matrix33 & m) const;
  Vector3 operator % (const Vector3 & v) const;
  Vector3 operator / (const float f) const;

  bool operator == (const Vector3 & v) const;
  bool operator != (const Vector3 & v) const;

  Vector3 operator - () const;

  Vector3 reflect(const Vector3 & Norm) const;

  static Vector3 randomInsideSphere(float radius);
};


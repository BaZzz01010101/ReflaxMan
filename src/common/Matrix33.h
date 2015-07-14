#pragma once

class Vector3;

class Matrix33
{
public:
  #pragma pack(push, 1)
  union
  {
    struct
    {
      float _11; float _12; float _13; 
      float _21; float _22; float _23; 
      float _31; float _32; float _33; 
    };
    float m[3][3];
  };
  #pragma pack(pop)

  Matrix33();
  Matrix33(const Vector3 & u, const Vector3 & v, const Vector3 & n);
  Matrix33(float f11, float f12, float f13,
    float f21, float f22, float f23,
    float f31, float f32, float f33);
  Matrix33(const Matrix33 & v);
  Matrix33 & operator = (const Matrix33 & _m);
  ~Matrix33();

  float det() const;
  void transpose();
  void invert();
  Matrix33 transposed() const;
  Matrix33 inverted() const;
  Vector3 getCol(const int i) const;
  void setCol(const int i, const Vector3 & v);

  Matrix33 & operator += (const Matrix33 & m);
  Matrix33 & operator -= (const Matrix33 & m);
  Matrix33 & operator *= (const float f);
  Matrix33 & operator /= (const float f);

  Matrix33 operator + (const Matrix33 & m) const;
  Matrix33 operator - (const Matrix33 & m) const;
  Matrix33 operator * (const float f) const;
  friend Matrix33 operator * (const float f, const Matrix33 & m);
  Vector3 operator * (const Vector3 & v) const;
  Matrix33 operator * (const Matrix33 & m) const;
  Matrix33 operator / (const float f) const;

  bool operator == (const Matrix33 & m) const;
  bool operator != (const Matrix33 & m) const;

  Matrix33 operator - () const;

  static Matrix33 makeRotation(const float yaw, const float pitch);
};



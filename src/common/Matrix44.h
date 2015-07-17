#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

class Vector3;

class Matrix44
{
public:
  union
  {
    struct
    {
      float _11; float _12; float _13; float _14;
      float _21; float _22; float _23; float _24;
      float _31; float _32; float _33; float _34;
      float _41; float _42; float _43; float _44;
    };
    float m[4][4];
  };

  Matrix44();
  Matrix44(const Matrix44 & v);
  Matrix44(float f11, float f12, float f13, float f14,
                     float f21, float f22, float f23, float f24,
                     float f31, float f32, float f33, float f34,
                     float f41, float f42, float f43, float f44);
  ~Matrix44();

  void transpose();
  Matrix44 transposed() const;
  float det33() const ;
  Vector3 getCol(const int i) const;
  void setCol(const int i, const Vector3 & v);

  Matrix44 & operator = (const Matrix44 & _m);

  Matrix44 & operator += (const Matrix44 & m);
  Matrix44 & operator -= (const Matrix44 & m);
  Matrix44 & operator *= (const float f);
  Matrix44 & operator /= (const float f);

  Matrix44 operator + (const Matrix44 & m) const;
  Matrix44 operator - (const Matrix44 & m) const;
  Matrix44 operator * (const float f) const;
  Vector3 operator % (const Vector3 & v) const;
  Matrix44 operator / (const float f) const;

  Matrix44 operator * (const Matrix44 & m) const;
//  Matrix44 operator % (const Matrix44 & v) const;

  friend Matrix44 operator * (const float f, const Matrix44 & m);

  bool operator == (const Matrix44 & m) const;
  bool operator != (const Matrix44 & m) const;

  Matrix44 operator - () const;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

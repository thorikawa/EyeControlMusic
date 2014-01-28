/*****************************************************************************
*   GeometryTypes.cpp
*   Example_MarkerBasedAR
******************************************************************************
*   by Khvedchenia Ievgen, 5th Dec 2012
*   http://computer-vision-talks.com
******************************************************************************
*   Ch2 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/

#include "GeometryTypes.hpp"

Matrix44 Matrix44::getTransposed() const
{
  Matrix44 t;
  
  for (int i=0;i<4; i++)
    for (int j=0;j<4;j++)
      t.mat[i][j] = mat[j][i];
    
  return t;
}

Matrix44 Matrix44::identity()
{
  Matrix44 eye;
  
  for (int i=0;i<4; i++)
    for (int j=0;j<4;j++)
      eye.mat[i][j] = i == j ? 1 : 0;
  
  return eye;
}

Matrix44 Matrix44::getInvertedRT() const
{
  Matrix44 t = identity();
  
  for (int col=0;col<3; col++)
  {
    for (int row=0;row<3;row++)
    { 
      // Transpose rotation component (inversion)
      t.mat[row][col] = mat[col][row];
    }
    
    // Inverse translation component
    t.mat[3][col] = - mat[3][col];
  }
  return t;
}

Vector3 Matrix44::operator*(const Vector3& v) const {
    /*
    float x = mat[0][0]*v.data[0] + mat[0][1]*v.data[1] + mat[0][2]*v.data[2] + mat[0][3];
    float y = mat[1][0]*v.data[0] + mat[1][1]*v.data[1] + mat[1][2]*v.data[2] + mat[1][3];
    float z = mat[2][0]*v.data[0] + mat[2][1]*v.data[1] + mat[2][2]*v.data[2] + mat[2][3];
    float w = mat[3][0]*v.data[0] + mat[3][1]*v.data[1] + mat[3][2]*v.data[2] + mat[3][3];
    */
    float x = mat[0][0]*v.data[0] + mat[1][0]*v.data[1] + mat[2][0]*v.data[2] + mat[3][0];
    float y = mat[0][1]*v.data[0] + mat[1][1]*v.data[1] + mat[2][1]*v.data[2] + mat[3][1];
    float z = mat[0][2]*v.data[0] + mat[1][2]*v.data[1] + mat[2][2]*v.data[2] + mat[3][2];
    float w = mat[0][3]*v.data[0] + mat[1][3]*v.data[1] + mat[2][3]*v.data[2] + mat[3][3];
    printf("w=%f\n", w);

    x /= w;
    y /= w;
    z /= w;
    Vector3 res;
    res.data[0] = x;
    res.data[1] = y;
    res.data[2] = z;
    return res;
}

Matrix33 Matrix33::identity()
{
  Matrix33 eye;
  
  for (int i=0;i<3; i++)
    for (int j=0;j<3;j++)
      eye.mat[i][j] = i == j ? 1 : 0;
  
  return eye;
}

Matrix33 Matrix33::getTransposed() const
{
  Matrix33 t;
  
  for (int i=0;i<3; i++)
    for (int j=0;j<3;j++)
      t.mat[i][j] = mat[j][i];
  
  return t;
}

Vector3 Vector3::zero()
{
  Vector3 v = { 0,0,0 };
  return v;
}

Vector3 Vector3::operator-() const
{
  Vector3 v = { -data[0],-data[1],-data[2] };
  return v;
}

Vector3 Vector3::crossProduct(const Vector3& b) {
    Vector3 v;
    v.data[0] = data[1]*b.data[2] - data[2]*b.data[1];
    v.data[1] = data[2]*b.data[0] - data[0]*b.data[2];
    v.data[2] = data[0]*b.data[1] - data[1]*b.data[0];
    return v;
}

Transformation::Transformation()
: m_rotation(Matrix33::identity())
, m_translation(Vector3::zero())
{
  
}

Transformation::Transformation(const Matrix33& r, const Vector3& t)
: m_rotation(r)
, m_translation(t)
{
  
}

Matrix33& Transformation::r()
{
  return m_rotation;
}

Vector3&  Transformation::t()
{
  return  m_translation;
}

const Matrix33& Transformation::r() const
{
  return m_rotation;
}

const Vector3&  Transformation::t() const
{
  return  m_translation;
}

Matrix44 Transformation::getMat44() const
{
  Matrix44 res = Matrix44::identity();
  
  for (int col=0;col<3;col++)
  {
    for (int row=0;row<3;row++)
    {
      // Copy rotation component
      res.mat[row][col] = m_rotation.mat[row][col];
    }
    
    // Copy translation component
    res.mat[3][col] = m_translation.data[col];
  }
  
  return res;
}

Transformation Transformation::getInverted() const
{
  return Transformation(m_rotation.getTransposed(), -m_translation); 
}
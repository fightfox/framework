#ifndef WORLD2D_PHYSICS_XY
#define WORLD2D_PHYSICS_XY

#include "../std.h"

struct XY {
  double x, y;
  XY() {
    x = 123456789.876;
    y = 987654321.234;
  }
  XY(double x, double y): x(x), y(y) {}
  XY(const XY& p): x(p.x), y(p.y) {}

  inline bool isInfinity() const {
    return (x==123456789.876 && y==987654321.234);
  }
  inline bool isZero() const {
    return (x==0 && y==0);
  }

  inline double distTo(const XY& p) const {
    double dx = x - p.x;
    double dy = y - p.y;
    return sqrt(dx*dx+dy*dy);
  }
  inline bool operator==(const XY& p) const {
    return x==p.x && y==p.y;
  }

  // +, -, *, /
  inline XY operator+(const XY& p) const {
    return XY(x+p.x, y+p.y);
  }
  inline XY operator-(const XY& p) const {
    return XY(x-p.x, y-p.y);
  }
  inline XY operator*(const double f) const {
    return XY(x*f, y*f);
  }
  inline XY operator/(const double f) const {
    return XY(x/f, y/f);
  }
  // +=, -=, *=, /=
  inline XY& operator+=(const XY& p) {
    x += p.x;
    y += p.y;
    return *this;
  }
  inline XY& operator-=(const XY& p) {
    x -= p.x;
    y -= p.y;
    return *this;
  }
  inline XY& operator*=(const double f) {
    x *= f;
    y *= f;
    return *this;
  }
  inline XY& operator/=(const double f) {
    x /= f;
    y /= f;
    return *this;
  }

  // norm and normalize
  inline double norm() const {
    return sqrt(x*x+y*y);
  }
  inline void normalize() {
    double z = norm();
    if (z < EPSILON) {
      x = 1;
      y = 0;
    }
    else {
      x /= z;
      y /= z;
    }
  }
  inline XY normalized() const {
    double z = norm();
    return z < EPSILON ? XY(1,0) : XY(x/z, y/z);
  }
  inline XY rotate(double angle) const {
    return XY(x*cos(angle)-y*sin(angle), x*sin(angle)+y*cos(angle));
  }
  inline double normSqr() const {
    return x*x+y*y;
  }
  inline double dot(const XY& p) const {
    return x*p.x+y*p.y;
  }
};

#endif // WORLD2D_PHYSICS_XY

#ifndef WORLD2D_PHYSICS_BB
#define WORLD2D_PHYSICS_BB

#include "../std.h"
#include "XY.h"

struct BB {
  double x1, y1, x2, y2;
  BB(double x1, double y1, double x2, double y2, int id = 0) 
  : x1(x1), y1(y1), x2(x2), y2(y2), id(id) {}
  BB(){
    id = -1;
  }
  uint32_t id;  // refer to shape's id 
  inline void setId(uint32_t id_) {id = id_;}
  inline bool intersects(const BB& bb) const {
    return x2>bb.x1 && y2>bb.y1 && x1<bb.x2 && y1<bb.y2; 
  }
  inline bool contains(const BB& bb) const {
    return x1<=bb.x1 && y1<=bb.y1 && x2>=bb.x2 && y2>=bb.y2; 
  }
  inline BB operator|(const BB& bb) const {
    return BB(min(x1,bb.x1), min(y1,bb.y1), max(x2,bb.x2), max(y2,bb.y2));
  }
  inline BB operator+(const XY v) const {
    return BB(x1+v.x, y1+v.y, x2+v.x, y2+v.y);
  }
  inline BB operator+(const double z) const {
    return BB(x1-z, y1-z, x2+z, y2+z);
  }
  inline BB operator-(double z) const {
    z = x2-x1<z*2 ? 0 : (y2-y1<z*2 ? 0 : z);
    return BB(x1+z, y1+z, x2-z, y2-z);
  }
  inline vector<BB> operator-(const BB& bb) const {
    // auto aa = *this;
    vector<BB> v;
    // TODO
    return v;
  }
  inline bool valid() const {
    return id!=-1;
  }
};

inline bool intersect(const BB& a, const BB& b) {
  return a.x2>b.x1 && a.y2>b.y1 && a.x1<b.x2 && a.y1<b.y2; 
}

#endif // WORLD2D_PHYSICS_BB

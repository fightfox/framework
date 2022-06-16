#ifndef WORLD2D_PHYSICS_LINE
#define WORLD2D_PHYSICS_LINE

#include "../std.h"
#include "XY.h"

struct Line {
  XY p, v;
  Line(XY p, XY v): p(p), v(v) {}
  inline pair<Time, Time> getCrashTime(const Line& l) const {
    XY d = p-l.p;
    double a = v.x*l.v.y-v.y*l.v.x;
    double b = d.y*l.v.x-d.x*l.v.y;
    double c = d.y*v.x-d.x*v.y;
    return abs(a)>EPSILON ? make_pair(b/a,c/a) : make_pair(INVALID_TIME,INVALID_TIME);
  }
  
};

#endif // WORLD2D_PHYSICS_LINE

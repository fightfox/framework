#ifndef WORLD2D_PHYSICS_STROKE
#define WORLD2D_PHYSICS_STROKE

#include "XY.h"

struct Stroke {
	enum Type {
		LINE,
		ARCH
	}	type;
	XY p0, p1;
	double r, theta;

	uint32_t id;
	// if it is line, it is p0 to p1; 
	// otherwise, p0 is circle center, p1 is mid-point, theta is the angle 
	Stroke(XY p0, XY p1): type(LINE), p0(p0), p1(p1) {}
	Stroke(XY p0, XY p1, double theta): p0(p0), p1(p1), theta(theta) {
		r = p0.distTo(p1);
	} 
	void setId(uint32_t id_) {
		id = id_;
	}
	// 	# {vector<XY>}getIntersectionByStroke(Stroke&)
};

#endif // WORLD2D_PHYSICS_STROKE
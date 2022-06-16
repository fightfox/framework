#ifndef WORLD2D_FIGURE
#define WORLD2D_FIGURE

#include "XY.h"
#include "BB.h"
#include <math.h>
#include <utility>

struct Figure {
  enum Type : uint8_t {
    CIRCLE,
    SEGMENT
    // SEGMENT, // TODO
    // POLYGON
  } type;
  XY p;         // central position of the figure
  double r;      // radius (the figure is expanded by radius r)

  Figure(Type type, XY p, double r): type(type), p(p), r(r) {}

  virtual bool checkCollision(Figure*) = 0;
  virtual bool mayCollide(Figure*) = 0;
  virtual BB getBB() const = 0;
  virtual double area() = 0;  // used to compute mass
  struct Circle;
  struct Segment;

  uint32_t id;   // id might be useful later
  void setId(uint32_t id_) {
    id = id_;
  }
  inline uint32_t getId() {
    return id;
  }

  virtual XY nVectorTo(Figure*) = 0; 
  virtual ~Figure() {}
  
private:
  inline bool ifCircleIntersetBB(XY p, double r, BB bb) {
    double m_x = (bb.x1 + bb.x2) / 2;
    double m_y = (bb.y1 + bb.y2) / 2;
    double h_w = bb.x2 - m_x;
    double h_h = bb.y2 - m_y;
    double l_x = abs(p.x - m_x);
    double l_y = abs(p.y - m_y);
    return l_x < h_w+r && l_y < h_h+r && 
          (l_x<h_w || l_y<h_h || sqr(l_x-h_w)+sqr(l_y-h_h)<sqr(r));
  }
};

typedef unique_ptr<Figure> Figure_uptr;
typedef shared_ptr<Figure> Figure_ptr;

//==============================================================================

struct Figure::Circle : Figure {
  Circle(XY p, double r): Figure(Type::CIRCLE, p, r) {}
  double area() override {
    return PI * r * r;
  }
  bool checkCollision(Figure* figure) override {
    if (figure->type == CIRCLE)
      return (p-figure->p).norm() < r+figure->r;
    else if (figure->type == SEGMENT)
      return figure->checkCollision(this);
    return false;
  }
  bool mayCollide(Figure* figure) override {
    if (figure->type == CIRCLE) {
      double len = (p-figure->p).norm();
      return (len<r+figure->r+EPSILON_LENGTH && len>=r+figure->r);
    }
    else if (figure->type == SEGMENT)
      return figure->mayCollide(this);
    return false;
  }
  BB getBB() const override {
    return BB(p.x-r, p.y-r, p.x+r, p.y+r);
  }
  
  XY nVectorTo(Figure* f) override {
    if (f->type==Type::CIRCLE) {
      return (f->p-p).normalized(); 
    }
    else if (f->type==SEGMENT) {
      return (f->nVectorTo(this))*(-1);
    }
    return XY();
  }
};


struct Figure::Segment : Figure {
  XY d_p, t_v, n_v;
  double half_len;
  Segment(XY p0, XY p1, double r): Figure(Type::SEGMENT, (p0+p1)/2, r) {
    d_p = p0-p;
    t_v = d_p.normalized();
    n_v = t_v.rotate(PI/2);
    half_len = d_p.norm();
  }
  double area() override {
    return PI * r * r + 2 * half_len * r;
  }
  bool checkCollision(Figure* figure) override {
    if (figure->type == CIRCLE) {
      XY delta = figure->p-p;
      double t = delta.dot(t_v);
      if (abs(t) < half_len)
        return abs(n_v.dot(delta)) < r+figure->r;
      else if (t>0)
        return (delta-d_p).norm() < r+figure->r;
      else
        return (delta+d_p).norm() < r+figure->r;
    }
    else if (figure->type == SEGMENT) {
      //TODO assume that segment will not influence other segment
      return false;
    }
    return false;
  }
  bool mayCollide(Figure* figure) override {
    if (figure->type == CIRCLE) {
      XY delta = figure->p-p;
      double t = delta.dot(t_v);
      double len;
      if (abs(t) <= half_len+EPSILON)
        len = abs(n_v.dot(delta));
      else if (t>0)
        len = (delta-d_p).norm();
      else
        len = (delta+d_p).norm();
      return (len<r+figure->r+EPSILON_LENGTH && len>=r+figure->r);
    }
    else if (figure->type == SEGMENT) {
      //TODO assume that segment will not influence other segment
      return false;
    }
    return false;
  }
  BB getBB() const override { 
    auto bb = BB(p.x+d_p.x-r, p.y+d_p.y-r, p.x+d_p.x+r, p.y+d_p.y+r)
      | BB(p.x-d_p.x-r, p.y-d_p.y-r, p.x-d_p.x+r, p.y-d_p.y+r);
    return bb;
  }

  XY nVectorTo(Figure* f) override {
    if (f->type==Type::CIRCLE) {
      XY delta = f->p-p;
      double t = delta.dot(t_v);
      if (abs(t)<=half_len+EPSILON) {
        if (n_v.dot(delta)>0)
          return n_v;
        else
          return n_v*(-1);
      }
      else if (t>0) {
        return (delta-d_p).normalized();
      }
      else {
        return (delta+d_p).normalized();
      }
    }
    else if (f->type==Type::SEGMENT) {
      assert(false && "Segment collides segment");
      return XY(0,0);
    }
    return XY();
  }

};

#endif // WORLD2D_FIGURE



/*


struct Figure::Box : Figure {
  double half_height, half_width;
  Box(XY p, double half_height, double half_width, double r = 0): 
    Figure(Type::BOX, p, r), half_height(half_height), half_width(half_width) {}
  double area() override {
    return half_height * half_width * 4;
  }
  bool checkCollision(Figure* figure) override {
    if (figure->type == CIRCLE)
      return ifCircleIntersetBB(figure->p, figure->r, getBB());
    else if (figure->type == Type::BOX)
      return getBB().intersects(figure->getBB());
    return false;
  }
  BB getBB() const override {
    return BB(p.x-half_width, p.y-half_height, 
              p.x+half_width, p.y+half_height);
  }
  XY nVectorTo(Figure* f) override {
    //TODO
    return XY(0,0);
  }
};

*/
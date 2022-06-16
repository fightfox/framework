#include "Trace.h"

void test_get_bb() {
  Figure_uptr circle = make_unique<Figure::Circle>(XY(10,20),7);
  Time t(1000000);
  Trace_ptr trace = make_unique<Trace>(std::move(circle), XY(1.3, 1.7), t);
  BB bb = trace->getBB(t+40);
  assert(abs(bb.x1-3)<=1e-6);
  assert(abs(bb.y1-13)<=1e-6);
  assert(abs(bb.x2-69)<=1e-6);
  assert(abs(bb.y2-95)<=1e-6);
}

void test_compute_collistion() {
  Time t(1000000);
  // create a circle whose center is (0, 20) and radius 7
  // circle is managed by a unique_ptr
  Figure_uptr circle = make_unique<Figure::Circle>(XY(0,20),7);
  // create a trace with velocity (1,0), the trace is circle at time t 
  Trace_ptr trace1 = make_unique<Trace>(std::move(circle), XY(1, 0), t);
  // after moving circle into trace's constructor, circle's pointer is empty
  // we reuse the Figure_uptr to hold a new circle
  circle = make_unique<Figure::Circle>(XY(10,0),4);
  Trace_ptr trace2 = make_unique<Trace>(std::move(circle), XY(0, 1), t+1);
  assert(trace2->computeCollision(trace1)-t==10);
  // now we make trace1 move way faster
  trace1->v.x = 10;
  assert(trace2->computeCollision(trace1)==-1);
  // now we make trace1 and trace2 already overlap
  trace2->figure->p.y = 19;
  assert(trace2->computeCollision(trace1)==t+1);

  auto f2 = trace1->getFigure(t+1000);
  assert(f2->p.x == 10000);
  assert(f2->p.y == 20);
}

int main() {
  test_get_bb();
  test_compute_collistion();
}
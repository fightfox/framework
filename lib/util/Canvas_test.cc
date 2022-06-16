#include "Canvas.h"

void test_basic() {
  // constructor
  Canvas cv;
  // insert
  cv[1] = BB(0, 0, 10, 10);
  cv[2] = BB(10, 10, 20, 20);
  // get
  BB* bb = cv.get(2);
  assert(bb->x1==10 && bb->y1==10 && bb->x2==20 && bb->y2==20 && bb->id==2);
  // modify
  cv[2] = BB(20, 20, 30, 30);;
  assert(cv.get(2)->x1==20 && cv.get(2)->y2==30);
  // foreach
  cv.foreach(BB(22, 22, 23, 24), [](uint32_t id) {
    assert(id == 2);
  });
  // remove
  cv[1] = nullptr;
}

int main() {
  test_basic();
}
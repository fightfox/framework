#include "Line.h"

void test_intersect() {
  Line a(XY(0,0), XY(1,0));
  Line b(XY(10,10), XY(0,2));
  auto ans = a.getCrashTime(b);
  ASSERT(abs(ans.first-10)<EPSILON, "test_intersect");
  ASSERT(abs(ans.second-(-5))<EPSILON, "test_intersect");
}

void test_intersect2() {
  Line a(XY(566, 180), XY(0, 0.2));
  Line b(XY(566, 180), XY(0, 0.2));
  auto ans = a.getCrashTime(b);
  cout << ans.first << " " << ans.second << endl;
  cout << a.p+a.v*ans.first << endl;
  cout << b.p+b.v*ans.second << endl;
}


void test_intersect3() {
  Line a(XY(0, 0), XY(1, 0.42332));
  Line b(XY(10.3243, 1), XY(0, 2));
  auto ans = a.getCrashTime(b);
  ASSERT(((a.p+a.v*ans.first)-(b.p+b.v*ans.second)).norm()<EPSILON, 
        "test_intersect3");
}


int main() {
  test_intersect2();
}
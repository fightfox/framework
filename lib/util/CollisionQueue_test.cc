#include "CollisionQueue.h"

int main() {
  CollisionQueue cq;
  cq.insert(100, 1, 2);
  cq.insert(200, 3, 2);
  cq.insert(50, 3, 1);
  cout << cq.earliestTime() << endl;
  cout << cq.consume()->u1 << endl;
  cout << cq.earliestTime() << endl;
}
#include "CooldownQueue.h"

enum Cooldown::Type: uint8_t {
  TEST
};

struct TestCooldown : Cooldown {
  TestCooldown(uint32_t id, Time time): Cooldown(TEST, id, time) {}
  void handle() override {}
};

void test_basic() {
  // construct
  CooldownQueue q;
  // insert
  uint32_t cd1 = q.insert(new TestCooldown(1, 1000));
  uint32_t cd2 = q.insert(new TestCooldown(2, 200));
  uint32_t cd3 = q.insert(new TestCooldown(3, 400));
  // earliestTime
  Time time = q.earliestTime();
  assert(time == 200);
  // remove
  q.remove(cd2);
  // consume
  unique_ptr<Cooldown> cd = q.consume();
  assert(cd->id==3 && cd->time==400);
  // modify
  q.modify(cd1, 300);
  assert(q.earliestTime()==300);
  assert(q.consume()->time==300);
  assert(!q.modify(cd3, 3333));
  // in the end of the test, check empty
  assert(q.earliestTime()>1e10);
}

int main() {
  test_basic();
}
#include "EventQueue.h"

int main() {
  EventQueue eq;
  auto e1 = new Event(Event::INITIALIZATION, 200, nullptr);
  auto e2 = new Event(Event::INITIALIZATION, 100, nullptr);
  auto e3 = new Event(Event::INITIALIZATION, 400, nullptr);
  eq.insert(e1);
  eq.insert(e2);
  eq.insert(e3);
  cout << eq.earliestTime() << endl;
  eq.remove(e2);
  cout << eq.earliestTime() << endl;
  eq.remove(e1);
  cout << eq.earliestTime() << endl;
}
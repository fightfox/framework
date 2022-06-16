#include "EventQueue.h"

void EventQueue::insert(Event* evt) {
    h.insert(evt);
  }
unique_ptr<Event> EventQueue::consume() {
  auto ans = unique_ptr<Event>(h.top());
  h.remove(0);
  return std::move(ans);
}
Time EventQueue::earliestTime() {
  return h.v.size() ? h.top()->time : 1LL<<62;
}
void EventQueue::remove(Event* evt) {
  h.remove(evt->heap_id);
}
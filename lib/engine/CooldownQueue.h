#ifndef WORLD2D_COOLDOWN_QUEUE
#define WORLD2D_COOLDOWN_QUEUE

#include "../std.h"
#include "../util/heap.h"

struct Cooldown {
  uint32_t id;
  Time time;
  uint32_t heap_id;
  uint32_t q_id;
  Cooldown(uint32_t id, Time time): id(id), time(time) {}
  Cooldown(Time time) : id(INVALID_ID), time(time) {}
  virtual ~Cooldown() {}
  virtual void handle() = 0;
};

struct CooldownQueue {
  heap<Cooldown> h;
  dense_hash_map<uint32_t, Cooldown*> cds;
  uint32_t last_cooldown_id;
  CooldownQueue() : last_cooldown_id(0) {
    cds.set_empty_key(-1);
    cds.set_deleted_key(-2);
  }
  uint32_t insert(Cooldown* cd) {
    h.insert(cd);
    cds[cd->q_id = ++last_cooldown_id] = cd;
    return last_cooldown_id;
  }
  unique_ptr<Cooldown> consume() {
    auto ans = unique_ptr<Cooldown>(h.top());
    h.remove(0);
    cds.erase(ans->q_id);
    return std::move(ans);
  }
  inline Time earliestTime() {
    return h.v.size() ? h.top()->time : 1LL<<62;
  }
  void remove(uint32_t q_id) {
    if (cds.count(q_id)) {
      auto cd = cds[q_id];
      h.remove(cd->heap_id);
      cds.erase(q_id);
      delete cd;
    }
  }
  bool modify(uint32_t q_id, Time new_time) {
    if (cds.count(q_id)) {
      auto cd = cds[q_id];
      cd->time = new_time;
      h.swim(cd->heap_id);
      h.sink(cd->heap_id);
      return true;
    }
    return false;
  }
};

/*
uint32_t cd_id = cooldown_queue.insert(new MyCooldown(...));
cooldown_queue.remove(cd_id);   // delete
Time t = cooldown_queue.earliestTime();
auto cd = unique_ptr<Cooldown> consume()  // delete


*/

#endif // WORLD2D_COOLDOWN_QUEUE
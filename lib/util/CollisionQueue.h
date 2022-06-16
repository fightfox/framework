#ifndef WOLRD2D_COLLISION_QUEUE
#define WOLRD2D_COLLISION_QUEUE

#include "../std.h"
#include "heap.h"

struct CollisionEvent {
  Time time;
  uint32_t u1, u2;
  bool colliding;
  uint32_t heap_id;
  CollisionEvent(Time time, uint32_t u1, uint32_t u2, bool bl): 
    time(time), u1(u1), u2(u2), colliding(bl) {}
};

struct CollisionQueue {
  heap<CollisionEvent> h;
  dense_hash_map<uint32_t, dense_hash_map<uint32_t, CollisionEvent*>> u[2];

  CollisionQueue() {
    for (int i = 0; i < 2; ++i ) {
      u[i].set_empty_key(-1);
      u[i].set_deleted_key(-2);
    }
  }
  // TODO: make it faster by using find.
  void insert(Time time, uint32_t u1, uint32_t u2, bool bl) {
    ASSERT(u1!=u2, "u1 and u2 should not same insert in CollisionQueue");
    if (u[bl].count(u1) && u[bl][u1].count(u2)) return;
    auto evt = new CollisionEvent(time, u1, u2, bl);
    h.insert(evt);
    if (!u[bl].count(u1)) {
      auto& mp = u[bl][u1];
      mp.set_empty_key(-1);
      mp.set_deleted_key(-2);
    }
    u[bl][u1][u2] = evt;
    if (!u[bl].count(u2)) {
      auto& mp = u[bl][u2];
      mp.set_empty_key(-1);
      mp.set_deleted_key(-2);
    }
    u[bl][u2][u1] = evt;
  }
  unique_ptr<CollisionEvent> consume() {
    auto ans = unique_ptr<CollisionEvent>(h.top());
    h.remove(0);
    auto u1 = ans->u1, u2 = ans->u2;
    bool bl = ans->colliding; 
    u[bl][u1].erase(u2);
    u[bl][u2].erase(u1);
    return std::move(ans);
  }
  inline Time earliestTime() {
    return h.v.size() ? h.top()->time : 1LL<<62;
  }
  void remove(uint32_t unit) {
    for (int bl = 0; bl < 2; ++bl) {
      if (!u[bl].count(unit)) continue;
      for (auto pr : u[bl][unit]) {
        auto u2 = pr.first;
        auto evt = pr.second;
        h.remove(evt->heap_id);
        u[bl][u2].erase(unit);
        delete evt;
      }
      u[bl].erase(unit);
    }
  }
};

#endif // WOLRD2D_COLLISION_QUEUE
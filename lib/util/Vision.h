#ifndef CANVAS_VISION
#define CANVAS_VISION

#include "BB.h"
#include "QuadTree.h"

struct Vision {
  dense_hash_map<uint32_t, BB> bbs;
  QuadTree qt;

  Vision(): qt(X_MIN, Y_MIN, X_MAX, Y_MAX) {
    bbs.set_empty_key(-1);
    bbs.set_deleted_key(-2);
  }

  struct IdHolder {
    Vision* vs; 
    uint32_t id;
    IdHolder(Vision* vs, uint32_t id): vs(vs), id(id) {}
    inline void operator=(const BB& b) {
      if (vs->bbs.count(id)) 
        vs->qt.remove(id);
      BB bb(b);
      bb.setId(id);
      vs->bbs[id] = bb;
      vs->qt.insert(bb);
    }
    inline void operator=(std::nullptr_t) {
      if (vs->bbs.count(id)) {
        vs->qt.remove(id);
        vs->bbs.erase(id);
      }
    }
  };
  inline IdHolder operator[](uint32_t id) {
    return IdHolder(this, id);
  }
  inline BB get(const uint32_t& player_id) {
    if (bbs.count(player_id)==0)
      return BB();
    else
      return bbs[player_id];
  }
  void foreach(const BB& bb, const function<void(uint32_t)>& fn) {
    qt.foreach(bb, [&bb, &fn, this](const BB& b){
      fn(b.id);
    });
  }
};

#endif // CANVAS_VISION

#ifndef WORLD2D_CANVAS
#define WORLD2D_CANVAS

#include "QuadTree.h"

struct Canvas {
  dense_hash_map<uint32_t, BB*> bbs;
  QuadTree qt;

  struct IdHolder {
    Canvas* cv; 
    uint32_t id;
    IdHolder(Canvas* cv, uint32_t id): cv(cv), id(id) {}
    inline void operator=(const BB& bb) {
      if (cv->bbs.count(id)) {
        delete cv->bbs[id];
        cv->qt.remove(id);
      } 
      BB* new_bb = new BB(bb);      
      new_bb->setId(id);
      cv->qt.insert(*new_bb);
      cv->bbs[id] = new_bb;
    }
    inline void operator=(std::nullptr_t) {
      if (cv->bbs.count(id)) {
        cv->qt.remove(id);
        delete cv->bbs[id];
        cv->bbs.erase(id);
      }
    }
  }; // end of IdHolder

  Canvas(): qt(X_MIN, Y_MIN, X_MAX, Y_MAX) {
    bbs.set_empty_key(-1);
    bbs.set_deleted_key(-2);
  }
  inline IdHolder operator[](uint32_t id) {
    return IdHolder(this, id);
  }
  inline BB get(uint32_t id) {
    if (bbs.count(id)) return *(bbs[id]);
    else return BB();
  }
  inline void foreach(const BB& bb, const function<void(uint32_t)>& fn) {
    vector<uint32_t> v;
    qt.getAllIdIntersectBB(bb, v);
    for (auto id: v) {
      fn(id);    
    }
  }
};

#endif // WORLD2D_CANVAS
#ifndef WORLD2D_ANIMATION
#define WORLD2D_ANIMATION

#include "Figure.h"
#include "Trace.h"
#include "QuadTree.h"

// Animation does not take any ownership of trace/figure
// It simply store raw pointers of trace/figure
// It is required that the pointers are valid during it's in Animation
struct Animation {
  dense_hash_map<uint32_t, Trace*> traces;
  Time timeUntil;
  QuadTree qt;
  
  Animation(): qt(X_MIN, Y_MIN, X_MAX, Y_MAX) {
    traces.set_empty_key(-1);
    traces.set_deleted_key(-2);
    timeUntil = 0;
  }
  void setUntil(Time t) {
    timeUntil = t;
  }

  struct IdHolder {
    Animation* am; 
    uint32_t id;
    IdHolder(Animation* am, uint32_t id): am(am), id(id) {}
    inline void operator=(Trace* tr) {
      Trace* old_trace = am->traces[id];
      if (old_trace != nullptr) {
        am->qt.remove(id);
        if (old_trace != tr) delete old_trace;
      }
      BB bb = tr->getBB(am->timeUntil);
      bb.setId(id);
      am->traces[id] = tr;
      am->qt.insert(bb);
    }
    inline void operator=(std::nullptr_t) {
      if (am->traces.count(id)) {
        am->qt.remove(id);
        delete am->traces[id];
        am->traces.erase(id);
      }
    }
    inline void operator=(Trace& tr) {
      operator=(&tr);
    }
    inline void operator=(const unique_ptr<Trace>& tr) {
      operator=(tr.get());
    }
    inline void operator=(const shared_ptr<Trace>& tr) {
      operator=(tr.get());
    }
  };

  inline IdHolder operator[](uint32_t id) {
    return IdHolder(this, id);
  }
  inline Trace* get(uint32_t id) {
    return traces[id];
  }
  // TODO: self collide self
  // void foreach(uint32_t id, const function<void(uint32_t, Time)>& fn) const {
  //   if (traces.count(id)) foreach(traces.at(id), fn);
  // }
  // void foreach(Figure* f, const function<void(uint32_t, Time)>& fn) const {
  //   vector<uint32_t> v;
  //   qt.getAllIdIntersectBB(f->getBB(), v);
  //   for (auto id: v) {
  //     Time t = traces.at(id)->computeCollision(*f);
  //     if (t!=-1 && t<=timeUntil) fn(id, t);
  //   }
  // }
  Trace* transfer(uint32_t id) {
    if (traces.count(id)) {
      auto ans = traces[id];
      traces.erase(id);
      qt.remove(id);
      return ans;
    }
    return nullptr;
  }

  void foreach(BB bb, const function<void(uint32_t)>& fn) const {
    vector<uint32_t> v;
    qt.getAllIdIntersectBB(bb, v);
    for (auto id: v) {
      fn(id);    
    }
  }
  // void foreach(Figure* f, Time time, 
  //   const function<void(uint32_t)>& fn) const {
  //   vector<uint32_t> v;
  //   qt.getAllIdIntersectBB(f->getBB(), v);
  //   for (auto id: v) {
  //     if (traces.at(id)->getFigure(time)->checkCollision(f)) fn(id);
  //   }
  // }
};

#endif // WORLD2D_ANIMATION
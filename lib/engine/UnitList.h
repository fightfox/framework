#ifndef WORLD2D_UNIT_LIST
#define WORLD2D_UNIT_LIST

#include "../std.h"
#include "../util/XY.h"
#include "Unit.h"

struct UnitList {
  uint32_t unit_last_id;
  int32_t dying_count;
  dense_hash_map<uint32_t, Unit*> units;
  unordered_set<uint32_t> move_units;
  UnitList() : unit_last_id(0), dying_count(0) {
    units.set_empty_key(-1);
    units.set_deleted_key(-2);
  }
  inline void insert(Unit* u) {
    //TODO CHANGE: allocate u->id
    u->id = (unit_last_id+=2);
    units[u->id] = u;
    if (u->v.normSqr()>EPSILONSQR)
      move_units.insert(u->id);
  }
  inline Unit* get(uint32_t id) {
    if (units.count(id)!=0)
      return units[id];
    else
      return nullptr; 
  }
  inline bool remove(uint32_t id) {
    if (units.count(id)!=0) {
      delete units[id];
      units.erase(id);
      move_units.erase(id);
      dying_count -= 1;
      return true;
    }
    return false;
  }
  inline void enmove(uint32_t id) {
    if (units.count(id)!=0)
      move_units.insert(id);
  }
  inline void dismove(uint32_t id) {
    if (units.count(id)!=0)
      move_units.erase(id);
  }
  void foreachmove(const function<void(uint32_t)>& fn) {
    vector<uint32_t> v;
    for (auto i: move_units)
      v.push_back(i);
    for (auto i: v)
      fn(i);
  }
  void foreach(const function<void(uint32_t)>& fn) {
    for (auto& pr: units)
      if (pr.second->alive) 
        fn(pr.first);
  }
};

#endif // WORLD2D_UNIT_LIST
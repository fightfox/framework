#ifndef WORLD2D_CAMERA
#define WORLD2D_CAMERA

#include "../std.h"
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>

struct Camera {
  google::dense_hash_map<uint32_t, google::dense_hash_set<uint32_t>> players;
  google::dense_hash_map<uint32_t, google::dense_hash_set<uint32_t>> units;
  Camera() {
    players.set_empty_key(-1);
    players.set_deleted_key(-2);
    units.set_empty_key(-1);
    units.set_deleted_key(-2);
  }
  inline void insert(uint32_t player, uint32_t unit) {
    if (!players.count(player)) {
      players[player].set_empty_key(-1);
      players[player].set_deleted_key(-2);
    }
    if (!units.count(unit)) {
      units[unit].set_empty_key(-1);
      units[unit].set_deleted_key(-2);
    }

    players[player].insert(unit);
    units[unit].insert(player);
  }
  inline void remove(uint32_t player, uint32_t unit) {
    players[player].erase(unit);
    units[unit].erase(player);
  }
  inline void remove(uint32_t id) {
    if (id&1) {
      for (auto unit : players[id])
        units[unit].erase(id);
      players.erase(id);
    }
    else {
      for (auto player : units[id])
        players[player].erase(id);
      units.erase(id);
    }
  }
  inline void foreach(uint32_t id, const function<void(uint32_t)>& fn) {
    if (players.count(id)) foreachUnit(id, fn);
    if (units.count(id)) foreachPlayer(id, fn);
  }
  inline bool checkConnection(uint32_t unit_id, uint32_t player_id) {
    return players.count(player_id) ? players[player_id].count(unit_id) : false;
  } 
private:
  void foreachPlayer(uint32_t id, const function<void(uint32_t)>& fn) {
    vector<uint32_t> v;
    for (auto i: units[id]) 
      v.push_back(i);
    for (auto i: v)
      fn(i);
  }
  void foreachUnit(uint32_t id, const function<void(uint32_t)>& fn) {
    vector<uint32_t> v;
    for (auto i: players[id])
      v.push_back(i);
    for (auto i: v)
      fn(i);
  }
};

#endif // WORLD2D_CAMERA
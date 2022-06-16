#ifndef WORLD2D_PLAYER_LIST
#define WORLD2D_PLAYER_LIST

#include "../std.h"
#include "Player.h"

struct PlayerList {
  dense_hash_map<uint32_t, Player*> players;
  uint32_t last_player_id;
  int32_t dying_count; 
  PlayerList() : last_player_id(-1), dying_count(0) {
    players.set_empty_key(-1);
    players.set_deleted_key(-2);
  }
  inline void insert(Player* p) {
    players[p->id] = p;
  }
  inline Player* get(uint32_t id) {
    if (players.count(id)!=0)
      return players[id];
    else
      return nullptr; 
  }
  inline void remove(uint32_t id) {
    if (players.count(id)) {
      players.erase(id);
      dying_count -= 1;  
    }
  }
  inline void foreach(const function<void(uint32_t)>& fn) {
    for (auto& pr: players)
      if (pr.second->alive)
        fn(pr.first);
  }
};

#endif // WORLD2D_PLAYER_LIST
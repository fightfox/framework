#ifndef WORLD2D_VIEW
#define WORLD2D_VIEW

#include "../std.h"
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>
// http://goog-sparsehash.sourceforge.net/doc/dense_hash_map.html

struct Sensor {
  static const uint32_t PRIV_BIT_NUM = 2; // 2
  static const uint32_t PRIV_MAX = 4;
  static const uint32_t PRIV_MASK = (1 << PRIV_BIT_NUM) - 1;
  uint32_t id;  // require: id < 2^{32-PRIV_BIT_NUM}
  uint8_t priv;
  uint32_t hash;
  Sensor(uint32_t id_, uint8_t priv_): id(id_), priv(priv_) {
    hash = (id_ << PRIV_BIT_NUM) | priv_;
  }
  Sensor(uint32_t hash_): id(hash_>>PRIV_BIT_NUM), priv(hash_ & PRIV_MASK), 
    hash(hash_) {}
  inline void setPriv(uint8_t priv_) {
    priv = priv_;
    hash = (id << PRIV_BIT_NUM) | priv;
  }
};

inline uint32_t getHash(uint32_t id, uint8_t priv) {
  return (id << Sensor::PRIV_BIT_NUM) | priv;
}

// TODO, [1..PRIV]
// TODO google dense hash init performance

struct SensorRecord {
  uint8_t max_priv; 
                        /*sensed*/  /*cnt*/ 
  google::dense_hash_map<uint32_t, uint32_t> reachable[Sensor::PRIV_MAX];

                         /*sensor*/  /*cnt*/
  google::dense_hash_map<uint32_t, uint32_t> reached[Sensor::PRIV_MAX];

                    /*sensed_id*/  /*priv*/
  google::dense_hash_map<uint32_t, uint8_t> direct_reachable[Sensor::PRIV_MAX];

                        /*sensor*/
  google::dense_hash_set<uint32_t> direct_reached[Sensor::PRIV_MAX];

                      /*sensed_id*/ /*priv*/  // for update
  google::dense_hash_map<uint32_t, uint8_t> old_priv; 
};

struct SensorDelta {
  uint32_t player_id, sensed_id;
  uint8_t prev_priv, new_priv;
  SensorDelta(uint32_t player_id, uint32_t sensed_id, uint8_t prev_priv, 
    uint8_t new_priv): player_id(player_id), sensed_id(sensed_id), 
    prev_priv(prev_priv), new_priv(new_priv) {}
  bool operator<(const SensorDelta& sd) const {
    if (player_id < sd.player_id) return true;
    if (player_id > sd.player_id) return false;
    if (sensed_id < sd.sensed_id) return true;
    if (sensed_id > sd.sensed_id) return false;
    return true;
  }
};

struct View {
  View() {
    s.set_empty_key(-1);
    s.set_deleted_key(-2);
  }
  ~View() {
    for (auto& pr: s) {
      delete pr.second;
    }
  }
  void insert(uint32_t id, uint8_t own_priv) {
    uint8_t top_priv = id&1 ? own_priv+1 : own_priv;
    auto sr = new SensorRecord();
    s[id] = sr;
    sr->max_priv = top_priv;
    sr->old_priv.set_empty_key(-1);
    sr->old_priv.set_deleted_key(-2);
    for (int i = 1; i <= top_priv; ++i ) {
      sr->direct_reachable[i].set_empty_key(-1);
      sr->direct_reached[i].set_empty_key(-1);
      sr->reachable[i].set_empty_key(-1);
      sr->reached[i].set_empty_key(-1);
      sr->direct_reachable[i].set_deleted_key(-2);
      sr->direct_reached[i].set_deleted_key(-2);
      sr->reachable[i].set_deleted_key(-2);
      sr->reached[i].set_deleted_key(-2);
    }
    for (uint8_t p = 1; p <= top_priv; ++p ) {
      Sensor sensor(id, p);
      sr->reachable[sensor.priv][sensor.hash] = 1;
      sr->reached[sensor.priv][sensor.hash] = 1;
    }
    for (uint8_t p = 1; p < top_priv; ++p )
      insert(Sensor(id, p+1), Sensor(id, p));
  }
  void remove(uint32_t id) {
    auto sr = s[id];
    vector<pair<Sensor, Sensor>> v;
    auto my_priv = sr->max_priv;
    if (id&1)
      for (const auto& pr: sr->old_priv)
        if (!(pr.first&1))
          tmp_minus.emplace_back(make_pair(id, pr.first));
    Sensor sensor(id, 0);
    for (uint8_t priv = my_priv; priv; --priv) {
      sensor.setPriv(priv);
      for (const auto& pr: sr->direct_reachable[sensor.priv]) {
        v.emplace_back(make_pair(sensor, Sensor(pr.first, pr.second)));
      }
      for (auto sensor_hash: sr->direct_reached[sensor.priv]) {
        v.emplace_back(make_pair(Sensor(sensor_hash), sensor));
      }
    }
    for (auto& pr: v) {
      remove(pr.first, pr.second);
    }
    s.erase(id);
    delete sr;
  }
  inline void observe(Sensor sensor, Sensor sensed) {
    auto sr1 = s[sensor.id], sr2 = s[sensed.id];
    assert(sr1 && sr2 && sensor.hash!=sensed.hash);
    uint8_t prev_priv = sr1->direct_reachable[sensor.priv][sensed.id];
    uint8_t new_priv = sensed.priv;
    if (prev_priv == new_priv) {
      if (prev_priv==0) sr1->direct_reachable[sensor.priv].erase(sensed.id);
      return;
    }

    if (prev_priv) { // remove sensor->Sensor(sensed.id, prev_priv)
      sensed.setPriv(prev_priv);
      sr1->direct_reachable[sensor.priv].erase(sensed.id);
      sr2->direct_reached[sensed.priv].erase(sensor.hash);
      if (sr1->reached[sensor.priv].size()==1 && 
          sr2->reachable[sensed.priv].size()==1) {
        if ((sr1->reachable[sensor.priv][sensed.hash]-=1)==0) {
          sr1->reachable[sensor.priv].erase(sensed.hash);
          sr2->reached[sensed.priv].erase(sensor.hash);
          if (sensor.id&1) updated.emplace_back(make_pair(sensor.id,sensed.id));
        }
        else 
          sr2->reached[sensed.priv][sensor.hash] -= 1;
      }
      else { 
        for (const auto& pr1: sr1->reached[sensor.priv]) {
          Sensor s1(pr1.first);
          auto sr1 = s[s1.id];
          for (const auto& pr2: sr2->reachable[sensed.priv]) {
            Sensor s2(pr2.first);
            auto sr2 = s[s2.id];
            sr1->reachable[s1.priv][s2.hash] -= pr1.second*pr2.second;
            sr2->reached[s2.priv][s1.hash] -= pr1.second*pr2.second;
            if (sr2->reached[s2.priv][pr1.first]==0) {
              sr1->reachable[s1.priv].erase(pr2.first);
              sr2->reached[s2.priv].erase(pr1.first);
              if (s1.id&1) updated.emplace_back(make_pair(s1.id, s2.id));
            }
          }
        } // end of the general case, for loop
      } // end of else
      sensed.setPriv(new_priv);
    } // end of remove

    if (new_priv) { // insert sensor->sensed
      sr1->direct_reachable[sensor.priv][sensed.id] = sensed.priv;
      sr2->direct_reached[sensed.priv].insert(sensor.hash);
      if (sr1->reached[sensor.priv].size()==1 && 
          sr2->reachable[sensed.priv].size()==1) {
        sr1->reachable[sensor.priv][sensed.hash] += 1;
        sr2->reached[sensed.priv][sensor.hash] += 1;
        if (sensor.id&1) updated.emplace_back(make_pair(sensor.id,sensed.id));
      }
      else {
        for (const auto& pr1: sr1->reached[sensor.priv]) {
          Sensor s1(pr1.first);
          auto sr1 = s[s1.id];
          for (const auto& pr2: sr2->reachable[sensed.priv]) {
            Sensor s2(pr2.first);
            sr1->reachable[s1.priv][s2.hash] += pr1.second*pr2.second;
            s[s2.id]->reached[s2.priv][s1.hash] += pr1.second*pr2.second;
            if (s1.id&1) updated.emplace_back(make_pair(s1.id, s2.id));
          }
        } // end of the general case, a for loop
      } // end of else
    } // end of insert
  }

  unique_ptr<vector<SensorDelta>> update() {
    unique_ptr<vector<SensorDelta>> ans = make_unique<vector<SensorDelta>>();
    sort(updated.begin(), updated.end());
    pair<uint32_t, uint32_t> last_pr(-1, -1);
    google::dense_hash_map<uint32_t, SensorRecord*>::iterator it1, it2;
    for (const auto& pr : updated) {
      if (pr == last_pr) continue;
      if (pr.first != last_pr.first) it1 = s.find(pr.first);
      if (it1 == s.end()) {
        last_pr = pr;
        continue;
      }
      it2 = s.find(pr.second);
      auto sr = it1->second;
      if (it2 != s.end()) {
        auto it_priv = sr->old_priv.find(pr.second);
        uint8_t old_priv = (it_priv==sr->old_priv.end()) ? 0 : it_priv->second;
        uint8_t new_priv = it2->second->max_priv;
        for ( ; new_priv; --new_priv) 
          if (sr->reachable[sr->max_priv].count(getHash(pr.second, new_priv)))
           break;
        if (new_priv > old_priv) {
          ans->emplace_back(SensorDelta(pr.first,pr.second,old_priv,new_priv));
        }
        if (!new_priv) sr->old_priv.erase(pr.second);
        else sr->old_priv[pr.second] = new_priv;
      }
      else  {
        sr->old_priv.erase(pr.second);
      }
      last_pr = pr;
    }
    updated.clear();
    return ans;
  }

  void update(vector<SensorDelta>& plus,
              vector<pair<uint32_t, uint32_t>>& zero) {
    sort(updated.begin(), updated.end());
    zero.insert(zero.end(), tmp_minus.begin(), tmp_minus.end());
    tmp_minus.clear();
    pair<uint32_t, uint32_t> last_pr(-1, -1);
    google::dense_hash_map<uint32_t, SensorRecord*>::iterator it1, it2;
    for (const auto& pr : updated) {
      if (pr == last_pr) continue;
      if (pr.first != last_pr.first) it1 = s.find(pr.first);
      if (it1 == s.end()) {
        last_pr = pr;
        continue;
      }
      it2 = s.find(pr.second);
      auto sr = it1->second;
      auto it_priv = sr->old_priv.find(pr.second);
      uint8_t old_priv = (it_priv==sr->old_priv.end()) ? 0 : it_priv->second;
      if (it2 != s.end()) {
        uint8_t new_priv = it2->second->max_priv;
        for ( ; new_priv; --new_priv) 
          if (sr->reachable[sr->max_priv].count(getHash(pr.second, new_priv)))
           break;
        if (new_priv > old_priv) {
          plus.emplace_back(SensorDelta(pr.first,pr.second,old_priv,new_priv));
        }
        if (!new_priv) {
          sr->old_priv.erase(pr.second);
          if (!(pr.second&1) && old_priv) zero.push_back(pr);
        }
        else sr->old_priv[pr.second] = new_priv;
      }
      else  {
        if (!(pr.second&1) && old_priv) zero.push_back(pr);
        sr->old_priv.erase(pr.second);
      }
      last_pr = pr;
    }
    updated.clear();
  }


  // for each player_id whose Sensor(player_id, max_priv) can reach sensor
  void foreach(Sensor sensor, const function<void(uint32_t player_id)>& fn) {
    google::dense_hash_map<uint32_t, bool> visited;
    visited.set_empty_key(-1);
    visited.set_deleted_key(-2);
    for (const auto& pr: s[sensor.id]->reached[sensor.priv]) {
      Sensor sensing(pr.first);
      if ((sensing.id&1) && !visited[sensing.id]) {
        fn(sensing.id);
        visited[sensing.id] = 1;
      }
    }
  }

  void debug() {
    cout << std::hex;
    for (const auto& pr: s) {
      auto sr = pr.second;
      cout << pr.first << " top priv: " << (uint32_t)sr->max_priv << "\n";
      for (int priv = 1; priv <= sr->max_priv; ++priv) {
        cout << "\t" << "priv: " << (uint32_t)priv << "\n";
        cout << "\t\t" << "direct_reachable:";
        for (const auto& pr: sr->direct_reachable[priv])
          cout << "\t" << pr.first << (uint32_t)pr.second;
        cout << "\n";
        cout << "\t\t" << "direct_reached:";
        for (const auto& pr: sr->direct_reached[priv])
          cout << "\t" << pr;
        cout << "\n";
        cout << "\t\t" << "reachable:";
        for (const auto& pr: sr->reachable[priv])
          cout << "\t" << pr.first << "(" << pr.second << ")";
        cout << "\n";
        cout << "\t\t" << "reached:";
        for (const auto& pr: sr->reached[priv])
          cout << "\t" << pr.first << "(" << pr.second << ")";
        cout << "\n";
      }
    }
  }

private:
  google::dense_hash_map<uint32_t, SensorRecord*> s; // sensor_record
  vector<pair<uint32_t, uint32_t>> updated;
  vector<pair<uint32_t, uint32_t>> tmp_minus;

  // insert sensor->sensed
  inline void insert(const Sensor& sensor, const Sensor& sensed) {
    auto sr1 = s[sensor.id], sr2 = s[sensed.id];
    sr1->direct_reachable[sensor.priv][sensed.id] = sensed.priv;
    sr2->direct_reached[sensed.priv].insert(sensor.hash);
    if (sr1->reached[sensor.priv].size()==1 && 
        sr2->reachable[sensed.priv].size()==1) {
        sr1->reachable[sensor.priv][sensed.hash] += 1;
        sr2->reached[sensed.priv][sensor.hash] += 1;
        if (sensor.id&1) updated.emplace_back(make_pair(sensor.id, sensed.id));
    }
    else {
      for (const auto& pr1: sr1->reached[sensor.priv]) {
        Sensor s1(pr1.first);
        auto sr1 = s[s1.id];
        for (const auto& pr2: sr2->reachable[sensed.priv]) {
          Sensor s2(pr2.first);
          sr1->reachable[s1.priv][s2.hash] += pr1.second*pr2.second;
          s[s2.id]->reached[s2.priv][s1.hash] += pr1.second*pr2.second;
          if (s1.id&1) updated.emplace_back(make_pair(s1.id, s2.id));
        }
      } // end of the general case, for loop
    } // end of else
  }

  // remove sensor->sensed
  inline void remove(const Sensor& sensor, const Sensor& sensed) {
    auto sr1 = s[sensor.id], sr2 = s[sensed.id];
    sr1->direct_reachable[sensor.priv].erase(sensed.id);
    sr2->direct_reached[sensed.priv].erase(sensor.hash);
    if (sr1->reached[sensor.priv].size()==1 && 
        sr2->reachable[sensed.priv].size()==1) {
      if ((sr1->reachable[sensor.priv][sensed.hash]-=1)==0) {
        sr1->reachable[sensor.priv].erase(sensed.hash);
        sr2->reached[sensed.priv].erase(sensor.hash);
        if (sensor.id&1) updated.emplace_back(make_pair(sensor.id, sensed.id));
      }
      else 
        sr2->reached[sensed.priv][sensor.hash] -= 1;
    }
    else { 
      for (const auto& pr1: sr1->reached[sensor.priv]) {
        Sensor s1(pr1.first);
        auto sr1 = s[s1.id];
        for (const auto& pr2: sr2->reachable[sensed.priv]) {
          Sensor s2(pr2.first);
          auto sr2 = s[s2.id];
          sr1->reachable[s1.priv][s2.hash] -= pr1.second*pr2.second;
          sr2->reached[s2.priv][s1.hash] -= pr1.second*pr2.second;
          if (sr2->reached[s2.priv][pr1.first]==0) {
            sr1->reachable[s1.priv].erase(pr2.first);
            sr2->reached[s2.priv].erase(pr1.first);
            if (s1.id&1) updated.emplace_back(make_pair(s1.id, s2.id));
          }
        }
      } // end of the general case, for loop
    } // end of else
  }

};

#endif // WORLD2D_VIEW
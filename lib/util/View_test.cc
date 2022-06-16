#include "View.h"
#include <sparsehash/dense_hash_map>

void test_basic() {
  // constructor
  View vw;
  // insert(player_id/unit_id, own_priv)
  vw.insert(1, 2); // player 1
  vw.insert(2, 2); // unit 2
  vw.insert(4, 2); // unit 4
  // observe(sensor, sensed)
  vw.observe(Sensor(1,3), Sensor(2,1)); // Sensor::Sensor(id, priv)
  vw.observe(Sensor(1,3), Sensor(4,1));
  // update
  unique_ptr<vector<SensorDelta>> up = vw.update();
  std::sort(up->begin(), up->end());
  assert(up->size() == 3);
  assert((*up)[0].player_id==1 && (*up)[0].sensed_id==1 && 
         (*up)[0].prev_priv==0 && (*up)[0].new_priv==3);
  assert((*up)[1].player_id==1 && (*up)[1].sensed_id==2 && 
         (*up)[1].prev_priv==0 && (*up)[1].new_priv==1);
  assert((*up)[2].player_id==1 && (*up)[2].sensed_id==4 && 
         (*up)[2].prev_priv==0 && (*up)[2].new_priv==1);
  // observe(sensor, sensed) can be also used as modify/remove
  vw.observe(Sensor(1,3), Sensor(4,2)); // now Sensor(1,3) can see priv 2 of 4
  vw.observe(Sensor(1,3), Sensor(2,0)); // now Sensor(1,3) cannot see 2
  // foreach, iterator through player_id who can see the sensor
  vw.foreach(Sensor(4,2), [](uint32_t id){
    assert(id == 1);
  });
  vw.foreach(Sensor(2,1), [](uint32_t id){
    assert(false); // now unit 2 is not sensed by any player
  });
}

// 30 ms: 1000 players, init camera #100, @{+20,-15}
void test_performance() {
  View vw;
  const int player_num = 1000;
  const int camera_size = 0;
  const int num_insert = 20;
  const int num_removal = 20;
  // insert players
  for (int i = 0; i < player_num; ++i ) {
    vw.insert(i*2+1, 2);
  }
  // insert init camera
  for (int i = 0; i < player_num; ++i ) {
    for (int k = 0; k < camera_size; ++k ) {
      int j = (i+k+1)%player_num;
      vw.observe(Sensor(i*2+1,3), Sensor(j*2+1,1));
    }
  }   
  // get update
  vw.update();

  // now a new round begins
  auto t1 = stdGetTime();
  for (int ii = 0; ii < 100; ++ii ) {
  // insert num_insert edges for each player
  for (int i = 0; i < player_num; ++i ) {
    for (int k = 0; k < num_insert; ++k ) {
      int j = (i+camera_size+k+1)%player_num;
      vw.observe(Sensor(i*2+1,3), Sensor(j*2+1,1));
    }
  }
  // remove num_removal edges from each player
  for (int i = 0; i < player_num; ++i ) {
    for (int k = 0; k < num_removal; ++k ) {
      int j = (i+k+1)%player_num;
      vw.observe(Sensor(i*2+1,3), Sensor(j*2+1,0));
    }
  }
  // get update
  // auto update = vw.update();
  // cout << update->size() << endl;
  }
  
  // count the elapsed time
  auto t2 = stdGetTime();
  cout << "[ " << t2-t1 << " (ms) ]" << endl;
}


void test_unordered_map() {
  unordered_map<uint32_t, uint32_t> mp;
  auto t1 = stdGetTime();
  int n = 1000000;
  for (int i = 0; i < n; ++i ) {
    uint64_t a = rand();
    a = a*a;
    mp[a] += rand();
  }
  auto t2 = stdGetTime();
  cout << t2-t1 << " (ms) " << endl;
}

void test_google_dense_map() {
  google::dense_hash_map<uint32_t, uint32_t> mp;
  mp.set_empty_key(-1);
  mp.set_deleted_key(-2);
  auto t1 = stdGetTime();
  int n = 1000000;
  for (int i = 0; i < n; ++i ) {
    uint64_t a = rand();
    a = a*a;
    mp[a] = rand();
  }
  auto t2 = stdGetTime();
  cout << t2-t1 << " (ms) " << endl;
}

void test_unordered_map_iterate() {
  unordered_map<uint64_t, uint32_t> mp;
  // uint32_t mp[1000000];
  
  int n = 200;
  for (int i = 0; i < n; ++i ) {
    uint64_t a = rand();
    a = a*a;
    mp[a] += rand();
  }

  int ans = 0;
  auto t1 = stdGetTime();
  for (int i = 0; i < 1000000; ++i ) {
    for (auto pr: mp) {
      if (pr.first) ans += pr.second;
    }
  }
  auto t2 = stdGetTime();
  cout << t2-t1 << " (ms) " << endl;
  cout << ans << endl;
}

void test_player_and_unit() {
  uint32_t p1 = 1, p2 = 3, u1 = 2, u2 = 4;
  View vw;
  vw.insert(p1, 2);
  vw.insert(p2, 2);
  vw.insert(u1, 1);
  vw.insert(u2, 1);
  vw.observe(Sensor(p1,2), Sensor(u1,1));
  vw.observe(Sensor(p2,2), Sensor(u2,1));
  vw.observe(Sensor(p1,2), Sensor(u2,1));
  vw.observe(Sensor(p2,2), Sensor(u1,1));
  vw.observe(Sensor(u1,1), Sensor(p1,1));
  vw.observe(Sensor(u2,1), Sensor(p2,1));
  auto update = vw.update();
  // for (auto delta: *update) {
  //   cout << delta.player_id << " " << delta.sensed_id << " " 
  //        << (uint32_t)delta.prev_priv << " " << (uint32_t)delta.new_priv <<endl;
  // }
  ASSERT(update->size()==8, "test_player_and_unit fails");
  vw.observe(Sensor(p1,2), Sensor(u2,0)); // remove p1->u2
  // vw.remove(u1);
  // vw.debug();
}

void test_foreach() {
  uint32_t p1 = 1, p2 = 3, u1 = 2, u2 = 4;
  View vw;
  vw.insert(p1, 2);
  vw.insert(p2, 2);
  vw.insert(u1, 1);
  vw.insert(u2, 1);
  vw.observe(Sensor(p1,2), Sensor(u1,1));
  vw.observe(Sensor(p2,2), Sensor(u2,1));
  vw.observe(Sensor(p1,2), Sensor(u2,1));
  vw.observe(Sensor(p2,2), Sensor(u1,1));
  vw.observe(Sensor(u1,1), Sensor(p1,1));
  vw.observe(Sensor(u2,1), Sensor(p2,1));
  vector<uint32_t> ans;
  vw.foreach(Sensor(u1,1), [&ans](uint32_t id){
    ans.push_back(id);
  });
  sort(ans.begin(), ans.end());
  ASSERT(ans==vector<uint32_t>({p1,p2}), "test_foreach fails 1");
  
  ans.clear();
  vw.observe(Sensor(p1,2), Sensor(u1,0)); 
  vw.foreach(Sensor(u1,1), [&ans](uint32_t id){
    ans.push_back(id);
  });
  ASSERT(ans==vector<uint32_t>({p2}), "test_foreach fails 2");

}

int main() {
  test_basic();
  test_new_update();
  // test_player_and_unit();
  test_performance();
  // test_foreach();
  // test_unordered_map();
  // test_google_dense_map();
}
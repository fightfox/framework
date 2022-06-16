#include "Camera.h"

void test_basic() {
  Camera cm;
  cm.insert(0, 1001);
  cm.insert(1, 1002);
  cm.insert(0, 1002);
  cm.insert(1, 1003);
  cm.foreach(1, [](uint32_t id){
    cout << "1 can see: " << id << endl;
  });
  cm.foreach(1003, [](uint32_t id) {
    cout << "1003 can be seen by: " << id << endl; 
  });
  cm.remove(1, 1003);
  cm.foreach(1, [](uint32_t id){
    cout << "Now 1 can see: " << id << endl;
  });
  cm.remove(1, 1002);
  cm.foreach(1, [](uint32_t id){
    cout << "1 is not supposed to see: " << id << endl;
  });
}

uint32_t index(uint32_t i) {
  return i*i*i;
}

void test_performance() {
  Camera cm;
  const int player_num = 1000;
  const int camera_size = 0;
  const int num_insert = 20;
  const int num_removal = 20;
  // insert init camera
  for (int i = 0; i < player_num; ++i ) {
    for (int k = 0; k < camera_size; ++k ) {
      int j = (i+k+1)%player_num;
      cm.insert(index(i*2+1), index(j*2+1));
    }
  }   
  // now a new round begins
  auto t1 = stdGetTime();
  for (int ii = 0; ii < 100; ++ii ) {
  // insert num_insert edges for each player
  for (int i = 0; i < player_num; ++i ) {
    for (int k = 0; k < num_insert; ++k ) {
      int j = (i+camera_size+k+1)%player_num;
      cm.insert(index(i*2+1), index(j*2+1));
    }
  }
  // remove num_removal edges from each player
  for (int i = 0; i < player_num; ++i ) {
    for (int k = 0; k < num_removal; ++k ) {
      int j = (i+k+1)%player_num;
      cm.remove(index(i*2+1), index(j*2+1));
    }
  }
  }
  // count the elapsed time
  auto t2 = stdGetTime();
  cout << "[ " << t2-t1 << " (ms) ]" << endl;
}

int main() {
  test_basic();
  test_performance();
}
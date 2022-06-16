#ifndef WORLD2D_STD
#define WORLD2D_STD

#include <algorithm>
using std::sort;
using std::min;
using std::max;
#include <atomic>
using std::atomic;
#include <cassert>
#include <cmath>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
using std::function;
#include <future>
#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
#include <iomanip>
#include <memory>
using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_unique;
using std::make_shared;
#include <mutex>
using std::mutex;
#include <queue>
using std::queue;
#include <set>
using std::set;
#include <string>
using std::string;
#include <thread>
using std::thread;
#include <unordered_map>
using std::unordered_map;
// #include <sparsepp/spp.h>
// #define unordered_map spp::sparse_hash_map
#include <unordered_set>
using std::unordered_set;
#include <utility>
using std::pair;
using std::make_pair;
#include <vector>
using std::vector;
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>
using google::dense_hash_map;
using google::dense_hash_set;

typedef double Time;

#define DEBUG cout << "[" << __FILE__ << ", Line " << __LINE__ << "] "
#define ASSERT(boo, description) assert(boo && description)

inline uint64_t stdGetTime() {
  using namespace std::chrono;
  milliseconds ms = duration_cast<milliseconds>(
    system_clock::now().time_since_epoch()
  );
  return ms.count();
}

inline void setInterval(function<void()> fn, uint32_t t) {
  thread th([fn, t](){
    while (true) {
      auto t1 = stdGetTime();
      fn();
      auto t2 = stdGetTime();
      // cout << "Interval: " << t2 - t1 << endl;
      int32_t tt = t-(int32_t)(t2-t1);
      tt = tt < 0 ? 0 : tt;
      std::chrono::milliseconds timespan(tt);
      std::this_thread::sleep_for(timespan);
    }
  });
  th.detach();
}

inline double rand01() {
  return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
}

inline double abs(double x) {
  return x > 0 ? x : -x;
}

template<typename T>
constexpr inline T sqr(T x) {return x*x;}

template<typename T>
class Queue {  // queue with lock
  mutex lk; // lock
  queue<T> q;
public:
  inline void lock() {lk.lock();}
  inline void unlock() {lk.unlock();}
  inline T consume() {
    T ans;
    lk.lock();
    if (!q.empty()) {
      ans = std::move(q.front());
      q.pop();
    }
    lk.unlock();
    return std::move(ans);
  }
  inline void produce(T t) {
    lk.lock();
    q.push(std::move(t));
    lk.unlock();
  }
};

// used for default inialization of quadtree
const double X_MIN = -100000;
const double X_MAX = +100000;
const double Y_MIN = -100000;
const double Y_MAX = +100000;
const double EPSILON = 0.0000000001;
const double EPSILONSQR = EPSILON*EPSILON;
const double DELTA_EPSILON = 0.0001;
const double PI = 3.1415926535898;
const double INNER_VELOCITY = 0.1;
const double EPSILON_LENGTH = 1;
const double INFINITY_MASS = 100000000;
const int64_t BIG_INT64 = 500000000;
const uint32_t INVALID_ID = -1;
const Time EPSILON_TIME = 0.01;
const Time INVALID_TIME = -1;
const Time IDLE_TIME = 3.0;
const Time TIME_PER_TICK = 40.0;
const Time MINIMUM_TIME_PER_TICK = 10.0;
const double VISION_PADDING = 300;
const double ZOOM_X = 1440;
const double ZOOM_Y = 900;
const double SCREEN_WIDTH = ZOOM_X + VISION_PADDING*3;
const double SCREEN_HEIGHT = ZOOM_Y + VISION_PADDING*3;
const double SCREEN_HALF_WIDTH = SCREEN_WIDTH/2;
const double SCREEN_HALF_HEIGHT = SCREEN_HEIGHT/2;



inline double sudoinverse(const double x) {
  if (x==INFINITY_MASS)
    return 0;
  else
    return 1/x;
} 


#endif // WORLD2D_STD
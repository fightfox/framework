#include "std.h"

void test_lock_performance() {
  mutex lk;
  auto t1 = getTime();
  int64_t ans = 0;
  int n = 100000000;
  for (int i = 0; i < n; ++i ) {
    lk.lock();
    ans += min(i, 100);
    lk.unlock();
  }
  auto t2 = getTime();
  DEBUG << t2-t1 << " (ms) " << "[dump " << ans << "]" << endl;
  // approx 3.3s / 10^8 lock & unlock
}

void test_queue_performance() {
  Queue<unique_ptr<int>> q;
  auto t1 = getTime();
  int n = 100000000;
  int64_t ans = 0;
  for (int i = 0; i < n; ++i ) { 
    q.produce(std::move(make_unique<int>(i)));
    auto tmp = q.consume();
    ans += *tmp;
  }
  auto t2 = getTime();
  DEBUG << t2-t1 << " (ms) " << "[dump " << ans << "]" << endl;
  // approx 14 s
}

struct Base {
  Base(){}
  // void virtual apple() = 0;
  int i;
};

struct Child: Base {
  int a[100];
  Child(){
    a[2] = 61;
  }
  void apple() {}
};


void test_base_virtual_delete() {
  const int n = 100000000;
  for (int i = 0; i < n; ++i ) {
    Base* p = new Child();
    delete p;
  }
  for (;;);
}

void test_unordered_map_performance() {
  auto t1 = getTime();
  const int N = 10000000;
  unordered_map<int, int> m;
  m.reserve(2 * N);
  for (int i = 0; i < N; i++) {
    m[i*i] = i;
  }
  auto t2 = getTime();
  cout << t2 - t1 << endl;
}

int main() {
  // test_lock_performance();
  // test_queue_performance();
  // test_base_virtual_delete();
  test_unordered_map_performance();
  cout << (1-(uint32_t)4) << endl;
}
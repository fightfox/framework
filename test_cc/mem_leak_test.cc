#include <queue>
#include <mutex>
#include <memory>
#include <iostream>
using namespace std;

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



struct Food {
  int i;
  Food(int i): i(i){}
};

Queue<unique_ptr<Food>> qq;


void doSomething() {
  qq.produce(move(make_unique<Food>(33)));
  qq.produce(move(make_unique<Food>(42)));
  qq.produce(move(make_unique<Food>(24)));
  auto kaka = qq.consume();
}


int main() {
  doSomething();
  auto haha = qq.consume();
  cout << "haha" << " " << haha->i << endl;
}
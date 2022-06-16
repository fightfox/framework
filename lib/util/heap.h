#ifndef WOLRD2D_HEAP
#define WOLRD2D_HEAP

#include "../std.h"

// Type T must contain the following 2 fields 
// * Time time
// * uint32_t heap_id
template<typename T>
struct heap {
  vector<T*> v;
  heap() {
    v.reserve(4000);  // tune
  }
  void swim(uint32_t i) {
    while (i && v[i]->time < v[i>>1]->time) {
      T* tmp = v[i>>1]; v[i>>1] = v[i]; v[i] = tmp;
      v[i>>1]->heap_id = i>>1;
      v[i]->heap_id = i;
      i >>= 1;
    }
  }
  void sink(uint32_t i) {
    while (i<<1 < v.size()) {
      int j = i<<1;
      if (j+1<v.size() && v[j+1]->time < v[j]->time) ++j;
      if (v[i]->time <= v[j]->time) break;
      T* tmp = v[j]; v[j] = v[i]; v[i] = tmp;
      v[j]->heap_id = j;
      v[i]->heap_id = i;
      i = j;
    }
  }
  void insert(T* t) {
    t->heap_id = v.size();
    v.push_back(t);
    swim(v.size()-1);
  }
  void remove(uint32_t i) {
    assert(i < v.size() && "removing an invalid heap index!");
    if (i == v.size()-1) v.pop_back();
    else {
      v[i] = v.back();
      v.pop_back();
      v[i]->heap_id = i;
      sink(i);
    }
  }
  inline T* top() {
    return v[0];
  }
};

#endif // WOLRD2D_HEAP
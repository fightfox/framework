// shared_ptr constructor example
#include "../lib/std.h"

const int maxn = 50000;
const int caseId = 1;
struct C {int data;
    C(int i) : data(i) {}
};

int add(shared_ptr<C> p, shared_ptr<C> q) {
    return p->data+q->data;
}

int addNuked(C* p, C* q) {
    return p->data+q->data;
}

shared_ptr<C> a[maxn];
int ans;

int main () {
  ans = 0;
  for (int i = 0; i < maxn; i++) {
      a[i] = make_shared<C>(i);
  }

  auto t1 = getTime();
  for (int i = 0; i < maxn; i++) 
    for (int j = 0; j < maxn; j++) {
        if (caseId==1)
            ans += add(a[i], a[j]);
        else
            ans += addNuked(a[i].get(), a[j].get());
    }
  auto t2 = getTime();
  cout<<ans<<endl;
  cout<<t2-t1 << " (ms)" << endl;
  return 0;
}
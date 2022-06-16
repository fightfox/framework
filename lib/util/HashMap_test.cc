#include "HashMap.h"
#include <sparsehash/dense_hash_map>

int f(int t) {
  if (t > 0) return t-(t*t/2900000%82991);
  else return f(t+1);
}

void test_hash_load_balance() {
  const int n = 64;   // number of bins
  const int m = 256;    // number of balls
  int *bin = new int[n]();
  int *cnt = new int[m+1]();
  // int bin[n] = {0}, cnt[m+1] = {0};
  for (int i = 0; i < m; ++i )
    bin[hash(i)% n]++;  // i*2+1, can try different patterns
  for (int i = 0; i < n; ++i )
    ++cnt[bin[i]];
  for (int i = 0; i <= m; ++i )
    if (cnt[i])
      cout << i << "\t" << cnt[i] << "\n";
}

void test_hash_performance() {
  int ans = 0;
  int n = 100000000;
  auto t1 = stdGetTime();
  for (int i = 0; i < n; ++i )
    ans += hash(i);
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  cout << ans << endl;
}

inline int index(int i) {
  return i*i*i;
}
void test_hash_map_1024_insert(){
  const int T = 100000;
  const int N = 1000;
  HashMap mp[1000];
  int ans = 0;
  auto t1 = stdGetTime();
  for (int t = 0; t < T; ++t ) {
    for (int i = 0; i < N; ++i ) {
      mp[t%1000][index(i)] += 100;
    }
  }
  for (int i = 0; i < N; ++i ) 
    ans += mp[44][index(i)];
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  cout << ans;
}

void test_new_default_init() {
  int* a = new int;
  cout << *a << endl;
}

void test_hash_map_1024_erase() {
  const int T = 100000;
  const int N = 1000;
  HashMap mp[1000];
  int ans = 0;
  auto t1 = stdGetTime();
  for (int t = 0; t < T; ++t ) {
    for (int i = 0; i < N; ++i ) {
      mp[t%1000][index(i)] += 100;
    }
    for (int i = N; i >= 0; --i ) {
      ans += mp[t%1000].erase(index(i));
    }
  }
  for (int i = 0; i < N; ++i ) 
    ans += mp[44][index(i)];
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  cout << ans;
}

void test_hash_map_1000000() {
  const int T = 10000;
  const int N = 500;
  HashMap mp;
  int ans = 0;
  auto t1 = stdGetTime();
  for (int t = 0; t < T; ++t ) {
    for (int i = 0; i < N; ++i ) {
      mp[index(i)] += 1;
    }
    for (int i = N-1; i >= 0; --i ) {
      mp[index(i)] += 1;
      // ans += mp.erase(index(i));
    }
  }
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  for (int i = 0; i < N; ++i ) 
    ans += mp[index(i)];
  cout << ans/(1e5*(t2-t1)) << endl;
  cout << ans << endl;
}

void test_unordered_map_1000000() {
  const int T = 10;
  const int N = 1000000;
  unordered_map<uint32_t, uint32_t> mp;
  int ans = 0;
  auto t1 = stdGetTime();
  for (int t = 0; t < T; ++t ) {
    for (int i = 0; i < N; ++i ) {
      mp[index(i)] += 1;
    }
    for (int i = N-1; i >= 0; --i ) {
      mp[index(i)] += 1;
      // ans += mp.erase(index(i));
    }
  }
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  for (int i = 0; i < N; ++i ) 
    ans += mp[index(i)];
  cout << ans/(1e5*(t2-t1)) << endl;
  cout << ans << endl;
}

void test_google_map_1000000() {
  const int T = 100000;
  const int N = 50;
  google::dense_hash_map<uint32_t, uint32_t> mp;
  mp.set_empty_key(-1);
  int ans = 0;
  auto t1 = stdGetTime();
  for (int t = 0; t < T; ++t ) {
    for (int i = 0; i < N; ++i ) {
      mp[index(i)] += 1;
    }
    for (int i = N-1; i >= 0; --i ) {
      mp[index(i)] += 1;
      // ans += mp.erase(index(i));
    }
  }
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  for (int i = 0; i < N; ++i ) 
    ans += mp[index(i)];
  cout << ans/(1e5*(t2-t1)) << endl;
  cout << ans << endl;
}


void test_array_1000000() {
  const int T = 1;
  const int N = 10000000;
  int a[N];
  int ans = 0;
  auto t1 = stdGetTime();
  for (int t = 0; t < T; ++t ) {
    for (int i = 0; i < N; ++i ) 
    {
      a[hash(i)%N] += 5;
    }
    for (int i = N; i >= 0; --i ) {
      ans += a[hash(i)%N];
    }
  }
  for (int i = 0; i < N; ++i ) 
    ans += a[hash(i)%N];
  auto t2 = stdGetTime();
  cout << t2 - t1 << " (ms) " << endl;
  cout << ans << endl;
}
 
void test_f() {
  const int T = 100000000;
  int ans = 0;
  auto t1 = stdGetTime();
  for (int i = 0; i < T; ++i ) {
    ans += f(i);
  }
  auto t2 = stdGetTime();
  cout << T/(1e5*(t2-t1)) << endl;
  cout << ans << endl;
}

int main() {
  // test_new_default_init();
  // test_hash_load_balance();
  // test_hash_performance();
  // test_hash_map_1024_insert();
  // test_hash_map_1024_erase();
  test_hash_map_1000000();
  // test_unordered_map_1000000();
  test_google_map_1000000();
  // test_array_1000000();
  // test_f();


}
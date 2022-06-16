#ifndef WORLD2D_HASH_MAP
#define WORLD2D_HASH_MAP

#include "../std.h"

__attribute__((always_inline)) 
inline uint32_t hash(uint32_t k) {
  const uint32_t m = 0x5bd1e995;
  const int r = 24;
  uint32_t h = 200322113 ^ 4; // seed
  k *= m; 
  k ^= k >> r; 
  k *= m; 
  h *= m; 
  h ^= k;
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
} 

// consts for linear probe hash table
// capacity,     bin_mask,      space-2     // insert/find performance (*10^8/s)
// bin_num must be a power of 2
static constexpr const uint32_t HMP[][3] =   // hash_map_parameter
{
  {      11,            0,       12*2     }, // raw array
  {      32,       64*2-2,       64*2+40  }, 
  {      64,      128*2-2,      128*2+60  },
  {     128,      256*2-2,      256*2+100 },
  {     256,      512*2-2,      512*2+160 },
  {     512,     1024*2-2,     1024*2+200 },
  {    1024,     2048*2-2,     2048*2+400 }, // fast:  ~10^8/s *
  {    2048,     4096*2-2,     4096*2+440 }, 
  {    4096,     8192*2-2,     8192*2+500 }, // 0.9
  {    8192,    16384*2-2,    16384*2+540 }, // 0.95
  {   16384,    32768*2-2,    32768*2+1030}, // 0.9
  {   32768,    65536*2-2,    65536*2+1040}, // 0.8
  {   65536,   131072*2-2,   131072*2+1050}, // 0.74
  {  131072,   262144*2-2,   262144*2+2060}, // 0.69
  {  262144,   524288*2-2,   524288*2+2070}, // 0.5
  {  524288,  1048576*2-2,  1048576*2+9080}, // 0.38
  { 1048576,  2097152*2-2,  2097152*2+3010}, // 0.33
  // The following ranges are not well tested and can be very inefficient
  { 2097152,  4194304*2-2,  4194304*2+3020},
  { 4194304,  8388608*2-2,  8388608*2+3030},
  { 8388608, 16777216*2-2, 16777216*2+4040},
  {16777216, 33554432*2-2, 33554432*2+4050},
  {33554432, 67108864*2-2, 67108864*2+4060},
  {67108864,134217728*2-2,134217728*2+9000},
};

struct HashMap {
  static const uint32_t EMPTY_KEY = -1;
  bool overflow;
  uint32_t* buffer;
  uint32_t sz;
  uint32_t lvl;

  HashMap() : sz(0), lvl(0) {
    overflow = false;
    buffer = new uint32_t[HMP[lvl][2]+4];
    for (int i = 0; i < HMP[lvl][2]+4; i += 2) {
      buffer[i] = EMPTY_KEY;
      buffer[i+1] = 0;
    }
  }
  
  ~HashMap() {
    delete [] buffer;
  }

  inline vector<pair<uint32_t, uint32_t>> get() {
    vector<pair<uint32_t, uint32_t>> ans;
    for (int i = 0; i < HMP[lvl][2]+4; i += 2) {
      if (buffer[i] != EMPTY_KEY)
        ans.push_back(make_pair(buffer[i], buffer[i+1]));
    }
    return ans;
  }
  inline void clear() {
    delete [] buffer;
    lvl = sz = 0;
    overflow = false;
    buffer = new uint32_t[HMP[lvl][2]+4];
    for (int i = 0; i < HMP[lvl][2]+4; i += 2) {
      buffer[i] = EMPTY_KEY;
      buffer[i+1] = 0;
    }
  }

  __attribute__((always_inline)) 
  inline uint32_t& operator[](const uint32_t& key) {
    if (sz >= HMP[lvl][0] || overflow) changeLevel(lvl+1);
    uint32_t* index = buffer + (hash(key) & HMP[lvl][1]);
    while (*index!=EMPTY_KEY) {
      if (*index==key) return index[1];
      index+=2;
    }
    ++sz; 
    *index = key;
    if (index == buffer+HMP[lvl][2]) overflow = true;
    return index[1];
  }

  inline bool count(const uint32_t& key) const {
    uint32_t* index = buffer + (hash(key) & HMP[lvl][1]);
    while (*index!=EMPTY_KEY) {
      if (*index==key) return true;      
      index+=2;
    }
    return false;
  }


  __attribute__((always_inline)) 
  inline bool erase(const uint32_t& key) {
    uint32_t i = hash(key) & HMP[lvl][1];
    while (buffer[i]!=EMPTY_KEY && buffer[i]!=key) i += 2;
    if (buffer[i]==key) {
      --sz;
      uint32_t j = i+2;
      while (buffer[j]!=EMPTY_KEY) {
        if ((hash(buffer[j])&HMP[lvl][1])<=i) {
          buffer[i] = buffer[j];
          buffer[i+1] = buffer[j+1];
          i = j;
        }
        j += 2;
      }
      buffer[i] = EMPTY_KEY;
      buffer[i+1] = 0;
      if (lvl>1 && sz<HMP[lvl-2][0]) changeLevel(lvl-1);
      return true;
    }
    else 
      return false;
  }

  __attribute__((always_inline)) 
  inline void changeLevel(uint32_t new_lvl) {
    if (new_lvl > sizeof(HMP)/12) return;
    uint32_t* b = buffer;
    auto old_lvl = lvl;
    lvl = new_lvl;
    buffer = new uint32_t[HMP[lvl][2]+4];
    for (int i = 0; i < HMP[lvl][2]+4; i += 2) {
      buffer[i] = EMPTY_KEY;
      buffer[i+1] = 0;
    }
    for (int i = 0; i < HMP[old_lvl][2]+4; i += 2) {
      if (b[i] != EMPTY_KEY) {
        uint32_t* index = buffer + (hash(b[i]) & HMP[lvl][1]);
        while (*index!=EMPTY_KEY) index+=2;
        *index = b[i];
        index[1] = b[i+1];
      }
    }
    delete [] b;
  }
};
#endif // WORLD2D_HASH_MAP

// Note: godbolt.org show assemble code in real time
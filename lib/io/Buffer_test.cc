#include "../std.h"
#include "Buffer.h"

// We assume the server is small endian 
// i.e. the last byte is stored in small address
void test_endian() {
  unsigned int i = 1;
  assert(*(char*)&i && "This computer is big endian. QUIT\n");
}

void test_float_size() {
  assert(sizeof(float)==4 && "double size should be 4");
}

void test_basic_read_and_writer() {
  Buffer::Writer bout;
  // write int8_t, int16_t, int32_t, double, int64_t, string
  bout << (int8_t)23 << (int16_t)8979 << -33455432 << 3.141592f 
       << 10000000000000000LL << "game1game2";
  // Construct a buffer reader using (char*, int)
  // It will copy the data into its own buffer
  Buffer::Reader bin(bout.getBufferPtr(), bout.getBufferSize());
  // read int_8
  int8_t i8;
  bin >> i8;
  assert(i8==23 && "fail on write/read int_8");
  // read int_8
  int16_t i16;
  bin >> i16;
  assert(i16==8979 && "fail on write/read int_16");
  // read int_32
  int i32;
  bin >> i32;
  assert(i32==-33455432 && "fail on write/read int");
  // read double
  double fl;
  bin >> fl;
  assert(std::abs(fl-3.141592)<1e-5 && "fail on write/read double");
  // read int64_t
  int64_t t;
  bin >> t;
  assert(t==10000000000000000LL && "fail on write/read int64_t");
  // read string
  string s;
  bin >> s;
  assert(s=="game1game2" && "fail on write/read string");
}

void test_unsigned_int() {
  uint64_t u64 = 1uLL << 63;
  uint32_t u32 = 1uL << 31;
  uint16_t u16 = -7;
  uint8_t u8 = 255;
  Buffer::Writer bout;
  bout << u8 << u16 << u32 << u64;
  uint64_t v64;
  uint32_t v32;
  uint16_t v16;
  uint8_t v8;
  Buffer::Reader bin(bout.getBufferPtr(), bout.getBufferSize());
  bin >> v8 >> v16 >> v32 >> v64;
  assert(v8==u8 && v16==u16 && v32==u32 && v64==u64 && "unsigned int fails");
}

void test_unsigned_and_signed() {
  uint32_t u32 = 10000;
  int32_t i32 = 10000;
  Buffer::Writer bout;
  bout << u32 << i32;
  int32_t a;
  uint32_t b;
  Buffer::Reader bin(bout.getBufferPtr(), bout.getBufferSize());
  bin >> a >> b;
  assert(u32==a && i32==b && "unsigned and signed not competible");
}

void test_reader_buffer_overflow() {
  int8_t i = 5;
  char ch;
  Buffer::Reader bin(&ch, 0);
  assert(bin && "reader should be true before overflow");
  bin >> i;
  assert(!bin && "reader should be false after overflow");
  assert(i==5 && "reader should not assign to any variable when overflow");
}

void test_very_long_message() {
  // write a very very long message
  auto t1 = stdGetTime();
  Buffer::Writer bout;
  for (int k = 0; k < 10; ++k ) {
    for (int j = 0; j < 40000; ++j )
      bout << (double)(j);
    for (int j = 0; j < 10000; ++j )
      bout << j;
    for (int64_t j = 0; j < 15000; ++j )
      bout << j;
    for (int j = 0; j < 3000; ++j )
      bout << "I have a good name!";
  }
  // 3,800,000 bytes
  auto t2 = stdGetTime();
  assert(bout.getBufferSize() == 3800000);
  cout << "[Line " << __LINE__ << "] " << t2-t1 << " (ms)" << endl;

  // read the very very long message
  t1 = stdGetTime();
  Buffer::Reader bin(bout.getBufferPtr(), bout.getBufferSize());
  double a;
  int b;
  int64_t c;
  string d;
  for (int k = 0; k < 10; ++k ) {
    for (int j = 0; j < 40000; ++j )
      assert(bin >> a && a == (double)j);
    for (int j = 0; j < 10000; ++j )
      assert(bin >> b && b == j);
    for (int64_t j = 0; j < 15000; ++j )
      assert(bin >> c && c == j);
    for (int j = 0; j < 3000; ++j )
      assert(bin >> d && d == "I have a good name!");
  }
  int8_t i8;
  assert(!(bin >> i8));
  t2 = stdGetTime();
  cout << "[Line " << __LINE__ << "] " << t2-t1 << " (ms) " << "\n"; 
}

void test_append_in_front() {
  Buffer::Writer bout;
  Buffer::Writer bout_;
  bout << (uint32_t)2;
  bout_ << (uint32_t)1;
  bout << bout_;
  Buffer::Reader bin(bout.getBufferPtr(), bout.getBufferSize());
  uint32_t a, b;
  bin >> a >> b;
  assert(a==1 && b==2);
}

// zlap: 3kB/s 10packets/s 
// agar: 6kB/s 
// psiball: 25packets/s each packets 300-400 bytes ~ 75-100 ints
// each second, we need to send 1000*25 packets to support 1000 players
// if we ignore new operation cost
// 75 (ms) when using string += 
// 35 (ms) when using string append
// 20 (ms) when using raw bytes
// // 16 (ms) when not considering overflow
// 21 (ms) when considering overflow ʕ•ᴥ•ʔ 
// in that case, it takes 2% time for the game discribed above
// note that it's almost best possible since it's about 4*10^8 bytes / second
void test_writer_performance_normal_message() {
  auto t1 = stdGetTime();
  const int n = 1000*25;
  for (int i = 0; i < n; ++i ) {
    Buffer::Writer bout;
    for (int j = 0; j < 40; ++j )
      bout << (double)(j);
    for (int j = 0; j < 10; ++j )
      bout << j;
    for (int64_t j = 0; j < 15; ++j )
      bout << j;
    for (int j = 0; j < 3; ++j )
      bout << "I have a good name!";
  } // 50*4+15*8+3*20=380 bytes
  auto t2 = stdGetTime();
  cout << "[Line " << __LINE__ << "] " << t2-t1 << " (ms) " << "\n";
}

void test_writer_performance_long_message() {
  auto t1 = stdGetTime();
  const int n = 1000*25;
  for (int i = 0; i < n; ++i ) {
    Buffer::Writer bout;
    for (int k = 0; k < 10; ++k ) {
      for (int j = 0; j < 40; ++j )
        bout << (double)(j);
      for (int j = 0; j < 10; ++j )
        bout << j;
      for (int64_t j = 0; j < 15; ++j )
        bout << j;
      for (int j = 0; j < 3; ++j )
        bout << "I have a good name!";
    }
  } // 3800 bytes
  auto t2 = stdGetTime();
  cout << "[Line " << __LINE__ << "] " << t2-t1 << " (ms) " << "\n"; // 228 (ms) k = 10; 1978 (ms) k = 100
}

void test_writer_performance_short_message() {
  auto t1 = stdGetTime();
  const int n = 1000*25;
  for (int i = 0; i < n; ++i ) {
    Buffer::Writer bout;
    for (int j = 0; j < 6; ++j )
      bout << (double)(j);
    for (int j = 0; j < 8; ++j )
      bout << j;
    for (int64_t j = 0; j < 3; ++j )
      bout << j;
    for (int j = 0; j < 1; ++j )
      bout << "I have a good name!";
  } // 100 bytes
  auto t2 = stdGetTime();
  cout << "[Line " << __LINE__ << "] " << t2-t1 << " (ms) " << "\n"; // 6 (ms) 
}

// 37 (ms) when doing assert
// 30 (ms) when not doing assert
// 28 (ms) when not using string as the buffer
// 23 (ms) when use .clear .append instead of = string()
// 22 (ms) when rearrange if and else
void test_reader_performance() {
  Buffer::Writer bout;
  for (int j = 0; j < 40; ++j )
    bout << (double)(j);
  for (int j = 0; j < 10; ++j )
    bout << j;
  for (int64_t j = 0; j < 15; ++j )
    bout << j;
  for (int j = 0; j < 3; ++j )
    bout << "I have a good name!";
  auto t1 = stdGetTime();
  for (int i = 0; i < 1000*25; ++i ) {
    Buffer::Reader bin(bout.getBufferPtr(), bout.getBufferSize());
    double a;
    int b;
    int64_t c;
    string d;
    for (int j = 0; j < 40; ++j )
      bin >> a; // assert(bin >> a && a == (double)j);
    for (int j = 0; j < 10; ++j )
      bin >> b; // assert(bin >> b && b == j);
    for (int64_t j = 0; j < 15; ++j )
      bin >> c; // assert(bin >> c && c == j);
    for (int j = 0; j < 3; ++j )
      bin >> d; // assert(bin >> d && d == "I have a good name!");
    // int8_t i8;
    // assert(!(bin >> i8));
  }
  auto t2 = stdGetTime();
  cout << "[Line " << __LINE__ << "] " << t2-t1 << " (ms) " << "\n"; 
}

int main() {
  // we assume the server is little-endian and double takes up 4 bits 
  test_endian(); 
  test_float_size();

  // test correctness
  test_basic_read_and_writer();
  test_unsigned_int();
  test_unsigned_and_signed();
  test_reader_buffer_overflow();
  test_very_long_message();
  test_append_in_front();

  // test performance
  test_writer_performance_normal_message();
  test_writer_performance_long_message();
  test_writer_performance_short_message();
  test_reader_performance();
}
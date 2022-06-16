// [export] Buffer::Reader, Buffer::Writer 
// [use case] read/write char|int|double|int64_t|string from/to a buffer
// [Caution] string length is at most 255
//           double from users might be invalid 
// TODO[endian]: we assume little-endian, in the future => endian-independent
// TODO: check invalid encoding of double
// TODO: support long strings
// TODO: support non-unicode compression (now we assume that all the names 
//       provided by players are unicode. About 80%-90% players actually do not 
//       use unicode.) 
#ifndef WORLD2D_IO_BUFFER
#define WORLD2D_IO_BUFFER

#include "cstring"
#include "../std.h"

namespace Buffer {
const int32_t BUFFER_RESERVE_SIZE = 956;  // ʕ•ᴥ•ʔ 
const int32_t BUFFER_RENEW_SIZE = 700;    // ʕ•ᴥ•ʔ 
//==============================================================================
struct Reader { 
  // we save a copy of string(_buffer_ptr, _buffer_size) here
  Reader(const char* b_ptr, int32_t b_size) {
    buffer_ptr = new char[b_size+1];
    memcpy(buffer_ptr, b_ptr, b_size);
    buffer_size = b_size;
    index = 0;
  }
  ~Reader() {
    delete[] buffer_ptr;
  }

  // similar to std::cin, it overloads operator>> and checks buffer overflow
  inline Reader& operator>>(int8_t& i8) {
    if (index < buffer_size) 
      i8 = (int8_t)buffer_ptr[index];
    ++index;
    return *this;
  }

  inline Reader& operator>>(uint8_t& u8) { // unsigned
    if (index < buffer_size) 
      u8 = (uint8_t)buffer_ptr[index];
    ++index;
    return *this;
  }

  inline Reader& operator>>(int16_t& i16) {
    if (index+1 < buffer_size) {
      i16 = *(int16_t*)(buffer_ptr+index);
    }
    index += 2;
    return *this;
  }

  inline Reader& operator>>(uint16_t& u16) { // unsigned
    if (index+1 < buffer_size) {
      u16 = *(uint16_t*)(buffer_ptr+index);
    }
    index += 2;
    return *this;
  }

  inline Reader& operator>>(int32_t& i32) {
    if (index+3 < buffer_size) {
      i32 = *(int32_t*)(buffer_ptr+index);
    }
    index += 4;
    return *this;
  }

  inline Reader& operator>>(uint32_t& u32) { // unsigned
    if (index+3 < buffer_size) {
      u32 = *(uint32_t*)(buffer_ptr+index);
    }
    index += 4;
    return *this;
  }

  inline Reader& operator>>(double& fl) {
    if (index+3 < buffer_size) {
      fl = (double)*(float*)(buffer_ptr+index);
    }
    index += 4;
    return *this;
  }

  inline Reader& operator>>(int64_t& i64) {
    if (index+7 < buffer_size) {
      i64 = *(int64_t*)(buffer_ptr+index);
    } 
    index += 8;
    return *this;
  }
  
  inline Reader& operator>>(uint64_t& u64) { // unsigned
    if (index+7 < buffer_size) {
      u64 = *(uint64_t*)(buffer_ptr+index);
    } 
    index += 8;
    return *this;
  }

  inline Reader& operator>>(string& s) {
    int32_t n = index<buffer_size ? (uint8_t)buffer_ptr[index] : 0;
    if (index+n < buffer_size) { 
      s = string(buffer_ptr+index+1, n);
    }
    index += n+1;
    return *this;
  }

  inline bool checkHeader(uint8_t x) {
    return (buffer_size>0 && (uint8_t)buffer_ptr[0]==x);
  }

  // when evaluated as a bool, it returns false iff buffer overflows
  inline explicit operator bool() const { 
    return index <= buffer_size;
  }

private:
  char* buffer_ptr;
  int32_t buffer_size;
  int32_t index; 
};

//==============================================================================
struct Writer {
  Writer(){
    index = 0;
  }

  // similar to std::cout, it overloads operator<<
  inline Writer& operator<<(const int8_t i8) {
    b[index++] = (char)i8;
    check_buffer_overflow();
    return *this;
  }
  inline Writer& operator<<(const uint8_t u8) { // unsigned
    b[index++] = (char)u8;
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(const int16_t i16) {
    *(int16_t*)(b+index) = i16;
    index += 2;
    check_buffer_overflow();
    return *this;
  }  
  inline Writer& operator<<(const uint16_t u16) { // unsigned
    *(uint16_t*)(b+index) = u16;
    index += 2;
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(const int32_t i32) {
    *(int32_t*)(b+index) = i32;
    index += 4;
    check_buffer_overflow();
    return *this;
  }
  inline Writer& operator<<(const uint32_t u32) { // unsigned
    *(uint32_t*)(b+index) = u32;
    index += 4;
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(const double fl) {
    *(float*)(b+index) = (float)fl;
    index += 4;
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(const int64_t i64) {
    *(int64_t*)(b+index) = i64;
    index += 8;
    check_buffer_overflow();
    return *this;
  }
  inline Writer& operator<<(const uint64_t u64) { // unsigned
    *(uint64_t*)(b+index) = u64;
    index += 8;
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(const string& s) {
    int32_t n = s.length();
    b[index++] = (char)n;
    for (int32_t i = 0; i < n; ++i)
      b[index++] = s[i];
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(const char* c_str) {  // c_string, performace++
    int32_t index_tmp = index;
    int32_t i = 0;
    while (c_str[i]) {
      b[++index] = c_str[i++];
    }
    b[index_tmp] = (char)i;
    ++index;
    check_buffer_overflow();
    return *this;
  }

  inline Writer& operator<<(Writer& writer) {
    long_string_buffer = 
      string(writer.getBufferPtr(), writer.getBufferSize())+long_string_buffer;
    return *this;
  }

  // getters which return buffer start pointer & buffer size, respectively
  inline const char* getBufferPtr() {
    if (long_string_buffer.size() == 0) return b;
    else {
      long_string_buffer.append(b, index);
      index = 0;
      return long_string_buffer.c_str();
    }
  }

  inline int32_t getBufferSize() {
    if (long_string_buffer.size() == 0) return index;
    else {
      if (index != 0) {
        long_string_buffer.append(b, index);
        index = 0;
      }
      return long_string_buffer.size();
    }
  }

  void clear() {
    long_string_buffer.clear();
    index = 0;
  }

private:   
  char b[BUFFER_RESERVE_SIZE];
  int32_t index;
  string long_string_buffer;
  inline void check_buffer_overflow() {
    if (index > BUFFER_RENEW_SIZE) {
      long_string_buffer.append(b, index);
      index = 0;
    }
  }
};
//==============================================================================
} // namespace Buffer

#endif // WORLD2D_IO_BUFFER

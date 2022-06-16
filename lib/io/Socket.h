#ifndef WORLD2D_IO_SOCKET
#define WORLD2D_IO_SOCKET

#include "../std.h"
#include "Buffer.h"

struct Socket;

struct InputPacket {
  shared_ptr<Socket> socket;
  unique_ptr<Buffer::Reader> bin;
  enum Type {
    CONNECT,
    START,
    MESSAGE,
    CLOSE
  } type;

  InputPacket(shared_ptr<Socket> socket_, 
              Type type_,
              unique_ptr<Buffer::Reader> bin_) {
    socket = socket_;
    type = type_;
    bin = std::move(bin_);
  }
};

struct OutputPacket {
  shared_ptr<Socket> socket;
  unique_ptr<Buffer::Writer> bout;
  enum Type {
    MESSAGE,
    CLOSE
  } type;
  OutputPacket(shared_ptr<Socket> socket_, 
               Type type_, 
               unique_ptr<Buffer::Writer> bout_) {
    socket = socket_;
    type = type_;
    bout = std::move(bout_);
  }
};

typedef Queue<unique_ptr<OutputPacket>> OutputPacketQueue;

// SocketManager takes charge of the creation of Socket (stored in shared_ptr)

// Each Socket instance stores an output buffer called ``bout'', managed as a 
// unique_ptr. Whenever invoking <<Socket::flush, {this, bout} will be pushed 
// into output_queue

// For each input message, {this, bin} will be automated pushed into 
// input_queue
struct Socket {
  uint32_t id; // socket id
  // Socket will be manager by shared_ptr, self will be set immediately after 
  // Socket's construction
  weak_ptr<Socket> self; 
  uint32_t player_id;

  enum State {
    OPEN,
    CLOSED
  } state;

  enum Type {
    WEB_SOCKET,
    BOT_SOCKET
  } type;

  // stat
  uint32_t in_packet_count, out_packet_count;
  uint32_t in_traffic_size, out_traffic_size;
  uint64_t last_active_time;

  Socket(int id, Type type, OutputPacketQueue* output_queue)
    : id(id), type(type), output_queue(output_queue) {
    bout = std::move(make_unique<Buffer::Writer>());
    state = OPEN;
    in_packet_count = out_packet_count = 0;
    in_traffic_size = out_traffic_size = 0;
//    last_active_time = now();
    player_id = INVALID_ID;
  }

  virtual ~Socket() {
    // cout << "# " << id << " is removed" << endl;
  }

// ======================== copy buffer writer =============================
  /* TEMPLATE
  template<typename T>
  inline Socket& operator<<(T t) {
    *bout << t;
    return *this;
  }
  */
  // REAL COPIERS
  inline Socket& operator<<(const int8_t t) {
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const uint8_t t) { // unsigned
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const int16_t t) {
    *bout << t;
    return *this;
  }  

  inline Socket& operator<<(const uint16_t t) { // unsigned
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const int32_t t) {
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const uint32_t t) { // unsigned
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const double t) {
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const int64_t t) {
    *bout << t;
    return *this;
  }
  
  inline Socket& operator<<(const uint64_t t) { // unsigned
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const string& t) {
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(const char* t) {  // c_string, performace++
    *bout << t;
    return *this;
  }

  inline Socket& operator<<(Buffer::Writer& writer) {
    *bout << writer;
    return *this;
  }

// ======================= the end of the copy ==============================

  // Similar to std::flush, use *this << Socket::flush to ``send'' the message 
  const static struct Flush {} flush;
  virtual Socket& operator<<(Flush) {
    output_queue->produce(std::move(make_unique<OutputPacket>(
      self.lock(), OutputPacket::MESSAGE, std::move(bout)
    )));
    bout = std::move(make_unique<Buffer::Writer>());
    return *this;
  }

  // use *this << Socket::end to try to close the socket
  // the message buffered but not sent will be delivered
  const static struct End {} end;
  virtual void operator<<(End) {
    output_queue->produce(std::move(make_unique<OutputPacket>(
      self.lock(), OutputPacket::CLOSE, nullptr
    )));
  }

  // the message buffered but not sent will be ignored
  const static struct Halt {} halt;
  virtual void operator<<(Halt) {
    state = Socket::CLOSED;
    output_queue->produce(std::move(make_unique<OutputPacket>(
      self.lock(), OutputPacket::CLOSE, nullptr
    )));
  }

  // used by SocketManager to properly send a message, close
  virtual void send(const char* buffer_ptr, int buffer_size) = 0;
  virtual void close() = 0;

protected:
  OutputPacketQueue* output_queue;
  unique_ptr<Buffer::Writer> bout; 
};

#endif // WORLD2D_IO_SOCKET
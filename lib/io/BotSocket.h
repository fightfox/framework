#ifndef WORLD2D_IO_BOTSOCKET
#define WORLD2D_IO_BOTSOCKET

#include "Socket.h"

struct BotSocket: Socket {
  BotSocket(): Socket(-1, Socket::BOT_SOCKET, nullptr){}
  void send(const char*, int) override {}
  void close() override {}
  Socket& operator<<(Flush) override {
    bout = std::move(make_unique<Buffer::Writer>());
    return *this;
  }
  void operator<<(End) override {}
  void operator<<(Halt) override {}
};

#endif // WORLD2D_IO_BOTSOCKET
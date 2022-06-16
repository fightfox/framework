#ifndef WORLD2D_IO_SOCKET_MANAGER
#define WORLD2D_IO_SOCKET_MANAGER

#include "../std.h"
#include "Socket.h"
#include "WebSocket.h"
#include "BotSocket.h"
#include <uWS/uWS.h>

struct Socket;

struct SocketManager {
  Queue<unique_ptr<InputPacket>> input_queue;    // interface with Game
  Queue<unique_ptr<OutputPacket>> output_queue;  // interface with Socket
  
  SocketManager();
  void start(uint16_t);  
  unique_ptr<InputPacket> pop();
  uint32_t getConnectionCount();
private:
  dense_hash_map<uint32_t, shared_ptr<Socket>> sockets;
  mutex lk;      // lock for sockets
  uWS::Hub hub;  // hub used by uWS
  atomic<int> lastSocketId;
  void setupWebSocketServer();
  void startWebSocketServer(uint16_t port);
};

#endif // WORLD2D_IO_SOCKET_MANAGER

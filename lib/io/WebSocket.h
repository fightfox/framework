#ifndef WORLD2D_IO_WEBSOCKET
#define WORLD2D_IO_WEBSOCKET

#include "Socket.h"
#include <uWS/uWS.h>

struct WebSocket: Socket {
  uWS::WebSocket<uWS::SERVER>* ws;
  WebSocket(int id, OutputPacketQueue* output_queue, uWS::WebSocket<uWS::SERVER>
    *ws): Socket(id, Socket::WEB_SOCKET, output_queue), ws(ws) {}
  void send(const char *buffer_ptr, int buffer_size) {
    if (state == OPEN)
      ws->send(buffer_ptr, buffer_size, uWS::BINARY);
  }
  void close() {
    if (state == OPEN)
      ws->close();
  }
};

#endif // WORLD2D_IO_WEBSOCKET
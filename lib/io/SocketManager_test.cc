#include "SocketManager.h"

int main() {
  cout.sync_with_stdio(0);
  cout.exceptions(cout.failbit);
  SocketManager socket_manager;
  socket_manager.start(3000);
  auto& q = socket_manager.input_queue;
  while (1) { // endless loop of the server
    while (auto packet = q.consume()) { 
      if (packet->type == InputPacket::MESSAGE) {
        auto& socket = *(packet->socket);
        auto& bin = *(packet->bin);
        int8_t ch;
        bin >> ch;
        socket << (string("Hello")+char(ch));
        for (uint32_t i = 0; i < 100; ++i )
          socket << i;
        socket << Socket::flush;
        if (socket.in_packet_count >= 100) {
          if (rand01() < 0.01) cout << socket.out_traffic_size << " " <<
            socket.out_packet_count << endl;
          socket << Socket::end;
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
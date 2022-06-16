#include "SocketManager.h"

SocketManager::SocketManager(){
  sockets.set_empty_key(-1);
  sockets.set_deleted_key(-2);
  lastSocketId = 0;
  setupWebSocketServer();
}

void SocketManager::start(uint16_t port) {
  startWebSocketServer(port);
}

unique_ptr<InputPacket> SocketManager::pop() {
  unique_ptr<InputPacket> top = input_queue.consume();
  // simple client:
  // auto is unique_ptr<InputPacket> with two members: shared_ptr<Socket> socket and unique_ptr<Buffer::Reader> bin
  while (top!=nullptr && top->type==InputPacket::CONNECT)
    top = std::move(input_queue.consume());
  if (top!=nullptr && top->bin && top->bin->checkHeader(0))
    top->type = InputPacket::START;
  // end here
  return std::move(top);
}

uint32_t SocketManager::getConnectionCount() {
  return sockets.size();
}

void SocketManager::setupWebSocketServer() {
  // when a new socket comes
  hub.onConnection([this](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) 
  {
    auto socket= make_shared<WebSocket>(++lastSocketId, &output_queue, ws);
    socket->self = socket;
    lk.lock();
    sockets[socket->id] = socket;
    lk.unlock();
    ws->setUserData((void*)(int64_t)socket->id);
    input_queue.produce(std::move(make_unique<InputPacket>(
      socket, InputPacket::CONNECT, nullptr
    )));
  });
  // when receiving a message from a socket
  hub.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char* buffer_ptr, 
    size_t buffer_size, uWS::OpCode opCode) {
    uint32_t id = (int64_t) ws->getUserData();
    lk.lock();
    auto& socket = sockets[id];
    lk.unlock();
    if (!socket) {
      cerr << "ERROR: SOCKET NOT FOUND ON MESSAGE!" << endl; 
      return;
    }
    ++socket->in_packet_count;
    socket->in_traffic_size += buffer_size;
    //socket->last_active_time = now();
    input_queue.produce(std::move(make_unique<InputPacket>(
      socket, 
      InputPacket::MESSAGE,
      std::move(make_unique<Buffer::Reader>(buffer_ptr, buffer_size))
      
    )));
  });
  // when a socket leaves
  hub.onDisconnection([this](uWS::WebSocket<uWS::SERVER> *ws, uint32_t code, 
    char *message, size_t length) {
    uint32_t id = (int64_t) ws->getUserData();
    lk.lock();
    auto& socket = sockets[id];
    lk.unlock();
    if (!socket) {
      cerr << "ERROR: SOCKET NOT FOUND ON DISCONNECTION!" << endl; 
      return;
    }
    socket->state = Socket::CLOSED;
    input_queue.produce(std::move(make_unique<InputPacket>(
      socket, InputPacket::CLOSE, nullptr
    )));
    sockets.erase(id);
  });
  // set the timer which acutally sends packets in output_queue
  Timer *timer = new Timer(hub.getLoop());
  timer->setData(&output_queue);
  timer->start([](Timer *timer) {
    auto output_queue = (OutputPacketQueue*) timer->getData();
    while (auto packet = output_queue->consume()) { 
      auto& socket = *(packet->socket);
      auto type = packet->type;
      auto& bout = *(packet->bout);
      if (type == OutputPacket::MESSAGE) {
        ++socket.out_packet_count;
        socket.out_traffic_size += bout.getBufferSize();
        socket.send(bout.getBufferPtr(), bout.getBufferSize());
      }
      else if (type == OutputPacket::CLOSE)
        socket.close();
    }
  }, 100, 5);  // ʕ•ᴥ•ʔ timeout, time_interval (ms).
}

void SocketManager::startWebSocketServer(uint16_t port) {
  thread th([this, port](){
    hub.listen(port);
    hub.run();
  });
  th.detach();
}
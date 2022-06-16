#include <uWS/uWS.h>
#include "../std.h"

struct Client {
	uint32_t id;
	uWS::WebSocket<uWS::SERVER>* ws;
	uint32_t sum;
	// mutex lk;
  void handle_message(char* message, int length) {}
  void handle_connect() {}
  void handle_disconnect() {}
  void send(char *message, int length) {
  	ws->send(message, length, uWS::BINARY);
  }
};

struct ClientManager {
	bool running;
	uint32_t sum;
	ClientManager();
	void start(uint32_t _port);
	void foreach(function<void(Client*)>);
	void broadcast(char* message, int length);
private:
	atomic<uint32_t> lastClientId;
	unordered_map<uint32_t, Client*> clients;
	uWS::Hub hub;		    // hub used by uWS
	uint32_t port;				// the port hub is listening to
};

void ClientManager::broadcast(char* message, int length) {
	foreach([message, length](Client* client){
		client->send(message, length);
	});
}

ClientManager::ClientManager() {
	lastClientId = 0;
	running = false;
	// when a new client comes
	hub.onConnection([this](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
		cout << "[LINE " << __LINE__ << "]" << "connection" 
				 << " ip: " << ws->getAddress().address << endl;
		Client* client = new Client({lastClientId++, ws, 0});
		clients[client->id] = client;
		ws->setUserData((void*) ((int64_t)client->id));
    client->handle_connect();
	});
	// when receiving a message from a client
	hub.onMessage([this](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
		uint32_t id = (int64_t) ws->getUserData();
    clients[id]->handle_message(message, length);
    broadcast(message, length);
	});
	// when a client leaves
	hub.onDisconnection([this](uWS::WebSocket<uWS::SERVER> *ws, uint32_t code, char *message, size_t length) {
		uint32_t id = (int64_t) ws->getUserData();
		clients[id]->handle_disconnect();
		clients.erase(id);
	});
}

void ClientManager::start(uint32_t _port) {
	sum = 0;
	port = _port;
	hub.listen(port);
	running = true;
	hub.run();
}

void ClientManager::foreach(function<void(Client*)> fn) {
	vector<Client*> cls;
	{
		for (auto pr : clients) {
			cls.push_back(pr.second);
		}
	}
	for (auto client : cls) {
		fn(client);
	}
}

ClientManager cm;

int main() {
	setInterval([](){
		if (cm.running) {
			cm.foreach([](Client* client){
			});
		}
	}, 40);
	cm.start(3000);
	return 0;
}

#ifndef WORLD2D_ENGINE_GAME
#define WORLD2D_ENGINE_GAME

#include "../std.h"
#include "UnitList.h"
#include "PlayerList.h"
#include "CooldownQueue.h"
#include "../util/Animation.h"
#include "../util/CollisionQueue.h"
#include "../util/XY.h"
#include "../util/LinearSolver.h"
#include "../io/SocketManager.h"
#include "../event/events.h"
#include "../event/EventQueue.h"
#include "../util/Canvas.h"
#include "../util/Camera.h"
#include "../util/Vision.h"
#include "../util/View.h"

enum class CollisionType : uint8_t {
  NONE,
  DETECTABLE,
  REPULSIVE,
  REPULSIVE_DETECTABLE
};

enum class BotOperation : uint8_t {
	START,
	PLAY,
	DEATH,
	ASSIGN
};

struct CollisionNode {
	uint32_t id; 
	XY v;
	double mass;
	Figure* figure;
	vector<pair<uint32_t, double>> edges;
	CollisionNode() {}
	CollisionNode(uint32_t id, const XY& v, double mass, Figure* figure) : id(id), v(v), mass(mass), figure(figure) {
		edges.clear();
	}
};

struct CollisionEdge {
	uint32_t id;
	uint32_t nodes[2];
	XY n_v;
	uint32_t variable_id;
	bool assoicated;
	CollisionEdge(uint32_t id, uint32_t from, uint32_t to, const XY& n_v) : id(id), n_v(n_v) {
		nodes[0] = from;
		nodes[1] = to;
		assoicated = false;
	}
};

struct Game {

	Time start_time;
	Time now;
	Time tick_head;

	EventQueue event_queue;
	UnitList unit_list;	
	PlayerList player_list;
	SocketManager socket_manager;
	CooldownQueue cooldown_queue;
	unordered_set<uint32_t> bot_list;

	Canvas canvas;
	Vision vision;
	Camera camera;
	View view;
	
	function<CollisionType(uint32_t, uint32_t)> getCollisionType;
	function<void(uint32_t, BotOperation)> fn_bot;

	vector<function<void(Init*)>> fns_init;
	vector<function<void(UpdateWorld*)>> fns_update_world;
	vector<function<void(OnCollision*)>> fns_on_collision;
	function<void(Start*)> fn_start;
	function<void(Command*)> fn_command;
	function<void(Close*)> fn_close;
	vector<function<void(RemoveUnit*)>> fns_remove_unit;
	vector<function<void(CreateUnit*)>> fns_create_unit;
	vector<function<void(CreatePlayer*)>> fns_create_player;
	function<void(End*)> fn_end;
	vector<function<void(RemovePlayer*)>> fns_remove_player;

	Animation animation;
	CollisionQueue collision_queue;
	unordered_set<int64_t> collision_marker;

	//============================ system driver ===================================

	Game();
	void mainLoop();
	inline void setupCollision(const function<CollisionType(uint32_t, uint32_t)>&);
	inline void setupBot(const function<void(uint32_t, BotOperation)>&);

	//============================ friendly port ===================================
	
	inline Time getTime();
	inline void idle(Time);
	inline XY getXY(uint32_t);
	inline double getR(uint32_t);
	inline Figure* getFigure(uint32_t);
	inline int32_t getUnitCount();
	inline int32_t getPlayerCount();
	inline uint32_t observe(uint32_t, uint8_t, uint32_t, uint8_t);
	inline bool isAlive(uint32_t);
	inline uint32_t addCooldown(Cooldown*);
	inline void modifyCooldown(uint32_t, Time);
	inline void removeCooldown(uint32_t);
	inline Unit* getUnit(uint32_t);
	inline Player* getPlayer(uint32_t);
	inline uint32_t getNewPlayerId();
	inline void disconnect(shared_ptr<Socket>&);
	inline bool collisionRepulsive(uint32_t, uint32_t);
	inline bool isPlayer(uint32_t);
	inline uint8_t getCameraPriv(uint32_t);
	inline bool isVisible(uint32_t);
	inline uint8_t getTimeDelta();
	
	//============================ triggers family =================================
	void trigger(function<void(Init*)>);
	void trigger(function<void(UpdateWorld*)>);
	void trigger(function<void(OnCollision*)>);
	void trigger(function<void(Start*)>);
	void trigger(function<void(Command*)>);
	void trigger(function<void(Close*)>);
	void trigger(function<void(RemoveUnit*)>);
	void trigger(function<void(CreateUnit*)>);
	void trigger(function<void(CreatePlayer*)>);
	void trigger(function<void(End*)>);
	void trigger(function<void(RemovePlayer*)>);

	//=========================== messages family ==================================
	Socket& output(uint32_t, uint8_t); 
	void sendShow(uint32_t, uint32_t); 	// 1
	void sendMove(uint32_t, uint32_t);	// 2
	void sendCreateUnit(uint32_t, uint32_t); // 3
	void sendRemovePlayer(uint32_t, uint32_t); // 4
	void sendCreatePlayer(uint32_t); // 5
	void sendRemoveUnit(uint32_t, uint32_t); // 6
	void sendChangeUnit(uint32_t, uint32_t); // 7
	void sendChangeR(uint32_t, uint32_t); // 8
	void sendChangeXY(uint32_t, uint32_t); // 9	
	void sendHide(uint32_t, uint32_t); // 10
	void sendAll(uint32_t); // 99

	//============================ actions family ==================================
	uint32_t createUnit(Unit*, Figure*);
	void moveUnit(uint32_t, Time);
	void updateCamera(uint32_t);
	void updateView(bool);
	uint32_t createPlayer(Player*);
	void showPlayer(uint32_t);
	bool removeUnit(uint32_t);
	void changeUnit(uint32_t, Figure*);
	void changeR(uint32_t, double);
	void changeXY(uint32_t, XY);
	void changeV(uint32_t, XY);
	bool removePlayer(uint32_t);
	void createBot(unique_ptr<Buffer::Reader>);
	void assignBot(uint32_t);
	void visiblize(uint32_t);
	void invisiblize(uint32_t);
	void update(uint32_t, uint8_t, uint8_t, const function<void(Socket&)>&);
	void update(uint8_t, const function<void(Socket&)>&);
	void update(uint32_t, uint8_t);
	

	//============================ collision family ==================================

	void updateCollisionPair(uint32_t, uint32_t);
	void updateCollision(uint32_t);
	void evolveTrace(uint32_t);
	void advanceEvolveCollision(uint32_t, uint32_t);
	void addCollisionEvent(uint32_t, uint32_t);
	void initTrace(uint32_t);
};

//=========================== friendly port (implement) ============================

inline void Game::setupCollision(const function<CollisionType(uint32_t, uint32_t)>& fn) {
	getCollisionType = fn;
}

inline void Game::setupBot(const function<void(uint32_t, BotOperation)>& fn) {
	fn_bot = fn;
}

inline Time Game::getTime() {
	using namespace std::chrono;
	milliseconds ms = duration_cast< milliseconds >(
	system_clock::now().time_since_epoch()
	);
	return (Time)ms.count()-start_time;
}

inline void Game::idle(Time t) {
	std::this_thread::sleep_for(std::chrono::milliseconds((int64_t)t));
}

inline XY Game::getXY(uint32_t id) {
	auto tr = animation.get(id);
	if (tr==nullptr)
		return XY();
	else 
  	return tr->evolve(now)->p;
}

inline double Game::getR(uint32_t id) {
	auto tr = animation.get(id);
	if (tr==nullptr)
		return -1;
  else
  	return tr->figure->r;
}

inline Figure* Game::getFigure(uint32_t id) {
	auto tr = animation.get(id);
	if (tr==nullptr)
		return nullptr; 
	else
		return tr->evolve(now); 
}

inline int32_t Game::getUnitCount() {
	return unit_list.units.size()-unit_list.dying_count;
}

inline int32_t Game::getPlayerCount() {
	return player_list.players.size()-player_list.dying_count;
}

inline uint32_t Game::observe(uint32_t me, uint8_t my_priv, uint32_t you, uint8_t ur_priv) {
	view.observe(Sensor(me, my_priv), Sensor(you, ur_priv));
	updateView(true);
	return you;
}

inline bool Game::isAlive(uint32_t id) {
	if (id&1) {
		auto player = player_list.get(id); 
		return player!=nullptr && player->alive;
	}
	else {
		auto unit = unit_list.get(id);
		return unit!=nullptr && unit->alive;
	}
}

inline uint32_t Game::addCooldown(Cooldown* cd) {
	return cooldown_queue.insert(cd);
}

inline void Game::modifyCooldown(uint32_t cd_id, Time time) {
	cooldown_queue.modify(cd_id, time);
}

inline void Game::removeCooldown(uint32_t cd_id) {
	cooldown_queue.remove(cd_id);	
}

inline Unit* Game::getUnit(uint32_t id) {
	return unit_list.get(id);
}

inline Player* Game::getPlayer(uint32_t id) {
	return player_list.get(id);
}

inline uint32_t Game::getNewPlayerId() {
	return (player_list.last_player_id+=2);
}

inline void Game::disconnect(shared_ptr<Socket>& socket) {
	*socket << Socket::end;
}

// the followings are not for the scripts

inline bool Game::collisionRepulsive(uint32_t id1, uint32_t id2) {
	auto collision_type = getCollisionType(id1, id2);
	return collision_type==CollisionType::REPULSIVE || collision_type==CollisionType::REPULSIVE_DETECTABLE;
}

inline bool Game::isPlayer(uint32_t id) {
	return id & 1;
}

inline uint8_t Game::getCameraPriv(uint32_t player_id) {
	return player_list.get(player_id)->own_priv+1;
}

inline bool Game::isVisible(uint32_t unit_id) {
	return unit_list.get(unit_id)->visible;
}

inline uint8_t Game::getTimeDelta() {
	if (now < tick_head)
		return 0;
	else if (now > tick_head + 254)
		return 254;
	else
		return (int32_t)(now-tick_head);
}

#endif // WORLD2D_ENGINE_GAME

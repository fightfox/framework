#include "../lib/engine/Game.h"
#include "../lib/script.h"

const int32_t MAXIMUM_BOT_AMOUNT = 800;//400;
const int32_t PEOPLE_AMOUNT = 400;
const int32_t PLAYER_AMOUNT = MAXIMUM_BOT_AMOUNT + PEOPLE_AMOUNT;
const double MAP_LENGTH = 6000; //6000;
const double TICK_TIME = 40;
const double BODY_INIT_RADIUS = 20;
const double WALL_THICKNESS = 3;
const double ZERO_SPEED_ZONE = 10;
const double MAXIMUM_SPEED_ZONE = 100;
const double BODY_SPEED = 0.125;
const double BULLET_SPEED = 0.2;
const double RECALL_RADIUS = 10;
const double RECALL_SPEED = 1.25;
const double FOOD_DENSITY = 0.00004;
const double MAXIMUM_FOOD_AMOUNT = FOOD_DENSITY*4*MAP_LENGTH*MAP_LENGTH;
const double FOOD_RATE = 0.1;
const double FOOD_RADIUS = 10;
const double RECALL_CD = 2000;
const double BOOST_CD = 1000;
const double PROBABLITY_RED = 0.15;
const double PROBABLITY_BLUE = 0.15;
const double PROBABLITY_GREEN = 0.7;
const double MINIMUM_BULLET_RADIUS = 20;
const double BOT_SPAWNING_RATE = 0.1;

using CT = CollisionType;
const CT CT_RULES[5][5] = {
//BODY              BULLET            RECALL            WALL            FOOD        
  {CT::REPULSIVE,   CT::DETECTABLE,   CT::DETECTABLE,   CT::REPULSIVE,  CT::DETECTABLE},
  {CT::DETECTABLE,  CT::DETECTABLE,   CT::NONE,         CT::REPULSIVE,  CT::DETECTABLE},
  {CT::DETECTABLE,  CT::NONE,         CT::NONE,         CT::NONE,       CT::NONE},
  {CT::REPULSIVE,   CT::REPULSIVE,    CT::NONE,         CT::NONE,       CT::REPULSIVE},
  {CT::DETECTABLE,  CT::DETECTABLE,   CT::NONE,         CT::REPULSIVE,  CT::REPULSIVE}
};

Game* g;

enum Unit::Type: uint8_t {
  BODY, BULLET, RECALL, WALL, FOOD
};

enum Player::Type: uint8_t {
  FIGHTER, SPECTATE
};

struct Body : Unit {
  uint32_t owner;
  Time boost_until;
  Body(uint32_t owner_) : Unit(BODY, 1), owner(owner_), boost_until(-1) {
    visible = false;
  }
  void toMessage(uint8_t priv, Socket& socket) {
    if (priv==1) socket << owner;
  }
};

Body* getBody(uint32_t unit_id) {
  return (Body*)(g->getUnit(unit_id));
}

struct Bullet : Unit {
  uint32_t owner;
  Time boost_until;
  Bullet(uint32_t owner_) : Unit(BULLET, 1), owner(owner_), boost_until(-1) {}
  void toMessage(uint8_t priv, Socket& socket) {
    if (priv==1) socket << owner;
  }
};

Bullet* getBullet(uint32_t unit_id) {
  return (Bullet*)(g->getUnit(unit_id));
}

struct Recall : Unit {
  uint32_t owner;
  Recall(uint32_t owner_, XY vv) : Unit(RECALL, 1), owner(owner_) {
    v = vv.normalized()*RECALL_SPEED;
  }
  void toMessage(uint8_t priv, Socket& socket) {
    if (priv==1) socket << owner;
  }
};

Recall* getRecall(uint32_t unit_id) {
  return (Recall*)(g->getUnit(unit_id));
}

struct Wall : Unit {
  Wall() : Unit(WALL, 1) {
    mass = INFINITY_MASS;
    //visible = false;
  }
  void toMessage(uint8_t priv, Socket& socket) {}
};

struct Food : Unit {
  enum FoodType : uint8_t {
    RED, BLUE, GREEN
  } food_type;
  Food(FoodType food_type) : Unit(FOOD, 1), food_type(food_type) {}
  Food(double p0, double p1, double p2) : Unit(FOOD, 1) {
    double rd = rand01();
    if (rd < p0)
      food_type = RED;
    else if (rd < p0+p1)
      food_type = BLUE;
    else
      food_type = GREEN;
  }
  void toMessage(uint8_t priv, Socket& socket) {
    if (priv==1) socket << (uint8_t)food_type;
  }
};

Food* getFood(uint32_t unit_id) {
  return (Food*)(g->getUnit(unit_id));
}

struct Fighter : Player {
  uint32_t my_body, my_bullet, my_recall;
  uint8_t state;
  string nickname;
  uint32_t recall_cd_id;
  Fighter(uint32_t player_id, shared_ptr<Socket>& socket, uint32_t my_body_, string nickname_) 
  : Player(FIGHTER, player_id, socket, 2), my_body(my_body_), nickname(nickname_) {
    my_bullet = -1;
    my_recall = -1;
    recall_cd_id = -1;
    state = 0;
  }
  void toMessage(uint8_t priv, Socket& socket) {
    if (priv==1) {
      socket << state << nickname;
    }
    if (priv==2) {
      socket << my_body << my_bullet << my_recall;
    }
  }
  void changeV(XY p) {
    if (state==2)
      return;
    double p_len = p.norm();
    double speed = state==0 ? BODY_SPEED : BULLET_SPEED;
    if (p_len < ZERO_SPEED_ZONE)
      p_len = 0;
    else if (p_len < MAXIMUM_SPEED_ZONE)
      p_len = (p_len-ZERO_SPEED_ZONE)/(MAXIMUM_SPEED_ZONE-ZERO_SPEED_ZONE)*speed;
    else
      p_len = speed;
    if (state==0) {
      double boost_factor = 1;
      if (g->now<getBody(my_body)->boost_until)
        boost_factor = 2;
      g->changeV(my_body, p.normalized()*p_len*boost_factor);
    }
    else {
      double boost_factor = 1;
      if (g->now<getBody(my_bullet)->boost_until)
        boost_factor = 2;
      g->changeV(my_bullet, p.normalized()*p_len*boost_factor);    
    }
  }
  BB getBB() {
    if (state==0)
      return getVisionBB(g->getXY(my_body));
    else if (state==1)
      return getVisionBB(g->getXY(my_bullet));
    else
      return getVisionBB(g->getXY(my_recall));
  }
};

Fighter* getFighter(uint32_t player_id) {
  return (Fighter*)(g->getPlayer(player_id));
}

struct RecallCooldown : Cooldown {
  RecallCooldown(uint32_t player_id) : Cooldown(player_id, g->now+RECALL_CD) {}
  void handle();
};


CollisionType collisionRule(uint32_t id1, uint32_t id2) {
  return CT_RULES[g->getUnit(id1)->type][g->getUnit(id2)->type];
}

void init_add_wall(Init* init) {
  XY p0(MAP_LENGTH, MAP_LENGTH),
    p1(MAP_LENGTH, -MAP_LENGTH),
    p2(-MAP_LENGTH, -MAP_LENGTH),
    p3(-MAP_LENGTH, MAP_LENGTH);
  g->createUnit(new Wall(), new Figure::Segment(p0, p1, WALL_THICKNESS));
  g->createUnit(new Wall(), new Figure::Segment(p1, p2, WALL_THICKNESS));
  g->createUnit(new Wall(), new Figure::Segment(p2, p3, WALL_THICKNESS));
  g->createUnit(new Wall(), new Figure::Segment(p3, p0, WALL_THICKNESS));
}

void spawnPlayer(uint32_t player_id, shared_ptr<Socket>& socket, const string& my_name) {
  auto body = new Body(-1);
  g->createUnit(body, new Figure::Circle(randXY(MAP_LENGTH), BODY_INIT_RADIUS));
  g->createPlayer(new Fighter(player_id, socket, body->id, my_name)); 
  g->observe(body->id, 1, player_id, 1);
  body->owner = player_id; 
  g->observe(player_id, 2, body->id, 1);
  g->update(body->id, 1, 105, [body, player_id](Socket& socket) {
    socket << body->id << player_id;
  });
}

void handle_start(Start* start) {
  uint8_t case_value;
  string my_name;
  auto& reader = *(start->reader);
  reader >> case_value >> my_name;
  spawnPlayer(start->id, start->socket, my_name);
}

void update_fire(uint32_t player_id, uint32_t bullet_id) {
  auto player = getFighter(player_id);
  player->state = 1;
  player->my_bullet = bullet_id;
  auto bullet = getBullet(bullet_id);
  bullet->owner = player_id;
  g->update(player_id, 2, 101, [player_id, bullet_id](Socket& socket) {
    socket << player_id << bullet_id;
  });
  g->update(player_id, 1, 103, [player_id](Socket& socket) {
    socket << player_id;
  });
}

void update_recall(uint32_t player_id, uint32_t recall_id) {
  auto player = getFighter(player_id);
  player->state = 2;
  player->my_bullet = -1;
  player->my_recall = recall_id;
  auto recall = getRecall(recall_id);
  recall->owner = player_id;
  g->update(player_id, 1, 102, [player_id, recall_id](Socket& socket) {
    socket << player_id << recall_id;
  });
}

void move_player(Fighter* player, XY p) {
  player->changeV(p);
}

void fire_bullet(Fighter* player) {
  if (player->state==0) {
    uint32_t bullet_id = g->createUnit(new Bullet(player->id),
      new Figure::Circle(g->getXY(player->my_body), g->getR(player->my_body)));
    update_fire(player->id, bullet_id);
    g->changeV(player->my_body, XY(0,0));
    player->recall_cd_id = g->addCooldown(new RecallCooldown(player->id));
    g->visiblize(player->my_body);
  }
}

void recall_bullet(Fighter* player) {
  if (player->state==1) {
    XY body_p = g->getXY(player->my_body), bullet_p = g->getXY(player->my_bullet);
    uint32_t recall_id = g->createUnit(new Recall(player->id, body_p-bullet_p),
      new Figure::Circle(bullet_p, RECALL_RADIUS));
    g->removeUnit(player->my_bullet);
    update_recall(player->id, recall_id);
    g->removeCooldown(player->recall_cd_id);
  }
}

void RecallCooldown::handle() {
  recall_bullet(getFighter(id));
}

void generate_food(XY p) {
  g->createUnit(new Food(0.3, 0.3, 0.4), new Figure::Circle(p, 50));
}

void handle_command(Command* cmd) {
  auto& reader = *(cmd->reader);
  uint8_t case_id;
  reader >> case_id;
  auto player = getFighter(cmd->player_id);
  double x, y;
  switch (case_id) {
    case 1:
    {
      reader >> x >> y;
      move_player(player, XY(x, y));
      break;
    }
    case 2:
    {
      fire_bullet(player);
      break;
    }
    case 3:
    {
      recall_bullet(player);
      break;
    }
    case 4:
    {
      reader >> x >> y;
      generate_food(XY(x, y)+g->getXY(player->my_body));
    }
  }
}

uint8_t getUnitType(uint32_t unit_id) {
  return (uint8_t)(g->getUnit(unit_id)->type);
}

void updateRebody(Fighter* player) {
  player->my_recall = -1;
  player->state = 0;
  g->update(player->id, 1, 104, [player](Socket& socket) {
    socket << player->id; 
  });
  g->invisiblize(player->my_body);
}

double getNewR(double old_r, double killed_r) {
  double a = old_r, b = killed_r, d = 2.5;
  if (b <= 15)
    return pow(pow(a, d)+500, 1/d);
  else 
    return pow(pow(a, d)+pow(b, d)/d + 2000, 1/d);
}

void updateKill(Fighter* killer, Fighter* killed) {
  g->changeR(killer->my_body, getNewR(g->getR(killer->my_body), g->getR(killed->my_body)));
}

void updateEat(Fighter* killer, Food* food) {
  g->changeR(killer->my_body, getNewR(g->getR(killer->my_body), g->getR(food->id)));
  g->removeUnit(food->id);
}

void handle_collision(OnCollision* oc) {
  uint32_t id1 = oc->id1, id2 = oc->id2;
  if (getUnitType(id1)>getUnitType(id2))
    std::swap(id1, id2);
  if (getUnitType(id1)==0 && getUnitType(id2)==2) {
    auto player = getFighter(getBody(id1)->owner);
    if (player->id != getRecall(id2)->owner)
      return;
    g->removeUnit(player->my_recall);
    updateRebody(player);
  }
  else if (getUnitType(id1)==0 && getUnitType(id2)==1) {
    auto killed = getFighter(getBody(id1)->owner);
    auto killer = getFighter(getBody(id2)->owner);
    if (killed == killer)
      return;
    updateKill(killer, killed);
    g->removePlayer(killed->id);
  }
  else if (getUnitType(id1)==0 && getUnitType(id2)==4) {
    auto body = getBody(id1);
    auto food = getFood(id2);
    auto food_type = food->food_type;
    updateEat(getFighter(body->owner), food);
    if (food_type==Food::RED)
      body->boost_until = g->now+BOOST_CD;
    else if (food->food_type==Food::BLUE) {

    }
  }
  else if (getUnitType(id1)==1 && getUnitType(id2)==4) {
    auto bullet = getBullet(id1);
    auto food = getFood(id2);
    auto food_type = food->food_type;
    auto player = getFighter(bullet->owner);
    updateEat(player, food);
    if (food_type==Food::RED)
      bullet->boost_until = g->now+BOOST_CD;
    else if (food_type==Food::BLUE) {
      g->modifyCooldown(player->recall_cd_id, g->now+RECALL_CD);
    }
  }
  else if (getUnitType(id1)==1 && getUnitType(id2)==1)
  {
    double r1 = g->getR(id1), r2 = g->getR(id2);
    if (r1<r2) {
      std::swap(id1, id2);
      std::swap(r1, r2);
    }
    recall_bullet(getFighter(getBullet(id2)->owner));
    r1 -= r2;
    if (r1==0)
      recall_bullet(getFighter(getBullet(id1)->owner));
    else
      g->changeR(id1, max(r1, MINIMUM_BULLET_RADIUS));
  }
}

void handle_dying(RemovePlayer* ru) {
  auto killed = getFighter(ru->player_id);
  if (killed->state==1)
    g->removeUnit(killed->my_bullet);
  if (killed->state==2)
    g->removeUnit(killed->my_recall);
  g->removeUnit(killed->my_body);
}
 
void handle_close(Close* close) {
  //g->removePlayer(close->player_id);
  g->assignBot(close->player_id);
}

void add_food(UpdateWorld* uw) {
  int32_t tmp = g->getUnitCount();
  int32_t k = (MAXIMUM_FOOD_AMOUNT-tmp)*FOOD_RATE;
  if (k < tmp && k == 0) k = 5;
  for (int32_t i = 0; i < k; i++)
    g->createUnit(new Food(PROBABLITY_RED, PROBABLITY_BLUE, PROBABLITY_GREEN),
      new Figure::Circle(randXY(MAP_LENGTH), FOOD_RADIUS));
}

struct Bot {
  double dir, adir;
  Bot(): dir(0), adir(0) {}
};
unordered_map<uint32_t, Bot*> bots;

void handle_bot(uint32_t player_id, BotOperation opt) {
  if (opt==BotOperation::START || opt==BotOperation::ASSIGN) {
    bots[player_id] = new Bot();
    return;
  }
  if (opt==BotOperation::DEATH) {
    delete bots[player_id];
    bots.erase(player_id);
    return;
  }
  auto player = getFighter(player_id);
  auto bot = bots[player_id];
  bot->dir += bot->adir;
  bot->adir += (rand01()-0.5) / 100;
  bot->adir = std::max(-1.0, std::min(bot->adir, 1.0));  
  if (player->state==0) {
    move_player(player, polarXY(bot->dir, 200));
    if (rand01() < 1.0/125)
      fire_bullet(player);
  }
  else if (player->state==1) {
    move_player(player, polarXY(bot->dir, 200));
    if (rand01() < 1.0/50)
     recall_bullet(player);
  }
}

void add_bots(UpdateWorld* uw) {
  int32_t tmp = g->getPlayerCount();
  int32_t total_bot = min(MAXIMUM_BOT_AMOUNT, PLAYER_AMOUNT-tmp+(int32_t)bots.size());
  int32_t k = std::ceil((total_bot-bots.size())*BOT_SPAWNING_RATE);
  for (int32_t i = 0; i < k; i++) {
    g->createBot(std::move(getNicknameBuffer("hahahehehehehaha")));
  }    
}

void send_ticktime(UpdateWorld* uw) {
  g->update(201, [](Socket& socket) {
    socket << (double)(g->now) << (uint32_t)(g->unit_list.unit_last_id)
      << (uint32_t)(g->player_list.last_player_id)
      << (uint32_t)(g->getPlayerCount())
      << (uint32_t)(g->socket_manager.getConnectionCount());
  });
}

void handle_end(End* end) {
  spawnPlayer(g->getNewPlayerId(), end->socket, ((Fighter*)(end->player))->nickname);
}

int main() {
  g = new Game();
  g->setupCollision(collisionRule);
  g->trigger(init_add_wall);
  g->trigger(handle_start);
  g->trigger(handle_command);
  g->trigger(handle_close);
  g->trigger(handle_collision);
  g->trigger(handle_dying);
  g->trigger(add_food);
  g->trigger(add_bots);
  g->setupBot(handle_bot);
  g->trigger(send_ticktime);
//  g->trigger(handle_end);
  g->mainLoop();

  /* TODO LIST
  1. fast advance collision for two objects (done)
  2. time twist in main loop to process update world earlier (no need)
  3. g->inform(entity_id, entity_priv) eager update based on to-message (done cc, to do js)
  4. invisible unit and netural unit for to-all update
  */
}
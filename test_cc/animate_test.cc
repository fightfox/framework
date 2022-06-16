#include "../lib/io/SocketManager.h"
#include "../lib/util/Figure.h"
#include "../lib/util/locate.h"
#include "../lib/engine/Game.h"

Game* g;

enum Unit::Type: uint8_t {
  TEST, WALL
};

struct TestUnit: Unit {
  uint32_t owner;
  TestUnit(double dir): Unit(Unit::TEST, 1), owner(-1) {
    v = XY(sin(dir), cos(dir)) * (0.01 / 12);
    mass = 1;  
  }
  void toMessage(uint8_t priv, Socket& socket) override {
    if (priv==1)
      socket << owner;
  }
};

struct TestWall: Unit {
  TestWall(): Unit(Unit::WALL, 1) {
    v = XY(0, 0);
    mass = INFINITY_MASS;
  }
  void toMessage(uint8_t priv, Socket& socket) override {}
};

enum Player::Type: uint8_t {
  TESTER
};

struct TestPlayer : Player {
  uint32_t my_unit;
  string nickname; 
  TestPlayer(uint32_t id, shared_ptr<Socket>& socket_, string str, uint32_t unit_id) : Player(TESTER, id, socket_, 2), nickname(str) {
    my_unit = unit_id;
  }
  BB getBB() override {
    XY center = g->getXY(my_unit);
    return BB(center.x-SCREEN_HALF_WIDTH, center.y-SCREEN_HALF_HEIGHT,
              center.x+SCREEN_HALF_WIDTH, center.y+SCREEN_HALF_HEIGHT);
  }
  void toMessage(uint8_t priv, Socket& socket) override {
    if (priv==1)
      socket << nickname;
    if (priv==2)
      socket << my_unit;
  }
};

struct Split : Cooldown {
  Split(uint32_t id, Time time) : Cooldown(id, g->now+time) {}
  void handle() {
    double dir = rand01()*PI*2;
    double rr = g->getR(id);
    g->createUnit(new TestUnit(dir), 
      new Figure::Circle(g->getXY(id)+XY(sin(dir), cos(dir))*rr*3, rr));
  }
};

void handle_init1(Init* init) {
  g->createUnit(new TestWall(), new Figure::Segment(XY(50, 50), XY(800, 800), 20));
}

void handle_init2(Init* init) {
  g->createUnit(new TestWall(), new Figure::Segment(XY(50, 800), XY(800, 50), 20));
}

void handle_start(Start* start) {
  uint8_t case_value;
  auto& reader = *(start->reader);
  string my_name;
  reader >> case_value >> my_name;
  auto unit = new TestUnit(rand01()*PI*2);
  unit->owner = start->id;
  g->createUnit(unit, new Figure::Circle(XY(300, 300), 20));
  g->createPlayer(new TestPlayer(start->id, start->socket, my_name, unit->id)); 
  g->observe(unit->id, 1, start->id, 1); 
  g->updateView(); // TODO
}

void handle_update_world(UpdateWorld* uw) {
  if (g->getPlayerCount()==0)
    return;
  for (int32_t i = 0; i < 2; i++)
    g->createUnit(new TestUnit(rand01()*PI*2), new Figure::Circle(XY(rand01()*800, rand01()*800), rand01()*50));
}

void handle_update_world2(UpdateWorld* uw) {
  if (g->getPlayerCount()==0)
    return;
  int32_t k = (int32_t)(rand01()*g->getUnitCount());
  int32_t id;
  g->unit_list.foreach([&k, &id](uint32_t unit_id) {
    if (k==0) {
      k = -1;
      id = unit_id;
    }
    else
      k -= 1;
  });
  g->removeUnit(id);
}

void handle_message(Command* cmd) {
  auto& reader = *(cmd->reader);
  uint8_t case_id;
  reader >> case_id;
  auto player = (TestPlayer*)(g->player_list.get(cmd->player_id));
//  auto my_p = g->getXY(player->my_unit);
//  auto my_ball = (TestUnit*)(g->unit_list.get(player->my_unit));
  double x, y, r, dir, x2, y2;
  switch (case_id) {
    case 1:
    {
      reader >> x >> y >> r >> dir;
      auto unit = new TestUnit(dir);
      auto figure = new Figure::Circle(XY(x,y), r);
      g->createUnit(unit, figure);
      break;
    }
    case 2:
    {
      reader >> x >> y >> x2 >> y2 >> r;
      auto unit = new TestWall();
      auto figure = new Figure::Segment(XY(x,y), XY(x2,y2), r);
      g->createUnit(unit, figure);
      break;
    }
    case 3:
    {
      g->addCooldown(new Split(player->my_unit, 3000));
      break;
    }
    case 4:
    {
      reader >> x >> y;
      XY p(x, y);
      g->changeXY(player->my_unit, p);
    }
    case 5:
    {
      reader >> x >> y;
      XY p(x, y);
      double p_len = p.norm();
      double max_len = 100;
      double min_len = 15;
      double speed = 0.2;
      if (p_len < min_len)
        p_len = 0;
      else if (p_len < max_len)
        p_len = (p_len-min_len)/(max_len-min_len)*speed;
      else
        p_len = speed;
      g->changeV(player->my_unit, p.normalized()*p_len);
    }
  }
}

void handle_close(Close* close) {}

void handle_remove_unit(RemoveUnit* ru) {
  if (rand01()<0.5)
    handle_update_world2(nullptr);
}

void handle_collision(OnCollision* oc) {
  uint32_t killed = -1;
  auto unit1 = g->unit_list.get(oc->id1);
  auto unit2 = g->unit_list.get(oc->id2);
  if (unit1->mass==2)
    killed = oc->id2;
  if (unit2->mass==2)
    killed = oc->id1;
  if (killed!=-1 && g->unit_list.get(killed)->type!=Unit::WALL)
    g->removeUnit(killed);
}

void handle_create_unit(CreateUnit* cu) {
  // after create a unit, this event will be triggered...
  // what it does here is that with 1/2 probability, a new unit is generated
  uint32_t id = cu->unit_id;
  if (rand01()<0.75) {
    double dir = rand01()*PI*2;   // new unit is adjacent to old one with a random angle
    double rr = g->getR(id);      
    g->createUnit(new TestUnit(dir), 
      new Figure::Circle(g->getXY(id)+XY(sin(dir)*rr, cos(dir)*rr), rr/2));
      // the radius is 1/2 of the old one
  }
}

void handle_create_player(CreatePlayer* cp) {
  g->createUnit(new TestUnit(-PI/4), new Figure::Circle(XY(400,400), 100));
}

void handle_collision2(OnCollision* oc) {
  uint32_t id1 = oc->id1, id2 = oc->id2;
  if (g->getFigure(id1)->type==Figure::SEGMENT) {
    uint32_t id3 = id1;
    id1 = id2;
    id2 = id3;
  }
  if (g->getFigure(id1)->type!=Figure::CIRCLE || g->getFigure(id2)->type!=Figure::SEGMENT)
    return;
  g->changeUnit(id1, new Figure::Segment(g->getXY(id1), g->getXY(id2), 10));
}

void handle_collision3(OnCollision* oc) {
  uint32_t id1 = oc->id1, id2 = oc->id2;
  if (rand01()<0.5) {
    uint32_t id3 = id1;
    id1 = id2;
    id2 = id3;
  }
  if (g->unit_list.get(id1)->type==Unit::WALL)
    return;
  if (g->unit_list.get(id2)->type==Unit::WALL)
    return;
  if (((TestUnit*)(g->unit_list.get(id1)))->owner==-1) {
    uint32_t id3 = id1;
    id1 = id2;
    id2 = id3;
  }
  if (((TestUnit*)(g->unit_list.get(id1)))->owner==-1)
    return;
  g->removeUnit(id2);
}

void handle_update_world3(UpdateWorld* uw) {
  g->unit_list.foreach([](uint32_t id) {
    if (rand01()<0.04)
      g->changeR(id, g->getR(id)+5);
  });
}

void update_ownership(uint32_t player_id, uint32_t unit_id) {
  auto player = (TestPlayer*)(g->player_list.get(player_id));
  auto unit = (TestUnit*)(g->unit_list.get(unit_id));
  g->observe(unit_id, 1, player_id, 1);
  g->observe(player_id, 2, unit_id, 1);
  g->updateView();
  unit->owner = player_id;
  player->my_unit = unit_id;
  g->update(player_id, 2, [player_id, unit_id](Socket& socket) {
    socket << (uint8_t)101 << player_id << unit_id;
  });
  g->update(unit_id, 1, [player_id, unit_id](Socket& socket) {
    socket << (uint8_t)102 << unit_id << player_id;
  });
}

void handle_death(RemoveUnit* ru) {
  auto unit_raw = g->unit_list.get(ru->unit_id);
  if (unit_raw->type==Unit::TEST) {
    auto unit = (TestUnit*)unit_raw;
    if (unit->owner!=-1) {
      uint32_t new_unit_id = -1;
      g->unit_list.foreach([&new_unit_id](uint32_t unit_id) {
        if (new_unit_id!=-1)
          return;
        auto unit = g->unit_list.get(unit_id);
        if (unit->type==Unit::TEST && ((TestUnit*)unit)->owner==-1)
          new_unit_id = unit_id;
      });
      if (new_unit_id!=-1)
        update_ownership(unit->owner, new_unit_id);
    }
  }
}

int main() {
  g = new Game();
  g->getCollisionType = [](uint32_t id1, uint32_t id2){
    if (g->unit_list.get(id1)->type == Unit::WALL && g->unit_list.get(id2)->type == Unit::WALL) 
      return CollisionType::NONE;
    return CollisionType::REPULSIVE_DETECTABLE;
  };
  g->trigger(handle_init1);
  g->trigger(handle_init2);
  g->trigger(handle_start);
  g->trigger(handle_message);
  g->trigger(handle_close);
 // g->trigger(handle_update_world);
 // g->trigger(handle_update_world2);
 // g->trigger(handle_remove_unit);
 // g->trigger(handle_collision);
  g->trigger(handle_create_unit);
  g->trigger(handle_create_player);
  g->trigger(handle_death);
 // g->trigger(handle_collision2);
  g->trigger(handle_collision3);
 // g->trigger(handle_update_world3);
  g->mainLoop();
}

#include "lib/world2d.h"

struct Data
{
  int32_t playerNum;
  Data() : playerNum(0) {}
};

enum Cooldown::Type {
  RPSCOOLDOWN
};

struct RPSCooldown : Cooldown {
  int32_t next_gesture;
  RPSCooldown(Game* game) : Cooldown(RPSCOOLDOWN, game) {}
  void handle() {
    unit->gesture = next_gesture;
    game->_updateUnit(unit, new ChangeGesture(next_gesture));
  }
  void toMessage(Writer& writer) {
    writer.writeChar(next_gesture);
    writer.writeInt64(time);
  }
};

enum Unit::Type {
  RPS
};

struct RPSUnit : Unit {
  int32_t gesture;
  Player_ptr owner;
  RPSCooldown cd;
  RPSUnit(float x, float y, Player_ptr& player, Game* game) 
    : Unit(RPS, new Circle(Figure::CIRCLE, XY(x, y), 10), game), cd(game) {
    gesture = rand() % 3;
    owner = player;
  }
  bool ifRepulsingUnit(Unit_ptr& unit) {
    return true;
  }
  void toMessage(Writer& writer) {
    writer.writeChar(gesture);
    writer.writeInt(owner->id);
    writer.writeCooldown(cd);
  }
};

enum Player::Type {
  RPS
};

struct RPSPlayer : Player
{
  Unit_ptr unit;
  string nickname;
  RPSPlayer(Socket* socket, string nickname, Game* game) : Player(RPS, socket, game), nickname(nickname), unit(unit_null) {}
  void adjustXYZ() {
    cam = unit->figure->p;
  }
  /* previous version:
  bool ifSeeingPlayer(Player_ptr player) {
    return ifSeeingUnit(player->unit);
  }
  */
  void foreachVisiblePlayer(function<void(Player_ptr&)>& fn) {
    set<int> visited;
    foreachVisibleUnits([&visited](Unit_ptr& unit) {
      if (visited.find(unit->owner->id)==visited.end()) {
        visited.insert(unit->owner->id);
        fn(unit->owner);
      }
    });
  }
  void toMessage(Writer& writer) {
    writer.writeInt(unit->id);
  }
};

enum Update::Type {
  ADD_UNIT, WIN_DUEL, LOSE_DUEL, GET_BIGGER, CHANGE_VELOCITY, CHANGE_GESTURE, SWITCH_GESTURE, ADD_PLAYER_NUM
};

struct AddUnit : Update
{
  int32_t id;
  AddUnit(int32_t id) : Update(ADD_UNIT), id(id) {}
  void toMessage(Writer& writer) {
    writer.writeChar(ADD_UNIT);
    writer.writeInt(id);
  }
};

struct WinDuel : Update {
  WinDuel() : Update(WIN_DUEL) {}
};

struct LoseDuel : Update {
  LoseDuel() : Update(LOSE_DUEL) {}
};

struct GetBigger : Update
{
  GetBigger() : Update(GET_BIGGER) {}
  void toMessage(Writer& writer) {
    writer.writeChar(GET_BIGGER);
  }
};

struct ChangeGesture : Update
{
  int32_t next_gesture;
  ChangeGesture(int32_t next_gesture) : Update(CHANGE_GESTURE), next_gesture(next_gesture) {}
  void toMessage(Writer& writer) {
    writer.writeChar(CHANGE_GESTURE);
    writer.writeChar(next_gesture);
  }
};

struct ChangeVelocity : Update {
  XY v;
  ChangeVelocity(Figure* figure) : Update(CHANGE_VELOCITY), v(figure->v) {}
  void toMessage(Writer& writer) {
    writer.writeChar(CHANGE_VELOCITY);
    writer.writeXY(v);
  }  
};

struct SwitchGesture : Update {
  RPSCooldown& cd;
  SwitchGesture(RPSCooldown& cd) : Update(SWITCH_GESTURE), cd(cd) {}
  void toMessage(Writer& writer) {
    writer.writeChar(SWITCH_GESTURE);
    writer.writeCooldown(cd);
  }   
};

struct AddPlayerNum : Update {
  int32_t num;
  AddPlayerNum(int32_t num) : num(num) {}
  void toMessage(Writer& writer) {
    writer.writeChar(ADD_PLAYER_NUM);
    writer.writeInt(num);
  }
};

Game* game;

int main() {

  game = new Game();

  game->addTrigger(Event::START, [](Event* _evt, Game* game) {
    auto evt = dynamic_cast<Event::Start*>(_evt);
    game->_createPlayer(make_shared<RPSPlayer>(evt->socket, evt->nickname));
    game->data->playerNum += 1;
    game->_updateGame(new AddPlayerNum(game->data->playerNum));
  });

  game->addTrigger(Event::AFTER_REMOVE_PLAYER, [](Event* _evt, Game* game)) {
    auto evt = dynamic_cast<Event::After_removePlayer*>(_evt);
    game->data->playerNum -= 1;
    game->_updateGame(new AddPlayerNum(game->data->playerNum));
  }

  game->addTrigger(Event::AFTER_CREATE_PLAYER, [](Event* _evt, Game* game) {
    auto evt = dynamic_cast<Event::After_createPlayer*>(_evt);
    auto unit = make_shared<RPSUnit>(rand(), rand(), evt->player);
    game->_createUnit(unit);
    evt->player->unit = unit;
    game->_updatePlayer(player, new AddUnit(unit->id));
  });

  game->addTrigger(Event::ON_COLLISION, [](Event* _evt, Game* game) {
    auto evt = dynamic_cast<Event::OnCollision*>(_evt);
    auto unit1 = evt->unit1, unit2 = evt->unit2;
    if ((unit1->gesture-unit2->gesture)%3==1) {
      game->_updatePlayer(unit1->player, new WinDuel());
      game->_updatePlayer(unit2->player, new LoseDuel());
    }
    else if ((unit1->gesture-unit2->gesture)%3==2) {
      game->_updatePlayer(unit2->player, new WinDuel());
      game->_updatePlayer(unit1->player, new LoseDuel());
    }
  });

  game->addTrigger(Event::AFTER_CREATE_UNIT, [](Event* _evt, Game* game) {
    auto evt = dynamic_cast<Event:After_createUnit*>(_evt);
    evt->unit->cd.enable(evt->unit);
  });

  game->addTrigger(Event::AFTER_UPDATE_PLAYER, [](Event* _evt, Game* game) {
    auto evt = dynamic_cast<Event::After_updatePlayer*>(_evt);
    auto player = dynamic_cast<RPSPlayer*>evt->player;
    if (evt->update->type==Update::WIN_DUEL) {
      float rr = player->unit->figure->r;
      game->changeUnit(player->unit, player->unit->figure->r+10); 
    }
    else if (evt->update->type==Update::LOSE_DUEL) {
      game->removeUnit(player->unit);
      game->removePlayer(player);
    }
  });

  game->addTrigger(Event::COMMAND, [](Event* _evt, Game* game) {
    auto evt = dynamic_cast<Event::Commnad*>(_evt);
    Reader& reader = evt->reader;
    int32_t caseId = reader.readChar();
    if (caseId==0) {
      player->unit->figure->v = reader.readXY();
      game->_updateUnit(player->unit, new ChangeVelocity(player->unit->figure));
    }
    if (caseId==1) {
      player->unit->cd.next_gesture = rand() % 3;
      player->unit->cd.begin(game->data->playerNum*1000);
      game->_updateUnit(player->unit, new SwitchGesture(player->unit->cd));
    }
  });

  game->run();
}

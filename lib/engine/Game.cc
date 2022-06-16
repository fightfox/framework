#include "Game.h"

//============================ system driver ===================================

Game::Game() {
  start_time = 0;
  start_time = getTime();
  getCollisionType = [](uint32_t id1, uint32_t id2) {return CollisionType::NONE;};
  fn_start = [](Start* start) {assert(false && "Not start implement!");};
  fn_command = [](Command* cmd) {};
  fn_close = [](Close* close) {assert(false && "Hackers may destroy your game!");};
  fn_bot = [](uint32_t player_id, BotOperation bo) {
    assert(false && "No bot operation specified!");
  };
  fn_end = [this](End* end) {disconnect(end->socket);};
}

void Game::mainLoop() {
  event_queue.insert(new Init(this));
  while (true) {
    unique_ptr<InputPacket> packet = std::move(socket_manager.pop());
    while (packet != nullptr) {
      if (packet->type == InputPacket::MESSAGE) {
        auto evt = new Command(getTime(), this, packet->socket, std::move(packet->bin));
        event_queue.insert(evt);
      }
      else if (packet->type == InputPacket::START) {
        auto evt = new Start(getTime(), this, packet->socket, std::move(packet->bin));
        event_queue.insert(evt);
      }
      else if (packet->type == InputPacket::CLOSE) {
        auto evt = new Close(getTime(), this, packet->socket);
        event_queue.insert(evt);
      }
      packet = std::move(socket_manager.pop());
    }
    Time earliest_event_time = event_queue.earliestTime();
    Time earliest_collision_time = collision_queue.earliestTime();
    Time earliest_cooldown_time = cooldown_queue.earliestTime();
    Time delay;
    Time real_time = getTime();
    if (earliest_event_time<earliest_collision_time && earliest_event_time<earliest_cooldown_time) {
      delay = earliest_event_time-real_time; 
      if (delay<=0) {
        auto evt = event_queue.consume();
        now = evt->time;
        evt->handle();
      }
    }
    else if (earliest_collision_time<earliest_cooldown_time) {
      delay = earliest_collision_time-real_time; 
      if (delay<=0) {
        auto evt = std::move(collision_queue.consume());
        now = evt->time;
        if (evt->colliding) {
          if (getCollisionType(evt->u1, evt->u2)==CollisionType::DETECTABLE)
            addCollisionEvent(evt->u1, evt->u2);
          else
            advanceEvolveCollision(evt->u1, evt->u2);
        }  
        else {
          evolveTrace(evt->u1);
          evolveTrace(evt->u2);
        }
      }
    }
    else {
      delay = earliest_cooldown_time-real_time; 
      if (delay<=0) {
        auto cd = std::move(cooldown_queue.consume());
        if (cd->id==INVALID_ID || isAlive(cd->id)) {
          now = cd->time;
          cd->handle();
        }
      }
    }
    if (delay>0)
      idle(min(delay, IDLE_TIME));
  }
}

//============================ triggers family =================================

void Game::trigger(function<void(Init*)> fn) {fns_init.push_back(fn);}
void Game::trigger(function<void(UpdateWorld*)> fn) {fns_update_world.push_back(fn);}
void Game::trigger(function<void(OnCollision*)> fn) {fns_on_collision.push_back(fn);}
void Game::trigger(function<void(Start*)> fn) {fn_start = fn;}
void Game::trigger(function<void(End*)> fn) {fn_end = fn;}
void Game::trigger(function<void(Command*)> fn) {fn_command = fn;}
void Game::trigger(function<void(Close*)> fn) {fn_close = fn;}
void Game::trigger(function<void(RemoveUnit*)> fn) {fns_remove_unit.push_back(fn);}
void Game::trigger(function<void(CreateUnit*)> fn) {fns_create_unit.push_back(fn);}
void Game::trigger(function<void(CreatePlayer*)> fn) {fns_create_player.push_back(fn);}
void Game::trigger(function<void(RemovePlayer*)> fn) {fns_remove_player.push_back(fn);}

//=========================== messages family ==================================

inline Socket& operator<<(Socket& socket, const XY& p) {
  return socket << p.x << p.y;
}

inline Socket& operator<<(Socket& socket, Figure* figure) {
  socket << (uint8_t)(figure->type);
  if (figure->type==Figure::CIRCLE) {
    socket << figure->r;
  }
  else if (figure->type==Figure::SEGMENT) {
    auto seg = (Figure::Segment*)figure;
    socket << seg->r << seg->d_p;
  }
  return socket;
}

inline Buffer::Writer& operator<<(Buffer::Writer& bout, const XY& p) {
  return bout << p.x << p.y;
}

Socket& Game::output(uint32_t player_id, uint8_t type_id) {
  auto& socket = *(player_list.get(player_id)->socket);
  socket << getTimeDelta() << type_id;
  return socket;
}

// 1
void Game::sendShow(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 1) << unit_id << animation.get(unit_id)->figure;
}
// special
void Game::sendMove(uint32_t unit_id, uint32_t player_id) {
  auto player = player_list.get(player_id);
  player->move_list << (uint8_t)getTimeDelta() << unit_id << animation.get(unit_id)->figure->p;
}
// 3
void Game::sendCreateUnit(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 3) << unit_id;
}
// 4
void Game::sendRemovePlayer(uint32_t player_id, uint32_t receiver_id) {
  output(receiver_id, 4) << player_id;
}
// 5
void Game::sendCreatePlayer(uint32_t player_id) {
  output(player_id, 5) << player_id;
}
// 6
void Game::sendRemoveUnit(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 6) << unit_id;
}
// 7
void Game::sendChangeUnit(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 7) << unit_id << animation.get(unit_id)->figure; 
}
// 8
void Game::sendChangeR(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 8) << unit_id << animation.get(unit_id)->figure->r; 
}
// 9
void Game::sendChangeXY(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 9) << unit_id << animation.get(unit_id)->figure->p;
}
// 10
void Game::sendHide(uint32_t unit_id, uint32_t player_id) {
  output(player_id, 10) << unit_id;
}
// 99
void Game::sendAll(uint32_t player_id) {
  auto player = player_list.get(player_id);
  player->move_list << (uint8_t)255;
  *(player->socket) << (uint8_t)255 << player->move_list << Socket::flush;
  player->move_list.clear();
  player->move_list << (double)animation.timeUntil;
}

void Game::update(uint32_t id, uint8_t priv, uint8_t type_id, const function<void(Socket&)>& fn) {
  view.foreach(Sensor(id, priv), [type_id, &fn, this](uint32_t player_id) {
    auto& socket = output(player_id, type_id); 
    fn(socket);
  });
}

void Game::update(uint8_t type_id, const function<void(Socket&)>& fn) {
  player_list.foreach([type_id, &fn, this](uint32_t player_id) {
    auto& socket = output(player_id, type_id);
    fn(socket);
  });
}


void Game::update(uint32_t id, uint8_t priv) {
  view.foreach(Sensor(id, priv), [id, priv, this](uint32_t player_id) {
    auto& socket = *(player_list.get(player_id)->socket);
    socket << getTimeDelta() << (uint8_t)12 <<  id << priv;
    if (id % 2==1) {
      auto player = player_list.get(id);
      socket << (uint8_t)player->type;
      player->toMessage(priv, socket);
    }
    else {
      auto unit = unit_list.get(id);
      socket << (uint8_t)unit->type;
      unit->toMessage(priv, socket);
    }
  });
}

void Game::updateView(bool show_move) {
  vector<SensorDelta> delta;
  vector<pair<uint32_t, uint32_t>> show_list;
  vector<pair<uint32_t, uint32_t>> hide_list;
  //          player_id unit_id
  view.update(delta, hide_list);
  for (auto& sd : delta) {
    auto player = player_list.get(sd.player_id);
    auto& socket = *(player->socket);
    socket << getTimeDelta() << (uint8_t)11 << sd.sensed_id << sd.prev_priv << sd.new_priv;  
    if (isPlayer(sd.sensed_id)) {
      auto sensed_player = player_list.get(sd.sensed_id);
      socket << (uint8_t)(sensed_player->type);
      for (uint8_t priv = sd.prev_priv+1; priv <= sd.new_priv; priv++)
        sensed_player->toMessage(priv, socket);   
    }
    else {
      auto sensed_unit = unit_list.get(sd.sensed_id);
      socket << (uint8_t)(sensed_unit->type);
      for (uint8_t priv = sd.prev_priv+1; priv <= sd.new_priv; priv++)
        sensed_unit->toMessage(priv, socket);
      if (sd.prev_priv==0 && sd.new_priv>0)
        show_list.push_back(make_pair(sd.player_id, sd.sensed_id));   
    }
  }
  for (auto& pr : show_list) {
    auto player_id = pr.first;
    auto unit_id = pr.second;
    if (show_move || unit_list.move_units.count(unit_id)==0)
      sendMove(unit_id, player_id);
    sendShow(unit_id, player_id);
  }
  for (auto& pr : hide_list) {
    auto player_id = pr.first;
    auto unit_id = pr.second;
    if (isAlive(unit_id))
      sendMove(unit_id, player_id);
    sendHide(unit_id, player_id);
  }
}

//============================ actions family ==================================

void Game::moveUnit(uint32_t unit_id, Time time) {
  auto figure = animation.get(unit_id)->evolve(time);
  auto unit_bb = figure->getBB();
  canvas[unit_id] = unit_bb;
  if (!unit_list.get(unit_id)->visible)
    return;
  // move unit from canvas.get(id) to figure->getBB()
  camera.foreach(unit_id, [unit_id, figure, &unit_bb, this](uint32_t player_id) {
    auto player_bb = vision.get(player_id);
    if (!unit_bb.intersects(player_bb)) {
      camera.remove(player_id, unit_id);
      view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit_id, 0));
    }
  });
  vision.foreach(unit_bb, [unit_id, &unit_bb, this](uint32_t player_id) {
    auto player_bb = vision.get(player_id);
    auto player_small_bb = player_bb-VISION_PADDING;
    if (unit_bb.intersects(player_small_bb) && !camera.checkConnection(unit_id, player_id)) {
      camera.insert(player_id, unit_id);
      view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit_id, 1));
    }
  });
}

void Game::updateCamera(uint32_t player_id) {
  // get bb from new vision 
  auto player_bb = vision.get(player_id);
  camera.foreach(player_id, [player_id, &player_bb, this](uint32_t unit_id) {
    auto unit_bb = canvas.get(unit_id);
    if (!unit_bb.intersects(player_bb)) {
      camera.remove(player_id, unit_id);
      view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit_id, 0));
    }
  });
  auto player_small_bb = player_bb-VISION_PADDING; 
  canvas.foreach(player_small_bb, [player_id, this](uint32_t unit_id) {
    if (isVisible(unit_id) && !camera.checkConnection(unit_id, player_id)) {
      camera.insert(player_id, unit_id);
      view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit_id, 1));
    }
  });
}

uint32_t Game::createUnit(Unit* unit, Figure* figure) {
  unit_list.insert(unit);
  auto trace = new Trace(figure, unit->v, now);
  animation[unit->id] = trace;
  evolveTrace(unit->id);
  // add into canvas and update camera based on vision
  auto unit_bb = figure->getBB();
  canvas[unit->id] = unit_bb;
  view.insert(unit->id, unit->own_priv);
  if (unit->visible) {
    vision.foreach(unit_bb, [unit, &unit_bb, this](uint32_t player_id) {
      auto player_small_bb = vision.get(player_id)-VISION_PADDING;
      if (player_small_bb.intersects(unit_bb)) {
        camera.insert(player_id, unit->id);
        view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit->id, 1));
      }
    });
    updateView(true);
    // send out create action based on view
    view.foreach(Sensor(unit->id, 1), [unit, this](uint32_t player_id) {
      sendCreateUnit(unit->id, player_id);
    });  
  }
  // new CreateUnit event
  event_queue.insert(new CreateUnit(now, this, unit->id));
  return unit->id;
}

uint32_t Game::createPlayer(Player* player) {
  player_list.insert(player); 
  player->move_list << (double)tick_head;
  // add into vision, update camera based on canvas, resulting in the update of view
  // update view, send out creation action based on view
  auto player_bb = player->getBB(); 
  vision[player->id] = player_bb;
  view.insert(player->id, player->own_priv);
  canvas.foreach(player_bb-VISION_PADDING, [player, this](uint32_t unit_id) {
    if (isVisible(unit_id)) {
      camera.insert(player->id, unit_id);
      view.observe(Sensor(player->id, player->own_priv+1), Sensor(unit_id, 1));
    }
  });
  updateView(true);
  // send out notification
  sendCreatePlayer(player->id);
  // trigger create player event
  event_queue.insert(new CreatePlayer(now, this, player->id));
  if (player->socket->type==Socket::BOT_SOCKET) {
    bot_list.insert(player->id);
    fn_bot(player->id, BotOperation::START);
  }
  return player->id;
}

bool Game::removeUnit(uint32_t unit_id) {
  auto unit = unit_list.get(unit_id);
  if (unit==nullptr || !unit->alive)
    return false;
  unit->alive = false;
  unit_list.dying_count += 1;
  collision_queue.remove(unit_id);
  animation.get(unit_id)->evolve(now);
  // send-out this action based on view
  view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
    sendMove(unit_id, player_id);
    sendRemoveUnit(unit_id, player_id);
  });
  Trace* dead_trace = animation.transfer(unit_id);
  // remove edges (hide unit) on camera (no need to updateView aftermath)
  // remove it from canvas, remove it from view
  // trigger its events
  for (auto& fn : fns_remove_unit) {
    auto evt = make_unique<RemoveUnit>(now, this, unit_id, dead_trace);
    fn(evt.get());
  }
  // update information
  if (unit->visible)
    camera.remove(unit_id);
  canvas[unit_id] = nullptr;
  view.remove(unit_id);
  updateView(true);
  delete dead_trace;
  // kill it
  unit_list.remove(unit_id);
  return true;
}

bool Game::removePlayer(uint32_t player_id) {
  auto player = player_list.get(player_id);
  if (player==nullptr || !player->alive)
    return false;
  player->alive = false;
  player_list.dying_count += 1;
  // send-out this action based on view
  view.foreach(Sensor(player_id, 1), [player_id, this](uint32_t receiver_id) {
    sendRemovePlayer(player_id, receiver_id);
  });
  // trigger events
  for (auto& fn : fns_remove_player) {
    auto evt = make_unique<RemovePlayer>(now, this, player_id);
    fn(evt.get());
  }
  // remove edges (hide unit) on camera (no need to updateView aftermath)
  // remove it from vision, remove it from view
  camera.remove(player_id);
  vision[player_id] = nullptr;
  view.remove(player_id);
  updateView(true);
  // kill it
  auto socket = player->socket; 
  if (socket->type==Socket::BOT_SOCKET) {
    fn_bot(player->id, BotOperation::DEATH);
    bot_list.erase(player->id);
  }
  sendAll(player_id);
  player_list.remove(player_id);
  if (socket->type==Socket::WEB_SOCKET)
    event_queue.insert(new End(now, this, player, socket));
  else
    delete player;
  return true;
}

void Game::changeUnit(uint32_t unit_id, Figure* figure) {
  auto unit = unit_list.get(unit_id);
  if (unit==nullptr || !unit->alive)
    return;
  auto trace = animation.get(unit_id);
  trace->evolve(now);
  view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
    sendMove(unit_id, player_id);
  });
  delete trace->figure;
  trace->figure = figure;
  animation[unit_id] = trace;
  // send out via view
  view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
    sendMove(unit_id, player_id);
    sendChangeUnit(unit_id, player_id);
  });
  evolveTrace(unit_id);
}

void Game::changeR(uint32_t unit_id, double r) {
  auto unit = unit_list.get(unit_id);
  if (unit==nullptr || !unit->alive)
    return;
  auto trace = animation.get(unit_id); 
  trace->figure->r = r;
  animation[unit_id] = trace;
  // send out via view
  view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
    sendChangeR(unit_id, player_id);
  });
  evolveTrace(unit_id);
}

void Game::changeXY(uint32_t unit_id, XY p) {
  auto unit = unit_list.get(unit_id);
  if (unit==nullptr || !unit->alive)
    return;
  auto trace = animation.get(unit_id);
  trace->evolve(now);
  view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
    sendMove(unit_id, player_id);
  });
  trace->figure->p = p;
  animation[unit_id] = trace;
  // broadcast via camera
  view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
    sendMove(unit_id, player_id);
  });
  evolveTrace(unit_id);
}

void Game::changeV(uint32_t unit_id, XY v) {
  auto unit = unit_list.get(unit_id);
  if (unit==nullptr || !unit->alive)
    return;
  unit->v = v;
  if (!v.isZero())
    unit_list.enmove(unit_id);
}

void Game::createBot(unique_ptr<Buffer::Reader> reader) {
  auto socket = make_shared<BotSocket>();
  event_queue.insert(new Start(now, this, socket, std::move(reader)));
}

void Game::assignBot(uint32_t player_id) {
  auto player = player_list.get(player_id);
  disconnect(player->socket);
  player->socket = make_shared<BotSocket>();
  bot_list.insert(player_id);
  fn_bot(player->id, BotOperation::ASSIGN);
}

void Game::visiblize(uint32_t unit_id) {
  auto unit = unit_list.get(unit_id);
  if (unit->visible)
    return; 
  // add into canvas and update camera based on vision
  unit->visible = true;
  auto unit_bb = canvas.get(unit_id);
  vision.foreach(unit_bb, [unit, &unit_bb, this](uint32_t player_id) {
    auto player_small_bb = vision.get(player_id)-VISION_PADDING;
    if (player_small_bb.intersects(unit_bb)) {
      camera.insert(player_id, unit->id);
      view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit->id, 1));
    }
  });
  updateView(true);    
}

void Game::invisiblize(uint32_t unit_id) {
  auto unit = unit_list.get(unit_id);
  if (!unit->visible)
    return;
  unit->visible = false;
  camera.foreach(unit_id, [unit_id, this](uint32_t player_id) {
    view.observe(Sensor(player_id, getCameraPriv(player_id)), Sensor(unit_id, 0));
  }); 
  camera.remove(unit_id);
  updateView(true);
}

//============================ collision family ==================================

void Game::updateCollisionPair(uint32_t id1, uint32_t id2) {
  Trace* tr1 = animation.get(id1);
  Trace* tr2 = animation.get(id2);
  pair<Time, Time> times = tr1->computeCollision(tr2);
  if (times.first!=INVALID_TIME && times.first>now && times.first<animation.timeUntil)
    collision_queue.insert(max(times.first-EPSILON_TIME, now), id1, id2, true);
  if (times.second!=INVALID_TIME && times.second>now && times.second<animation.timeUntil)
    collision_queue.insert(times.second+EPSILON_TIME, id1, id2, false);
}

void Game::updateCollision(uint32_t id) {
  animation.foreach(animation.get(id)->getBB(animation.timeUntil), [id, this](uint32_t id2){
    if (id==id2 || getCollisionType(id, id2)==CollisionType::NONE)
      return;
    updateCollisionPair(id, id2);
  });
}

void Game::addCollisionEvent(uint32_t id1, uint32_t id2) {
  auto collision_type = getCollisionType(id1, id2);
  if (collision_type!=CollisionType::DETECTABLE && collision_type!=CollisionType::REPULSIVE_DETECTABLE)
    return;
  if (collision_marker.count(BIG_INT64*id1+id2)+collision_marker.count(BIG_INT64*id2+id1)==0) {
    event_queue.insert(new OnCollision(now, this, id1, id2));
    collision_marker.insert(BIG_INT64*id1+id2);
  }
}

void Game::initTrace(uint32_t id) {
  auto unit = unit_list.get(id);
  if (unit==nullptr)
    return;
  double my_mass = unit->mass;
  Trace* tr = animation.get(id);
  Figure* fg = tr->figure;
  XY my_v(0, 0);
  canvas.foreach(fg->getBB()+EPSILON_LENGTH, [id, my_mass, &my_v, fg, this](uint32_t id2){
    if (id==id2 || getCollisionType(id, id2)==CollisionType::NONE)
      return;
    Figure* fg2 = animation.get(id2)->figure;
    if (fg->checkCollision(fg2)) {
      addCollisionEvent(id, id2);
      if (collisionRepulsive(id, id2)) {
        double ur_mass = unit_list.get(id2)->mass;
        XY dv = fg->nVectorTo(fg2)*INNER_VELOCITY;
        if (my_mass!=INFINITY_MASS)
          my_v -= dv*(ur_mass/(my_mass+ur_mass));
      }
    }
  });
  if (!my_v.isZero()) {
    tr->v = my_v;
  }
  else
    tr->v = unit->v;
  animation[id] = tr;
  if (tr->v.isZero())
    unit_list.dismove(id);
}

void Game::evolveTrace(uint32_t id) {
  auto unit = unit_list.get(id);
  if (unit==nullptr)
    return;
  double my_mass = unit->mass;
  Trace* tr = animation.get(id);
  Figure* fg = tr->evolve(now);
  XY my_v(0, 0);
  collision_queue.remove(id);
  vector<uint32_t> changed_units;
  animation.foreach(fg->getBB(), [id, my_mass, &my_v, fg, &changed_units, this](uint32_t id2){
    if (id==id2 || getCollisionType(id, id2)==CollisionType::NONE)
      return;
    Trace* tr2 = animation.get(id2);
    Figure* fg2 = tr2->evolve(now);
    if (fg->checkCollision(fg2)) {
      addCollisionEvent(id, id2);
      if (collisionRepulsive(id, id2)) {
        double ur_mass = unit_list.get(id2)->mass;
        XY dv = fg->nVectorTo(fg2)*INNER_VELOCITY;
        if (my_mass!=INFINITY_MASS)
          my_v -= dv*(ur_mass/(my_mass+ur_mass));
        if (ur_mass!=INFINITY_MASS) {
          tr2->v += dv*(my_mass/(my_mass+ur_mass));
          animation[id2] = tr2;
          collision_queue.remove(id2);
          changed_units.push_back(id2);   
          if (!tr2->v.isZero())
            unit_list.enmove(id2);
        }
      }
    }
  });
  if (!my_v.isZero()) {
    tr->v = my_v;
  }
  else
    tr->v = unit->v;
  animation[id] = tr;
  if (!tr->v.isZero())
    unit_list.enmove(id);
  else
    unit_list.dismove(id);
  updateCollision(id);
  for (auto id2 : changed_units)
    updateCollision(id2);
}



inline pair<XY, XY> computeTwoCollision(Figure* fg1, Figure* fg2, XY& v1, XY& v2, double mass1, double mass2) {
  XY n_vector = fg1->nVectorTo(fg2);
  XY t_vector = n_vector.rotate(PI/2);
  double t1 = v1.dot(t_vector);
  double t2 = v2.dot(t_vector);
  double n1 = v1.dot(n_vector);
  double n2 = v2.dot(n_vector);
  double n3 = (n1*mass1+n2*mass2)/(mass1+mass2);
  if (mass1==INFINITY_MASS)
    n3 = n1;
  if (mass2==INFINITY_MASS)
    n3 = n2;
  return make_pair(n_vector*n3+t_vector*t1, n_vector*n3+t_vector*t2);
}


void Game::advanceEvolveCollision(uint32_t id0, uint32_t id1) {
  Unit* units[2];
  units[0] = unit_list.get(id0);
  if (units[0]->mass==INFINITY_MASS) {
    std::swap(id0, id1);
    units[1] = units[0];
    units[0] = unit_list.get(id0);
  }
  else
    units[1] = unit_list.get(id1);
  if (units[0]->mass==INFINITY_MASS)
    return;

  dense_hash_map<uint32_t, CollisionNode*> collision_nodes;
  collision_nodes.set_empty_key(-1);
  collision_nodes.set_deleted_key(-2);
  vector<uint32_t> bfs_nodes;
  vector<CollisionEdge*> bfs_edges;
  unordered_set<int64_t> edge_marker;
  vector<uint32_t> variable_seq;
  uint32_t ids[2] = {id0, id1};
  
  Trace* trs[2] = {animation.get(id0), animation.get(id1)};
  Figure* fgs[2] = {trs[0]->evolve(now), trs[1]->evolve(now)};
  BB both;
  vector<uint32_t> first_tough;
  if (units[1]->mass==INFINITY_MASS)
    both = fgs[0]->getBB()+EPSILON_LENGTH;
  else 
    both = (fgs[0]->getBB() | fgs[1]->getBB())+EPSILON_LENGTH;
  animation.qt.getAllIdIntersectBB(both, first_tough);
  bool quick = true;
  if (first_tough.size()>2) { 
    for (uint32_t i = 0; i < 2; i++) {
      collision_nodes[ids[i]] = new CollisionNode(ids[i], trs[i]->v, units[i]->mass, fgs[i]);
      bfs_nodes.push_back(ids[i]);
    }
    bfs_edges.push_back(new CollisionEdge(0, id0, id1, fgs[0]->nVectorTo(fgs[1])));
    edge_marker.insert(BIG_INT64*id0+id1);
    for (auto next_id : first_tough) {
      if (next_id==id0 || next_id==id1)
        continue;
      Trace* next_trace = animation.get(next_id);
      Figure* next_figure = next_trace->evolve(now);  
      for (uint32_t i = 0; i < 2; i++) {
        if (!collisionRepulsive(ids[i], next_id) || units[i]->mass==INFINITY_MASS)
          continue;
        if (fgs[i]->mayCollide(next_figure)) {
          quick = false;        
          if (collision_nodes.count(next_id)==0) {
            collision_nodes[next_id] = new CollisionNode(next_id, next_trace->v, 
              unit_list.get(next_id)->mass, next_figure);
            bfs_nodes.push_back(next_id);
          }
          uint32_t edge_id = bfs_edges.size();
          bfs_edges.push_back(new CollisionEdge(edge_id, ids[i], next_id,
            fgs[i]->nVectorTo(next_figure)));
          edge_marker.insert(BIG_INT64*ids[i]+next_id);
        } 
      }
    }
  }
  if (quick) {
    pair<XY, XY> tmp = computeTwoCollision(fgs[0], fgs[1], trs[0]->v, trs[1]->v, 
      units[0]->mass, units[1]->mass);
    trs[0]->v = tmp.first;
    trs[1]->v = tmp.second;
    if (!tmp.first.isZero())
      unit_list.enmove(id0);
    if (!tmp.second.isZero())
      unit_list.enmove(id1);
    animation[id0] = trs[0];
    animation[id1] = trs[1];
    collision_queue.remove(id0);
    if (units[1]->mass != INFINITY_MASS)
      collision_queue.remove(id1);

    vector<uint32_t> second_tough;
    if (units[1]->mass==INFINITY_MASS)
      both = trs[0]->getBB(animation.timeUntil);
    else 
      both = both+(std::max(tmp.first.norm(), tmp.second.norm())*(animation.timeUntil-now)+EPSILON_LENGTH);
    animation.qt.getAllIdIntersectBB(both, second_tough);

    for (auto id2 : second_tough) {
      if (id2==id0 || id2==id1)
        continue;
      for (uint32_t i = 0; i < 2; i++) {
        uint32_t id = ids[i];
        if (units[i]->mass==INFINITY_MASS || getCollisionType(id, id2)==CollisionType::NONE)
          continue;
        updateCollisionPair(id, id2);
      }
    }

    addCollisionEvent(id0, id1);
    for (auto& pr : collision_nodes)
      delete pr.second;
    for (auto edge : bfs_edges)
      delete edge;
    return;
  }
  
  uint32_t top = 2;
  while (top<bfs_nodes.size()) {
    auto& current_node = *(collision_nodes[bfs_nodes[top]]);
    top += 1;
    if (current_node.mass==INFINITY_MASS)
      continue;
    animation.foreach(current_node.figure->getBB()+EPSILON_LENGTH, 
      [&collision_nodes, &bfs_edges, &bfs_nodes, &edge_marker, &current_node, this]
      (uint32_t next_id){
      if (next_id==current_node.id)
        return;
      if (!collisionRepulsive(current_node.id, next_id))
        return;
      Trace* next_trace = animation.get(next_id);
      Figure* next_figure = next_trace->evolve(now);
      if (edge_marker.count(BIG_INT64*next_id+current_node.id)==0 
        && current_node.figure->mayCollide(next_figure)) {        
        if (collision_nodes.count(next_id)==0) {
          collision_nodes[next_id] = new CollisionNode(next_id, next_trace->v, 
            unit_list.get(next_id)->mass, next_figure);
          bfs_nodes.push_back(next_id);
        }
        uint32_t edge_id = bfs_edges.size();
        bfs_edges.push_back(new CollisionEdge(edge_id, current_node.id, next_id,
          current_node.figure->nVectorTo(next_figure)));
        edge_marker.insert(BIG_INT64*current_node.id+next_id);
      }
    });
  }

  while (true) {
    bool flag = false;
    for (auto edge : bfs_edges) {
      if (edge->assoicated)
        continue;
      auto& node_from = *(collision_nodes[edge->nodes[0]]);
      auto& node_to = *(collision_nodes[edge->nodes[1]]);
      if (node_from.v.dot(edge->n_v)>node_to.v.dot(edge->n_v)) {
        edge->variable_id = variable_seq.size();
        variable_seq.push_back(edge->id);
        edge->assoicated = true;
        node_from.edges.push_back(make_pair(edge->id, 1));
        node_to.edges.push_back(make_pair(edge->id, -1));
        flag = true;
      }
    }
    if (!flag)
      break;
    LinearSolver ls(variable_seq.size());
    for (uint32_t i = 0; i < variable_seq.size(); i++) {
      auto edge = bfs_edges[variable_seq[i]];
      for (int32_t k = 0; k < 2; k++) {
        double sign = 1-k*2;
        auto& c = *(collision_nodes[edge->nodes[k]]);
        ls.addConstant(i, -c.v.dot(edge->n_v)*sign);
        for (auto& pr : c.edges) {
          auto edge2 = bfs_edges[pr.first];
          ls.addCoefficient(i, edge2->variable_id, 
            edge2->n_v.dot(edge->n_v)*sudoinverse(c.mass)*sign*pr.second);
        }
      }
    }
    vector<double> ans = ls.solve();
    for (uint32_t i = 0; i < variable_seq.size(); i++) {
      auto edge = bfs_edges[variable_seq[i]];
      for (int32_t k = 0; k < 2; k++) {
        auto& c = *(collision_nodes[edge->nodes[k]]);
        c.v -= edge->n_v*(ans[i]*(k*2-1)*sudoinverse(c.mass));
      }
    }
  }

  int32_t tail = 0;
  for (int32_t i = 0; i < bfs_nodes.size(); i++) {
    uint32_t id = bfs_nodes[i];
    auto& c = *(collision_nodes[id]);
    Trace* tr = animation.get(id);
    if (!(tr->v-c.v).isZero()) {
      tr->v = c.v;
      animation[id] = tr;
      collision_queue.remove(id);
      bfs_nodes[tail] = id;
      tail += 1;
    }
    if (!tr->v.isZero())
      unit_list.enmove(id);
  }
  for (int32_t i = 0; i < tail; i++) {
    uint32_t id = bfs_nodes[i];
    animation.foreach(animation.get(id)->getBB(animation.timeUntil), 
      [id, &edge_marker, this](uint32_t id2){
      if (id==id2 || getCollisionType(id, id2)==CollisionType::NONE)
        return;
      if (edge_marker.count(BIG_INT64*id+id2)+edge_marker.count(BIG_INT64*id2+id)>0)
        return;
      updateCollisionPair(id, id2);
    });
  }
  for (auto i : variable_seq) {
    auto edge = bfs_edges[i];
    addCollisionEvent(edge->nodes[0], edge->nodes[1]);
  }
  for (auto& pr : collision_nodes)
    delete pr.second;
  for (auto edge : bfs_edges)
    delete edge;
}


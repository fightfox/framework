#include "UpdateWorld.h"
#include "../engine/Game.h"

UpdateWorld::UpdateWorld(Time time, Game* game) : Event(UPDATE_WORLD, time, game) {}
void UpdateWorld::handle() {
  // update view and send out message
  g->player_list.foreach([this](uint32_t player_id) {
    g->vision[player_id] = g->player_list.get(player_id)->getBB();
  });  
  g->unit_list.foreachmove([this](uint32_t id){
    g->moveUnit(id, g->now);
  });
  g->player_list.foreach([this](uint32_t player_id) {
    g->updateCamera(player_id);
  });  
  g->updateView(false);
  g->unit_list.foreachmove([this](uint32_t unit_id){
    g->view.foreach(Sensor(unit_id, 1), [unit_id, this](uint32_t player_id) {
      g->sendMove(unit_id, player_id);
    });
  });
  g->player_list.foreach([this](uint32_t player_id) {
    g->sendAll(player_id);
  });
  // open a new world
  g->tick_head = g->animation.timeUntil;
  Time next_time = max(g->getTime()+MINIMUM_TIME_PER_TICK, g->now+TIME_PER_TICK);
  g->animation.setUntil(next_time);
  g->event_queue.insert(new UpdateWorld(next_time, g));
  g->collision_marker.clear();
  g->unit_list.foreachmove([this](uint32_t id) {
    g->initTrace(id);
  });
  g->unit_list.foreachmove([this](uint32_t id){
    g->animation.foreach(g->animation.get(id)->getBB(g->animation.timeUntil), [id, this](uint32_t id2){
      if (id==id2 || g->getCollisionType(id, id2)==CollisionType::NONE)
        return;
      g->updateCollisionPair(id, id2);
    });
  }); 
  // trigger 
  for (auto& fn : g->fns_update_world)
    fn(this);
  for (auto player_id : g->bot_list)
    g->fn_bot(player_id, BotOperation::PLAY);
}
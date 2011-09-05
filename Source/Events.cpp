#include "StdHeader.h"
#include "EventSystem\Events.h"


char const * const TempEvent::gkName = "temp";
char const * const Evt_New_Game::gkName = "new_game";
char const * const Evt_Melee_Attack::gkName = "melee_attack";
char const * const Evt_Game_State::gkName = "game_state";
char const * const Evt_New_Actor::gkName = "new_actor";
char const * const Evt_Destroy_Actor::gkName = "destroy_actor";
char const * const Evt_Move_Actor::gkName = "move_actor";
char const * const Evt_Queue_Actor_Move::gkName = "queue_actor_move";
char const * const Evt_Change_Actor_Direction::gkName = "change_actor_direction";
char const * const Evt_Set_Actor_Move::gkName = "set_actor_move";
char const * const Evt_Change_Actor_AnimationLoop::gkName = "change_actor_animiationloop";
char const * const Evt_Move_Camera::gkName = "move_camera";
char const * const Evt_Find_Closest_Actor::gkName = "closest_actor";
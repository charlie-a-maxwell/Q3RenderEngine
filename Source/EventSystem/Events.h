#pragma once

#include "StdHeader.h"
#include "EventSystem\event.h"
#include "GameLogic\Actor.h"

class TempEvent: public Event
{
public:
	static char const * const gkName;

	explicit TempEvent ():Event(gkName, 0, IEventDataPtr())
	{}

};

struct EvtData_New_Actor: public IEventData
{
	explicit EvtData_New_Actor (ActorId id, ActorParams *p)
	{
		m_id = id; 
		m_pActorParams = reinterpret_cast<ActorParams *>(SAFE_NEW char[p->GetSize()]);
		memcpy(m_pActorParams, p, p->GetSize());
	}

	~EvtData_New_Actor ()
	{
		SAFE_DELETE(m_pActorParams);
	}
	ActorId m_id;
	ActorParams *m_pActorParams;
};

class Evt_New_Actor: public Event
{
public:
	static char const * const gkName;

	explicit Evt_New_Actor(ActorId id, ActorParams *p): 
				Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_New_Actor(id, p)))
	{}
};

class Evt_New_Game: public Event
{
public:
	static char const * const gkName;

	explicit Evt_New_Game (): Event(gkName, 0, IEventDataPtr()){}
};

struct EvtData_Melee_Attack: public IEventData
{
	explicit EvtData_Melee_Attack (ActorId attacker, ActorId receiver, int a)
	{
		m_attackerId = attacker;
		m_receiverId = receiver;
		m_amount = a;
	}

	ActorId m_attackerId;
	ActorId m_receiverId;
	int		m_amount;
};

class Evt_Melee_Attack: public Event
{
public:
	static char const * const gkName;

	explicit Evt_Melee_Attack (ActorId attacker, ActorId receiver, int a): 
				Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Melee_Attack(attacker, receiver, a))){}
};


class Evt_Game_State: public Event
{
public:
	static char const * const gkName;

	explicit Evt_Game_State (BaseGameState value): 
			Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData<BaseGameState>( value ))){}
};

struct EvtData_Destroy_Actor: public IEventData
{
	explicit EvtData_Destroy_Actor (ActorId id)
	{
		m_id = id; 
	}

	ActorId m_id;
};

class Evt_Destroy_Actor: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Destroy_Actor (ActorId id): 
			Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Destroy_Actor(id))) {}
};


struct EvtData_Move_Actor: public IEventData
{
	explicit EvtData_Move_Actor (ActorId id, Mat4x4 mat)
	{
		m_Id = id;
		m_Mat = mat;
	}

	ActorId m_Id;
	Mat4x4 m_Mat;
};

class Evt_Move_Actor: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Move_Actor (ActorId id, Mat4x4 mat) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Move_Actor(id, mat))) {}
};


struct EvtData_Change_Actor_Direction: public IEventData
{
	explicit EvtData_Change_Actor_Direction (ActorId id, int mat)
	{
		m_Id = id;
		m_Dir = mat;
	}

	ActorId m_Id;
	int m_Dir;
};

class Evt_Change_Actor_Direction: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Change_Actor_Direction (ActorId id, int mat) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Change_Actor_Direction(id, mat))) {}
};

struct EvtData_Change_Actor_AnimationLoop: public IEventData
{
	explicit EvtData_Change_Actor_AnimationLoop (ActorId id, bool loop)
	{
		m_Id = id;
		m_Loop = loop;
	}

	ActorId m_Id;
	bool m_Loop;
};

class Evt_Change_Actor_AnimationLoop: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Change_Actor_AnimationLoop (ActorId id, bool loop) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Change_Actor_AnimationLoop(id, loop))) {}
};



struct EvtData_Queue_Actor_Move: public IEventData
{
	explicit EvtData_Queue_Actor_Move (ActorId id, Mat4x4 mat)
	{
		m_Id = id;
		m_Mat = mat;
	}

	ActorId m_Id;
	Mat4x4 m_Mat;
};

class Evt_Queue_Actor_Move: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Queue_Actor_Move (ActorId id, Mat4x4 mat) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Queue_Actor_Move(id, mat))) {}
};

struct EvtData_Set_Actor_Move: public IEventData
{
	explicit EvtData_Set_Actor_Move (ActorId id, Mat4x4 mat)
	{
		m_Id = id;
		m_Mat = mat;
	}

	ActorId m_Id;
	Mat4x4 m_Mat;
};

class Evt_Set_Actor_Move: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Set_Actor_Move (ActorId id, Mat4x4 mat) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Set_Actor_Move(id, mat))) {}
};

struct EvtData_Move_Camera: public IEventData
{
	explicit EvtData_Move_Camera (ActorId id, Vec3 vec)
	{
		m_Id = id;
		m_Dir = vec;
	}

	ActorId m_Id;
	Vec3 m_Dir;
};

class Evt_Move_Camera: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Move_Camera (ActorId id, Vec3 vec) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Move_Camera(id, vec))) {}
};

struct EvtData_Find_Closest_Actor: public IEventData
{
	explicit EvtData_Find_Closest_Actor (ActorId id)
	{
		m_Id = id;
	}

	ActorId m_Id;
};

class Evt_Find_Closest_Actor: public Event
{
public:
	static char const * const gkName;
	explicit Evt_Find_Closest_Actor (ActorId id) :
		Event(gkName, 0, IEventDataPtr(SAFE_NEW EvtData_Find_Closest_Actor(id))) {}
};

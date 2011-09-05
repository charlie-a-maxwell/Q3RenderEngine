#pragma once

#include "StdHeader.h"

struct PlaneParams : public ActorParams
{
	bool m_IsPaused;
	Color m_Color;
	std::string m_Texture;
	bool m_hasTextureAlpha;
	float m_radius;

	PlaneParams():ActorParams(), m_IsPaused(false), m_Color(g_White), m_hasTextureAlpha(false), m_radius(0.0f) 
		{ m_Type = AT_GROUND; m_Texture=""; m_Size=sizeof(PlaneParams);}

	virtual bool VInit(std::istrstream &in){ return true; }
	virtual bool VSerialize(std::ostrstream &out){ return true; }
};

struct MissileParams : public ActorParams
{
	bool m_LoopingAnim;
	float m_speed;
	MissileParams():ActorParams(), m_LoopingAnim(false), m_speed(1.0f) {m_Size=sizeof(MissileParams);}

	virtual bool VInit(std::istrstream &in){ return true; }
	virtual bool VSerialize(std::ostrstream &out){ return true; }
};

struct CharacterParams : public ActorParams
{
	GameViewId m_viewId;

	CharacterParams():ActorParams()
	{
		m_Type = AT_CHARACTER;
		m_Size = sizeof(CharacterParams);
	}

	virtual bool VInit(std::istrstream &in){ return true; }
	virtual bool VSerialize(std::ostrstream &out){ return true; }
};


// Base class for all the game components
class Actor: public IActor
{
protected:
	shared_ptr<ActorParams>		m_params;
	int							m_elapsedTime;
	int							m_timeToStart;
	BuffList					m_buffs;
public:
	Actor();
	Actor(shared_ptr<ActorParams> p);

	virtual Mat4x4 const &VGetMat() {return m_params->m_Mat;}
	virtual void VSetMat(const Mat4x4 &m) {m_params->m_Mat = m;}
	virtual void VOnUpdate(int deltaMS);
	virtual shared_ptr<ActorParams> VGet() {return m_params;}
	virtual void VSetId(ActorId id) {m_params->m_Id = id;}
	virtual void VSetParams(shared_ptr<ActorParams> p) {m_params = p;}
	virtual bool VTakeDamage(int damage);
	virtual void VSetDirection(Vec3 b);
	virtual void VApplyBuff(shared_ptr<IBuff> buff);
};


// Used to do visual effects
class EffectActor : public Actor
{
protected:
	float			m_size;
public:
	EffectActor(): Actor() {}
	EffectActor(shared_ptr<ActorParams> p, float size) : Actor(p), m_size(size) {}
	virtual float VGetRadius() {return m_size;}
};

// A missle that will track it's target until it gets close enough.
class MissileActor : public Actor
{
	ActorId m_target;
	ActorId m_tower;
	Mat4x4 m_location;

public:
	MissileActor(shared_ptr<MissileParams> p, ActorId id, ActorId tar): Actor(p), m_tower(id),m_target(tar),m_location(Mat4x4::g_Identity) {}
	virtual void VOnUpdate(int deltaMS);
	virtual void SetTarget(ActorId id);
	virtual void OnFire(ActorId id);
};

// Modifies some part of the actor
class Buff: public IBuff
{
protected:
	ActorId m_id;
	BuffType m_type;
	int		m_time;

public:
	Buff(ActorId id, BuffType type, int time): m_id(id),m_type(type),m_time(time) {}
	virtual void VApply() {}
	virtual void VRemove() {}
	virtual bool VOnUpdate(int deltaMS) { return false;}
	virtual BuffType VGetType() {return m_type;}
	virtual ActorId VGetActorId() {return m_id;}
};

// A buff that slows the target
class Slow: public Buff
{
public:
	Slow(ActorId id):Buff(id, BT_ICE, 1100) {}
	virtual void VApply();
	virtual void VRemove();
	virtual bool VOnUpdate(int deltaMS);
};
#include "Actors.h"
#include "Game.h"


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Slow///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Changes the actor's speed when applied.
void Slow::VApply()
{
	/*shared_ptr<IActor> actor = g_App->m_pGame->GetActor(m_id);

	if (actor)
	{
		int speed = actor->VGet()->m_speed;
		speed = speed/2;
		actor->VGet()->m_speed = speed;
	}*/
}

// Resets the actor's speed when removed.
void Slow::VRemove()
{
	/*shared_ptr<IActor> actor = g_App->m_pGame->GetActor(m_id);

	if (actor)
	{
		int speed = actor->VGet()->m_speed;
		speed = speed * 2;
		actor->VGet()->m_speed = speed;
	}*/
}

// Slowly ticks down until it is removed.
bool Slow::VOnUpdate(int deltaMS)
{
	m_time -= deltaMS;
	if (m_time <= 0)
		return true;

	return false;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Actor//////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Default constructor.
Actor::Actor()
{
	shared_ptr<ActorParams> p;
	m_params = p;
	m_elapsedTime = 0;
	m_timeToStart = rand() % 3000;
}

// Builds the actor out from the actor params.
Actor::Actor(shared_ptr<ActorParams> p)
{
	m_params = p;
	m_elapsedTime = 0;
	m_timeToStart = rand() % 3000;
}

// Updates the buffs on the actor and then checks if there is place set to move the actor to.
void Actor::VOnUpdate(int elapsedTime)
{
	// move the character if there is a matrix
	const int frameUpdate = 10;

	for (BuffList::iterator it = m_buffs.begin(); it != m_buffs.end();)
	{
		if ((*it)->VOnUpdate(elapsedTime))
		{
			(*it)->VRemove();
			it = m_buffs.erase(it);
		}
		else
			it++;
	}
}

// Will face the actor in the direction given from its current location
void Actor::VSetDirection(Vec3 B)
{

}

// Gives damage to the actor and sends an event if it dies.
bool Actor::VTakeDamage(int damage)
{
	/*m_params->m_life -= damage;
	if (m_params->m_life < 0)
	{
		safeQueueEvent(EventPtr (SAFE_NEW Evt_Remove_Actor(m_params->m_Id)));
		return 0;
	}*/

	return 1;
}

// Adds a buff on the actor. Can only have 1 buff of each type going at a time.
void Actor::VApplyBuff(shared_ptr<IBuff> buff)
{
	for (BuffList::iterator it = m_buffs.begin(); it != m_buffs.end(); it++)
		if ( (*it)->VGetType() == buff->VGetType())
			return;

	m_buffs.push_back(buff);
	buff->VApply();
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////MissileActor///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Moves the missile towards its target and does damage if it gets close.
// Similar to the normal actor movement.
void MissileActor::VOnUpdate(int deltaMS)
{

}

// Missiles on fire function. Used to conform to TowerActor, but without the luascript
void MissileActor::OnFire(ActorId id)
{

}

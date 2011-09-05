#include "StdHeader.h"
#include "EventListener.h"



IEventManager * IEventManager::Get()
{
	return g_pEventMgr;
}

IEventManager::IEventManager(
	char const * const pName,
	bool setAsGlobal )
{
	if ( setAsGlobal )
		g_pEventMgr = this;
}

IEventManager::~IEventManager()
{
	if ( g_pEventMgr == this )
		g_pEventMgr = NULL;
}

bool safeAddListener (EventListenerPtr const & inHandler, EventType const & inType)
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->addListener(inHandler, inType);
}

bool safeDelListener (EventListenerPtr const & inHandler, EventType const & inType)
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->delListener(inHandler, inType);
}

bool safeTriggerEvent (Event const & inEvent)
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->trigger(inEvent);
}

bool safeQueEvent (EventPtr const & inEvent)
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->queueEvent(inEvent);
}

bool safeAbortEvent (EventType const & inType, bool allOfType)
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->abortEvent(inType, allOfType);
}

bool safeTickEventManager (unsigned long maxMillis )
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->tick(maxMillis);
}

bool safeValidateType (EventType const & inType)
{
	assert(IEventManager::Get() && "No Event Manager!");
	return IEventManager::Get()->validateType(inType);
}
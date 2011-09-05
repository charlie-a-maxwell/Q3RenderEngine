#pragma once
#include "StdHeader.h"
#include "EventSystem\event.h"

class IEventListener
{
public:
	explicit IEventListener () {};
	virtual ~IEventListener () {};

	//Returns ascii text name of this listener
	virtual char const * GetName (void)=0;

	virtual bool HandleEvent( Event const & event ) =0;
	//return false if event is NOT consumed and true if it is
};

typedef shared_ptr<IEventListener>   EventListenerPtr;

typedef shared_ptr<Event>           EventPtr;

class IEventManager
{
public:
	enum eConstants { kINFINITE= 0xffffffff};

	explicit IEventManager(char const * const pName, bool setAsGlobal);
	

	virtual ~IEventManager ();
	

	virtual bool addListener (EventListenerPtr const & inHandler, EventType const & inType) =0;
	virtual bool delListener (EventListenerPtr const & inHandler, EventType const & inType) =0;
	virtual bool trigger (Event const & inEvent ) const =0;
	virtual bool queueEvent (EventPtr const & inEvent) =0;
	virtual bool abortEvent (EventType const & inType, bool allOfType = false) =0;
	virtual bool tick (unsigned long maxMillis = kINFINITE ) =0;
	virtual bool validateType (EventType const & inType) const =0;


private:

	static IEventManager * Get();

	friend bool safeAddListener (EventListenerPtr const & inHandler, EventType const & inType);
	friend bool safeDelListener (EventListenerPtr const & inHandler, EventType const & inType);

	friend bool safeTriggerEvent (Event const & inEvent) ;

	friend bool safeQueEvent (EventPtr const & inEvent);

	friend bool safeAbortEvent (EventType const & inType, bool allOfType = false);

	friend bool safeTickEventManager (unsigned long maxMillis = kINFINITE) ;

	friend bool safeValidateType (EventType const & inType);
};

bool safeAddListener( EventListenerPtr const & inHandler,
					  EventType const & inType );
	
bool safeDelListener( EventListenerPtr const & inHandler,
					  EventType const & inType );

bool safeTriggerEvent( Event const & inEvent );

bool safeQueEvent( EventPtr const & inEvent );

bool safeAbortEvent( EventType const & inType,
					 bool allOfType /* = false */ );

bool safeTickEventManager( unsigned long maxMillis
					/* = IEventManager::kINFINITE */ );

bool safeValidateEventType( EventType const & inType );

static IEventManager * g_pEventMgr = NULL;

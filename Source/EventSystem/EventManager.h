#pragma once

#include "StdHeader.h"
#include "EventSystem\EventListener.h"
#include <vector>
#include <list>
#include <map>
#include <set>

typedef std::vector<EventListenerPtr> EventListenerList;
typedef std::vector<EventType> EventTypeList;

class EventManager: public IEventManager
{
public:

	explicit EventManager(char const * const pName, bool setAsGlobal);
	virtual ~EventManager();

	bool addListener (EventListenerPtr const & inHandler, EventType const & inType) ;
	bool delListener (EventListenerPtr const & inHandler, EventType const & inType) ;
	bool trigger (Event const & inEvent ) const ;
	bool queueEvent (EventPtr const & inEvent) ;
	bool abortEvent (EventType const & inType, bool allOfType = false) ;
	bool tick (unsigned long maxMillis = kINFINITE ) ;
	bool validateType (EventType const & inType) const ;

	EventListenerList getListenerList(EventType const & eventType) const;
	EventTypeList getTypeList () const;

private:

	typedef std::set<EventType>		EventTypeSet;
	typedef std::pair<EventTypeSet::iterator, bool> EventTypeSetIRes;
	typedef std::list<EventListenerPtr> EventListenerTable;
	typedef std::map<unsigned int, EventListenerTable> EventListenerMap;

	typedef std::pair<unsigned int, EventListenerTable> EventListenerMapEnt;
	typedef std::pair<EventListenerMap::iterator,bool> EventListenerMapIRes;

	typedef std::list<EventPtr> EventQueue;

	enum eConstants {kNumQueues = 2};

	EventTypeSet m_typeList;
	EventListenerMap m_registry;
	EventQueue m_queues[kNumQueues];

	int m_activeQueue;

};
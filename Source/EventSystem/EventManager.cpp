#pragma once
#include "StdHeader.h"
#include "EventSystem\EventManager.h"
#include <windows.h>


EventManager::EventManager(char const * const pName, bool setAsGlobal)
:IEventManager(pName,setAsGlobal), m_activeQueue(0)
{
}

EventManager::~EventManager()
{
	m_activeQueue = 0;
}

bool EventManager::addListener(EventListenerPtr const & inListener, EventType const & inType)
{
	if( !validateType (inType))
		return false;

	EventTypeSet::iterator evit = m_typeList.find(inType);

	//not in list
	if(evit== m_typeList.end())
	{
		EventTypeSetIRes ires = m_typeList.insert(inType);

		if(ires.second==false)
			return false;

		if(ires.first==m_typeList.end())
			return false;

		evit = ires.first;

	}

	EventListenerMap::iterator elmit = m_registry.find(inType.getIdent());

	//not in list
	if(elmit == m_registry.end())
	{
		EventListenerMapIRes elmIRes = m_registry.insert(EventListenerMapEnt(inType.getIdent(),
			EventListenerTable()));

		//check to see if it inserted correctly
		if(elmIRes.second == false)
			return false;

		//empty list?
		if(elmIRes.first== m_registry.end())
			return false;

		elmit = elmIRes.first;
	}

	EventListenerTable & evlTable = (*elmit).second;

	for(EventListenerTable::iterator it = evlTable.begin(), itEnd = evlTable.end(); it!=itEnd; it++)
	{
		bool bListenerMatch = (*it == inListener);

		if(bListenerMatch)
			return false;
	}

	//event listener not in map
	evlTable.push_back(inListener);

	return true;
}

bool EventManager::delListener(EventListenerPtr const & inListener, EventType const & inType)
{
	//check if it is a valid type
	if( !validateType(inType))
		return false;

	bool rc = false;

	for( EventListenerMap::iterator it = m_registry.begin(), itEnd = m_registry.end(); it!= itEnd; it++)
	{
		unsigned int const kEventId = it->first;

		EventListenerTable & table = it->second;

		for(EventListenerTable::iterator it2 = table.begin(), it2End = table.end(); it2!= it2End; it2++)
		{
			if(*it2 == inListener)
			{
				//match found
				table.erase(it2);

				rc=true;

				break;
			}
		}
	}

	return rc;
}

bool EventManager::trigger(Event const & inEvent) const
{
	if(!validateType(inEvent.getType()))
		return false;

	EventListenerMap::const_iterator itWC = m_registry.find(0);

	if(itWC != m_registry.end())
	{
		EventListenerTable const & table = itWC->second;

		bool processed = false;

		for(EventListenerTable::const_iterator it2 = table.begin(), it2End = table.end(); it2!= it2End; it2++)
		{
			(*it2)->HandleEvent(inEvent);
		}
	}

	EventListenerMap::const_iterator it = m_registry.find(inEvent.getType().getIdent());

	if(it==m_registry.end())
		return false;

	EventListenerTable const & table = it->second;

	bool processed = false;

	for(EventListenerTable::const_iterator it2 = table.begin(), it2End = table.end(); it2!=it2End; it2++)
	{
		if( (*it2)->HandleEvent(inEvent))
		{
			processed=true;
		}
	}

	return processed;
}

bool EventManager::queueEvent(EventPtr const & inEvent) 
{
	assert(m_activeQueue >= 0);
	assert(m_activeQueue < kNumQueues);

	if(!validateType(inEvent->getType()))
		return false;

	EventListenerMap::const_iterator it= m_registry.find( inEvent->getType().getIdent());

	if(it== m_registry.end())
	{
		//if global listener is not active, then abort queue add
		EventListenerMap::const_iterator itWC = m_registry.find(0);

		if(itWC==m_registry.end())
		{
			return false;
		}
	}

	m_queues[m_activeQueue].push_back(inEvent);

	return true;
}

bool EventManager::abortEvent(EventType const & inType, bool allOfType)
{
	assert( m_activeQueue >= 0);
	assert( m_activeQueue < kNumQueues);

	if(!validateType(inType))
		return false;

	EventListenerMap::iterator it = m_registry.find(inType.getIdent());

	if(it== m_registry.end() )
		return false;

	bool rc = false;

	for(EventQueue::iterator it = m_queues[m_activeQueue].begin(), itEnd = m_queues[m_activeQueue].end();
			it!= itEnd; it++)
	{
		if( (*it)->getType()= inType)
		{
			m_queues[m_activeQueue].erase(it);
			rc = true;
			if(!allOfType)
				break;
		}
	}

	return rc;
}

bool EventManager::tick(unsigned long maxMillis)
{
	unsigned long curMs = GetTickCount();
	unsigned long maxMs = maxMillis == IEventManager::kINFINITE ? IEventManager::kINFINITE : (curMs+maxMillis);

	EventListenerMap::const_iterator itWC = m_registry.find(0);

	//swap active queues
	int	queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue+1)%kNumQueues;

	m_queues[m_activeQueue].clear();

	while( m_queues[queueToProcess].size() > 0)
	{
		EventPtr event = m_queues[queueToProcess].front();

		m_queues[queueToProcess].pop_front();

		EventType const & eventType = event->getType();
		EventListenerMap::const_iterator itListeners = m_registry.find(eventType.getIdent());

		if( itWC != m_registry.end() )
		{
			EventListenerTable const & table = itWC->second;

			bool processed = false;

			for(EventListenerTable::const_iterator it2 = table.begin(), it2End=table.end();
				it2!=it2End; it2++)
			{
				(*it2)->HandleEvent(*event);
			}
		}
		
		//no listeners for this event type
		if (itListeners == m_registry.end() )
			continue;

		unsigned int const kEventId = itListeners->first;
		EventListenerTable const & table = itListeners->second;

		for(EventListenerTable::const_iterator it = table.begin(), itEnd = table.end();
			it!=itEnd; it++)
		{
			if( (*it)->HandleEvent(*event))
			{
				break;
			}
		}

		curMs= GetTickCount();

		if(maxMillis != IEventManager::kINFINITE)
		{
			if(curMs >= maxMs)
				break;
		}
	}

	//move the queue to the active queue
	bool queueFlushed = (m_queues[queueToProcess].size() ==0);

	if(!queueFlushed)
	{
		while(m_queues[queueToProcess].size() > 0)
		{
			EventPtr event = m_queues[queueToProcess].back();

			m_queues[queueToProcess].pop_back();

			m_queues[m_activeQueue].push_front(event);
		}
	}

	return queueFlushed;
}

bool EventManager::validateType(EventType const & inType) const
{
	if(inType.getIdentStr()== NULL)
		return false;

	if(inType.getIdent() == 0 && (strcmp(inType.getIdentStr(),wildCard)!=0))
		return false;

	EventTypeSet::const_iterator evIt= m_typeList.find(inType);

	if(evIt!=m_typeList.end())
	{
		EventType const & known = *evIt;

		char const * const kKnownTag = known.getIdentStr();
		char const * const kNewTag = inType.getIdentStr();

		int cv = strcmp(kKnownTag,kNewTag);

		assert(cv == 0 && "You are horribly unluckly");

		if(cv!= 0)
			return false;
	}

	return true;
}

EventListenerList EventManager::getListenerList(EventType const & eventType) const
{
	if(!validateType(eventType))
		return EventListenerList();

	EventListenerMap::const_iterator itListeners = m_registry.find(eventType.getIdent());

	if(itListeners == m_registry.end())
		return EventListenerList();

	EventListenerTable const & table = itListeners->second;

	if(table.size()==0)
		return EventListenerList();

	EventListenerList result;

	result.reserve(table.size() );

	for(EventListenerTable::const_iterator it = table.begin(), itEnd= table.end(); it!=itEnd; it++)
	{
		result.push_back(*it);
	}

	return result;
}

EventTypeList EventManager::getTypeList() const
{
	if(m_typeList.size()==0)
		return EventTypeList();

	EventTypeList result;

	result.reserve( m_typeList.size() );
	
	for(EventTypeSet::const_iterator it = m_typeList.begin(), itEnd = m_typeList.end();
		it!=itEnd; it++)
	{
		result.push_back(*it);
	}

	return result;
}
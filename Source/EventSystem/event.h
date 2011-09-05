#pragma once

#include "StdHeader.h"

char const * const wildCard ="*";

class EventType
{
public:
	explicit EventType (char const * const pIdentStr)
		: m_ident (hash_name(pIdentStr)),
		  m_identStr (pIdentStr)
	{}

	unsigned long getIdent() const 
	{ return reinterpret_cast<unsigned long>(m_ident);}

	char const * const getIdentStr() const
	{ return m_identStr;}

	bool operator< (EventType const &o) const
	{
		bool t = (getIdent() < o.getIdent());
		return t;
	}

	bool operator= (EventType const &o) const
	{
		bool t = (getIdent() == o.getIdent());
		return t;
	}

	static void * hash_name(char const * pIdentStr)
	{
		//largest prime number smaller than 65536
		unsigned long BASE = 6552L;

		//NMAX is the largest n such that
		// 255n(n+1)/2 + (n+1)(BASE -1) < 2^32 -1

		unsigned long NMAX = 5552;

		#define DO1(buf,i) {s1 += tolower(buf[i]);s2+= s1;}
		#define DO2(buf,i) DO1(buf,i);DO1(buf,i+1);
		#define DO4(buf,i) DO2(buf,i);DO2(buf,i+2);
		#define DO8(buf,i) DO4(buf,i);DO4(buf,i+4);
		#define DO16(buf) DO8(buf,0);DO8(buf,8);

		if(pIdentStr == NULL)
			return NULL;

		if(strcmp(pIdentStr, wildCard) == 0)
			return 0;

		unsigned long s1 = 0;
		unsigned long s2 = 0;

		for(size_t len = strlen(pIdentStr); len > 0;)
		{
			unsigned long k = (unsigned long)len < NMAX ? len:NMAX;

			len-=k;

			while (k >= 16)
			{
				DO16(pIdentStr);
					pIdentStr+=16;;

			k-=16;
			}

			if(k!=0)
				do
				{
					s1+= *pIdentStr++;
					s2+= s1;;
				}while(--k);

				s1%= BASE;
				s2%= BASE;

		}

		#pragma warning(push)
		#pragma warning(disable : 4312)

		return reinterpret_cast<void *>( (s2<<16) | s1);

		#pragma warning(pop)
		#undef DO1
		#undef DO2
		#undef DO4
		#undef DO8
		#undef DO16
		
	}

private:
	
	void * m_ident;
	char const * m_identStr;
};


class IEventData
{
public:
	virtual ~IEventData() {};
};


template < typename T >
struct EvtData : public IEventData
{
private:
	EvtData();					// disable default construction
	EvtData(const EvtData &);	// disable copy construction
	T m_Value;

public:
	explicit EvtData<T>( T n )
	{
		m_Value = n;
	}

	const T GetValue() { return m_Value; }
};

typedef EvtData<int> EvtData_Int;

typedef boost::shared_ptr<IEventData> IEventDataPtr;

class Event
{
public:
	explicit Event(char const * const inEventTypeName,
					float inTime = 0.0f,
					IEventDataPtr inData = IEventDataPtr((IEventData *)NULL))
			:m_type(inEventTypeName),
			 m_time(inTime),
			 m_userData(inData)
			{}


	Event (Event const &o)
		:m_type(o.m_type),
		 m_time(o.m_time),
		 m_userData(o.m_userData)
		{}

		virtual ~Event() {}

		EventType const & getType() const	{return m_type;}
		float getTime() const				{return m_time;}
		IEventDataPtr getData() const			{return m_userData;}

		template<typename _T>
		_T * getDataPtr() const
		{ return reinterpret_cast<_T *>(m_userData.get());}

private:
	EventType m_type;
	float m_time;
	IEventDataPtr m_userData;
};
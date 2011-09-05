#pragma once
//========================================================================
// CProcess.h : Defines a simple cooperative multitasker
//
// Part of the GameCode2 Application
//
// GameCode2 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 2nd Edition" by Mike McShaffry, published by
// Paraglyph Press. ISBN: 1-932111-91-3
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the author a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: http://www.amazon.com/exec/obidos/ASIN/1932111913/gamecodecompl-20/
//
// There's also a companion web site at http://www.mcshaffry.com/GameCode/portal.php
//
// (c) Copyright 2005 Michael L. McShaffry
//
// This work is licensed under the Creative Commons Attribution-ShareAlike License. 
// To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/1.0/ 
// or send a letter to:
//      Creative Commons
//      559 Nathan Abbott Way
//      Stanford, California 94305, USA.
//
//========================================================================


//========================================================================
//  Content References in Game Coding Complete - 2nd Edition
// 
//  class CProcess			- Chapter 6, page 171-179
//  class CProcessManager	- Chapter 6, page 171-179
//  class CWaitProcess		- Chapter 6, page 179-181
//========================================================================


#include <list>

//////////////////////////////////////////////////////////////////////
// Enums
//////////////////////////////////////////////////////////////////////

// This process type enumeration is obviously subject to changes
// based on the game design, for example, we are assuming the game will
// have separate behaviors for voice, music, and sound effects
// when in actuality, this engine will play all sound processes the same way

enum PROCESS_TYPE
{
	PROC_NONE,
	PROC_WAIT,
	PROC_SPRITE,
	PROC_CONTROL,
	PROC_SCREEN,
	PROC_MUSIC,
	PROC_SOUNDFX,
	PROC_INTERPOLATOR,
	PROC_GAMESPECIFIC
};


//////////////////////////////////////////////////////////////////////
// Flags
//////////////////////////////////////////////////////////////////////

static const int PROCESS_FLAG_ATTACHED		= 0x00000001;


//////////////////////////////////////////////////////////////////////
// CProcess Description
//
// This is a base class whose instantiations attach to the CProcessManager,
// which updates them once per game loop. Useful for creating animations,
// game objects that need ticking, etc.
//
//////////////////////////////////////////////////////////////////////

class CProcess 
{
	friend class CProcessManager;

protected:
	int					m_iType;		// type of process running
	bool				m_bKill;		// tells manager to kill and remove
	bool				m_bActive;
	bool				m_bPaused;
	bool				m_bInitialUpdate;	// initial update?
	shared_ptr<CProcess>	m_pNext;
	
private:
	unsigned int	m_uProcessFlags;

public:
	CProcess(int ntype, unsigned int uOrder = 0);
	virtual ~CProcess();	

public:

	virtual bool			IsDead(void) const { return(m_bKill);};
	virtual void			Kill();
	
	virtual int				GetType(void) const { return(m_iType); };
	virtual void			SetType(const int t) { m_iType = t; };

	virtual	bool			IsActive(void) const { return m_bActive; };
	virtual void			SetActive(const bool b) { m_bActive = b; };
	virtual	bool			IsAttached()const;
	virtual	void			SetAttached(const bool wantAttached);

	virtual bool			IsPaused(void) const { return m_bPaused; };
	virtual void			TogglePause() {m_bPaused = !m_bPaused;}	// call to pause a process
	
	bool					IsInitialized()const { return ! m_bInitialUpdate; };

	shared_ptr<CProcess> const GetNext(void) const { return(m_pNext);}
	virtual void			SetNext(shared_ptr<CProcess> nnext);
	
	virtual void			OnUpdate(const int deltaMilliseconds);
	virtual void			OnInitialize(){};

private:
	CProcess();						//disable default construction
	CProcess(const CProcess& rhs);	//disable copy construction
};

inline void CProcess::OnUpdate( const int deltaMilliseconds )
{
	if ( m_bInitialUpdate )
	{
		OnInitialize();
		m_bInitialUpdate = false;
	}
}


/////////////////////////////////////////////////////////////////////////////
// ProcessList Description
//
// ProcessList is a list of smart CProcess pointers.
//
//////////////////////////////////////////////////////////////////////

typedef std::list<shared_ptr<CProcess> > ProcessList;


/////////////////////////////////////////////////////////////////////////////
// CProcessManager Description
//
// CProcessManager is a container for CProcess objects.
//
//////////////////////////////////////////////////////////////////////

class CProcessManager
{
public:
	void UpdateProcesses(int deltaMilliseconds);
	void DeleteProcessList();
	bool IsProcessActive( int nType );
	void Attach( shared_ptr<CProcess> pProcess );
	bool HasProcesses();

protected:
	ProcessList	m_ProcessList;	

private:
	void Detach( shared_ptr<CProcess> pProcess );
};


//////////////////////////////////////////////////////////////////////
// CWaitProcess Description
//
// This process forces a delayed execution of a dependant process.
//
// CWaitProcess Example
//   CWaitProcess *wait = new CWaitProcess(2000);
//	 MyOtherProcess *other = new MyOtherProcess;
//   wait->SetNext(other);
// 
//////////////////////////////////////////////////////////////////////

class CWaitProcess : public CProcess
{
protected:
	unsigned int	m_uStart;
	unsigned int	m_uStop;

public:
	CWaitProcess(unsigned int iNumMill );
	virtual void OnUpdate(const int deltaMilliseconds);
};




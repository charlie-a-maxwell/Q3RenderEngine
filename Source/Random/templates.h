#pragma once
//========================================================================
// Templates.h : Some useful templates
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
//  Content References in Game Coding Complete 2nd Edition
// 

//========================================================================
//
//  Original Code written at Compulsive Development
//
//========================================================================

//========================================================================
//  Content References in Game Coding Complete 2nd Edition
// 
//	class singleton -			Chapter 3, page 62-63
//  class optional -			Chapter 3, page 80-86
//
//========================================================================


//-----------------------------------------------------------------------------
// singleton template manages setting/resettting global variables.
//-----------------------------------------------------------------------------

template <class T>
class singleton
{
	T m_OldValue;
	T* m_pGlobalValue;

public:
	singleton(T newValue, T* globalValue)
	{ 
		m_pGlobalValue = globalValue;
		m_OldValue = *globalValue; 
		*m_pGlobalValue = newValue;
	}

	~singleton() { *m_pGlobalValue = m_OldValue; }
};

//////////////////////////////////////////////////////////////////////////////
// optional.h
//
// An isolation point for optionality, provides a way to define
// objects having to provide a special "null" state.
//
// In short:
//
// struct optional<T>
// {
//     bool m_bValid;
//
//	   T	m_data;
// };
//
//

#include <new>
#include <assert.h>

class optional_empty { };

template <unsigned long size>
class optional_base
{
public:
    // Default - invalid.

    optional_base() : m_bValid(false) { }

    optional_base & operator = (optional_base const & t)
    {
		m_bValid = t.m_bValid;
		return * this;
    }

	//Copy constructor
    optional_base(optional_base const & other)
		: m_bValid(other.m_bValid)  { }

	//utility functions
	bool const valid() const		{ return m_bValid; }
	bool const invalid() const		{ return !m_bValid; }

protected:
    bool m_bValid;
    char m_data[size];  // storage space for T
};

template <class T>
class optional : public optional_base<sizeof(T)>
{
public:
    // Default - invalid.

    optional()	 {    }
    optional(T const & t)  { construct(t); m_bValid = (true);  }
	optional(optional_empty const &) {	}

    optional & operator = (T const & t)
    {
        if (m_bValid)
        {
            * GetT() = t;
        }
        else
        {
            construct(t);
			m_bValid = true;	// order important for exception safety.
        }

        return * this;
    }

	//Copy constructor
    optional(optional const & other)
    {
		if (other.m_bValid)
		{
			construct(* other);
            m_bValid = true;	// order important for exception safety.
		}
    }

    optional & operator = (optional const & other)
    {
		assert(! (this == & other));	// don't copy over self!
		if (m_bValid)
		{						// first, have to destroy our original.
			m_bValid = false;	// for exception safety if destroy() throws.
								// (big trouble if destroy() throws, though)
			destroy();
		}

		if (other.m_bValid)
		{
			construct(* other);
			m_bValid = true;	// order vital.

		}
		return * this;
    }


	bool const operator == (optional const & other) const
	{
		if ((! valid()) && (! other.valid())) { return true; }
		if (valid() ^ other.valid()) { return false; }
		return ((* * this) == (* other));
	}

	bool const operator < (optional const & other) const
	{
		// equally invalid - not smaller.
		if ((! valid()) && (! other.valid()))   { return false; }

		// I'm not valid, other must be, smaller.
		if (! valid())	{ return true; }

		// I'm valid, other is not valid, I'm larger
		if (! other.valid()) { return false; }

		return ((* * this) < (* other));
	}

    ~optional() { if (m_bValid) destroy(); }

	// Accessors.

	T const & operator * () const			{ assert(m_bValid); return * GetT(); }
	T & operator * ()						{ assert(m_bValid); return * GetT(); }
	T const * const operator -> () const	{ assert(m_bValid); return GetT(); }
	T		* const operator -> ()			{ assert(m_bValid); return GetT(); }

	//This clears the value of this optional variable and makes it invalid once again.
	void clear()
	{
		if (m_bValid)
		{
			m_bValid = false;
			destroy();
		}
	}

	//utility functions
	bool const valid() const		{ return m_bValid; }
	bool const invalid() const		{ return !m_bValid; }

private:


    T const * const GetT() const { return reinterpret_cast<T const * const>(m_data); }
    T * const GetT()			 { return reinterpret_cast<T * const>(m_data);}
	void construct(T const & t)  { new (GetT()) T(t); }
    void destroy() { GetT()->~T(); }
};



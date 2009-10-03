/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Bloody.Rabbit
*/

#ifndef __REF_PTR_H__INCL__
#define __REF_PTR_H__INCL__

/**
 * \brief Reference counting based smart pointer.
 *
 * This smart pointer cares about acquiring/releasing reference
 * of the stored object. Manual to be as performant as possible.
 *
 * \author Bloody.Rabbit
 */
template<typename X>
class RefPtr
{
public:
	/**
	 * Primary constructor.
	 *
	 * @param[in] p Pointer to object to be referenced.
	 */
	explicit RefPtr(X* p = NULL)
	: mPtr( p )
	{
		if( *this )
			(*this)->IncRef();
	}
	/**
	 * Copy constructor.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	RefPtr(const RefPtr& oth)
	: mPtr( oth.get() )
	{
		if( *this )
			(*this)->IncRef();
	}
	/**
	 * Casting copy constructor.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	template<typename Y>
	RefPtr(const RefPtr<Y>& oth)
	: mPtr( oth.get() )
	{
		if( *this )
			(*this)->IncRef();
	}

	/**
	 * Destructor, releases reference.
	 */
	~RefPtr()
	{
		if( *this )
			(*this)->DecRef();
	}

	/**
	 * Copy operator.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	RefPtr& operator=(const RefPtr& oth)
	{
		if( *this )
			(*this)->DecRef();

		mPtr = oth.get();

		if( *this )
			(*this)->IncRef();

		return *this;
	}
	/**
	 * Casting copy operator.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	template<typename Y>
	RefPtr& operator=(const RefPtr<Y>& oth)
	{
		if( *this )
			(*this)->DecRef();

		mPtr = oth.get();

		if( *this )
			(*this)->IncRef();

		return *this;
	}

	/**
	 * @return Stored reference.
	 */
	X* get() const { return mPtr; }

	/**
	 * @return True if stores a reference, false otherwise.
	 */
	operator bool() const { return !( get() == NULL ); }

	X& operator*() const { assert( *this ); return *get(); }
	X* operator->() const { assert( *this ); return get(); }

	/**
	 * Compares two references.
	 *
	 * @return True if both references are of same object, false if not.
	 */
	template<typename Y>
	bool operator==(const RefPtr<Y>& oth) const
	{
		return ( get() == oth.get() );
	}

	/**
	 * Acts as static_cast.
	 */
	template<typename Y>
	static RefPtr StaticCast(const RefPtr<Y>& oth)
	{
		return RefPtr( static_cast<X*>( oth.get() ) );
	}

protected:
	X* mPtr;
};

/**
 * \brief Class RefPtr can cooperate with.
 *
 * This class has all stuff needed to cooperate with
 * RefPtr. If you want some of your classes to be
 * reference-counted, derive them from this class.
 *
 * \author Bloody.Rabbit
 */
class RefObject
{
	template<typename X>
	friend class RefPtr;
public:
	/**
	 * \brief Initializes reference count.
	 *
	 * @param[in] initRefCount Initial reference count.
	 */
	RefObject(size_t initRefCount)
	: mRefCount( initRefCount )
	{
	}

	/**
	 * \brief Destructor; must be virtual.
	 *
	 * Must be virtual if proper destructor should be
	 * invoked upon destruction.
	 */
	virtual ~RefObject()
	{
		assert( mRefCount == 0 );
	}

protected:
	void IncRef() const
	{
		mRefCount++;
	}
	void DecRef() const
	{
		assert( mRefCount > 0 );
		mRefCount--;

		if( mRefCount == 0 )
			delete this;
	}

	mutable size_t mRefCount;
};

#endif /* !__REF_PTR_H__INCL__ */


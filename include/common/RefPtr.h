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
 * of the stored object. Manual be to as performant as possible.
 *
 * \author Bloody.Rabbit
 */
template<class _Ty>
class RefPtr
{
public:
	/**
	 * Primary constructor.
	 *
	 * @param[in] p Pointer to object to be referenced.
	 */
	explicit RefPtr(_Ty *p = NULL)
	: mPtr( p )
	{
		if( *this )
			mPtr->IncRef();
	}
	/**
	 * Copy constructor.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	RefPtr(const RefPtr &oth)
	: mPtr( oth.mPtr )
	{
		if( *this )
			mPtr->IncRef();
	}
	/**
	 * Destructor, releases reference.
	 */
	~RefPtr()
	{
		if( *this )
			mPtr->DecRef();
	}

	/**
	 * Copy operator.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	RefPtr &operator=(const RefPtr &oth)
	{
		if( *this )
			mPtr->DecRef();

		mPtr = oth.mPtr;

		if( *this )
			mPtr->IncRef();

		return *this;
	}

	/**
	 * Casting copy constructor.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	template<class _Ty2>
	RefPtr(const RefPtr<_Ty2> &oth)
	: mPtr( oth.get() )
	{
		if( *this )
			mPtr->IncRef();
	}

	/**
	 * Casting copy operator.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	template<class _Ty2>
	RefPtr &operator=(const RefPtr<_Ty2> &oth)
	{
		if( *this )
			mPtr->DecRef();

		mPtr = oth.get();

		if( *this )
			mPtr->IncRef();

		return *this;
	}

	/**
	 * @return Stored reference.
	 */
	_Ty *get() const { return mPtr; }

	/**
	 * @return True if stores a reference, false otherwise.
	 */
	operator bool() const { return ( get() != NULL ); }

	_Ty &operator*() const { return *get(); }
	_Ty *operator->() const { return get(); }

	/**
	 * Compares two references.
	 *
	 * @return True if both references are of same object, false if not.
	 */
	template<class _Ty2>
	bool operator==(const RefPtr<_Ty2> &oth) const
	{
		return ( get() == oth.get() );
	}

	/**
	 * Acts as static_cast.
	 */
	template<class _Ty2>
	static RefPtr StaticCast(const RefPtr<_Ty2> &oth)
	{
		return RefPtr( static_cast<_Ty *>( oth.get() ) );
	}

protected:
	_Ty *mPtr;
};

#endif /* !__REF_PTR_H__INCL__ */


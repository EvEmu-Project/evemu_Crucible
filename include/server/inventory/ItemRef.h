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

#ifndef __ITEM_REF__H__INCL__
#define __ITEM_REF__H__INCL__

class InventoryItem;

class Blueprint;
class Ship;
class CelestialObject;
class Skill;
class Owner;

class Character;
class SolarSystem;
class Station;

/**
 * Smart pointer to maintain InventoryItem's references.
 */
template<class _Ty>
class ItemRef
{
public:
	/**
	 * Primary constructor.
	 *
	 * @param[in] p Pointer to object to be referenced.
	 */
	explicit ItemRef(_Ty *p = NULL)
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
	ItemRef(const ItemRef &oth)
	: mPtr( oth.mPtr )
	{
		if( *this )
			mPtr->IncRef();
	}
	/**
	 * Destructor, releases reference.
	 */
	~ItemRef()
	{
		if( *this )
			mPtr->DecRef();
	}

	/**
	 * Copy operator.
	 *
	 * @param[in] oth Object to copy the reference from.
	 */
	ItemRef &operator=(const ItemRef &oth)
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
	ItemRef(const ItemRef<_Ty2> &oth)
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
	ItemRef &operator=(const ItemRef<_Ty2> &oth)
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
	bool operator==(const ItemRef<_Ty2> &oth) const
	{
		return ( get() == oth.get() );
	}

	/**
	 * Acts as static_cast.
	 */
	template<class _Ty2>
	static ItemRef StaticCast(const ItemRef<_Ty2> &oth)
	{
		return ItemRef( static_cast<_Ty *>( oth.get() ) );
	}

protected:
	_Ty *mPtr;
};

/*
 * Typedefs for all item classes we have:
 */
typedef ItemRef<InventoryItem>      InventoryItemRef;

typedef ItemRef<Blueprint>          BlueprintRef;
typedef ItemRef<Ship>               ShipRef;
typedef ItemRef<CelestialObject>    CelestialObjectRef;
typedef ItemRef<Skill>              SkillRef;
typedef ItemRef<Owner>              OwnerRef;

typedef ItemRef<Character>          CharacterRef;
typedef ItemRef<SolarSystem>        SolarSystemRef;
typedef ItemRef<Station>            StationRef;

#endif /* !__ITEM_REF__H__INCL__ */


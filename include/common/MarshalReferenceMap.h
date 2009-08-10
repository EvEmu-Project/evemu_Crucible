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
	Author:		Captnoord
*/

#ifndef UNMARSHAL_REFERENCE_MAP_H
#define UNMARSHAL_REFERENCE_MAP_H

#include "common.h"
#include "PyRep.h"

/**
 * MarshalReferenceMap.h only implement the UnmarshalReferenceMap, the Marshal reference tech
 * isn't researched/implemented for the moment. a cpp file should be created from the code in
 * this header but for the moment this will do.
 * @note because we don't own the pointer / own the memory of the referenced objects. Its unclear when the refered object is deleted so
 * we have to eighter keep the Object here ( own the pointer ) or make sure that the referenced object know he is a referenced object so
 * it isn't accidentally deleted.
 */

/**
 * The UnmarshalReferenceMap class keeps track of referenced objects within
 * a marshal stream.
 * 
 * @author Captnoord
 */
class UnmarshalReferenceMap
{
public:

	/**
	 * Constructor that sets the expected object count of referenced objects.
	 * @param objectCount is the number of expected referenced objects within the marshal stream, its also known in the client as as MapCount.
	 */
	UnmarshalReferenceMap(const uint32 objectCount) : mStoredObjectCount( objectCount )
	{
		assert( mStoredObjectCount < 0x100 ); // kinda normal..... 256 referenced objects otherwise crash

		mReferenceObjects = new PyRep*[ mStoredObjectCount ];
		memset( mReferenceObjects, 0, mStoredObjectCount * sizeof( PyRep* ) );
		mOrder.resize( objectCount );
	}

	/**
	 * Destructor deletes the dynamic array.
	 */
	~UnmarshalReferenceMap()
	{
		SafeDeleteArray( mReferenceObjects );
	}

	/**
	 * Get a stored referenced Object.
	 * @param location is the index number of the referenced object.
	 * @return A referenced PyRep base Object.
	 */
	EVEMU_INLINE PyRep* GetStoredObject(uint32 location)
	{
		if( ( location - 1 ) < mStoredObjectCount )
			return mReferenceObjects[ location - 1 ];
		else
			return NULL;
	}

	/**
	 * Reserves space for object.
	 * @return Number to be passed to StoreReferencedObject
	 */
	EVEMU_INLINE uint32 ReserveObjectSpace()
	{
		if( mOrder.empty() )
		{
			printf( "Trying to store too many objects.\n" );
			abort(); // crash when we are storing more objects than we expect
		}
		uint32 index = mOrder.front();
		mOrder.erase( mOrder.begin() ); // pop front
		return index;
	}

	/**
	 * Stores a referenced Object.
	 * @param index is number obtained by previous call to ReserveObjectSpace.
	 * @param object is the object that is marked as a object that has many references.
	 */
	EVEMU_INLINE void StoreReferencedObject(uint32 index, PyRep * object)
	{
		if( ( index - 1 ) < mStoredObjectCount )
			mReferenceObjects[ index - 1 ] = object;
	}

	/**
	 * Sets order index.
	 * @param storeIndex is index in order of storing objects.
	 * @param getIndex is index in order of getting objects.
	 */
	EVEMU_INLINE void SetOrderIndex(uint32 storeIndex, uint32 getIndex)
	{
		if(    mOrder.size() == mStoredObjectCount // allow order setup only until first object is stored
		    && ( storeIndex - 1 ) < mStoredObjectCount && ( getIndex - 1 ) < mStoredObjectCount )
		{
			mOrder[ storeIndex - 1 ] = getIndex;
		}
	}

	/**
	 * Get the current stored objects.
	 * @return A unsigned integer that represents the amount of referenced Objects stored.
	 */
	EVEMU_INLINE uint32 GetStoredCount()
	{
		return mStoredObjectCount;
	}

protected:
	/** 
	 * keeps track of the amount of objects that are actually stored
	 */
	const uint32 mStoredObjectCount;

	/**
	 * pointer container to keep track of the pointers...
	 * and of course: "we do not own the pointers in this list"
	 */
	PyRep** mReferenceObjects;

	/**
	 * this vector keeps track of ordering
	 */
	std::vector<uint32> mOrder;
};

#endif//UNMARSHAL_REFERENCE_MAP_H

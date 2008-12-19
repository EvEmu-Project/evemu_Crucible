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
	UnmarshalReferenceMap(const uint32 objectCount) : expectedObjectsCount(objectCount), storedObjectCount(0), storeObjectIndex(0)
	{
		assert(expectedObjectsCount < 0x100); // kinda normal..... 256 referenced objects otherwise crash
		mReferenceObjects = new PyRep*[expectedObjectsCount];
	}

	/**
	 * Get a stored referenced Object.
	 * @param location is the index number of the referenced object.
	 * @return A referenced PyRep base Object.
	 */
	ASCENT_INLINE PyRep* GetStoredObject(uint32 location)
	{
		if (location == 0 || location > storedObjectCount )
			return NULL;
		return mReferenceObjects[location-1];
	}

	/**
	 * Stores a referenced Object.
	 * @param object is the object that is marked as a object that has many references.
	 */
	ASCENT_INLINE void StoreReferencedObject(PyRep * object)
	{
		assert(storeObjectIndex < expectedObjectsCount); // crash when we are storing more objects than we expect
		mReferenceObjects[storeObjectIndex] = object;
		storeObjectIndex++;

		storedObjectCount = storeObjectIndex;
	}

	/**
	 * Get the current stored objects.
	 * @return A unsigned integer that represents the amount of referenced Objects stored.
	 */
	ASCENT_INLINE uint32 GetStoredCount()
	{
		return storedObjectCount;
	}

	/**
	 * Get the expected referenced object count.
	 * @return A unsigned integer that represents the expected object count.
	 */
	ASCENT_INLINE uint32 GetMaxStoredCount()
	{
		return expectedObjectsCount;
	}

protected:
	/** 
	 * keeps track of the amount of objects that are actually stored
	 */
	uint32 storedObjectCount;

	/**
	 * keeps track of the store index
	 */
	uint32 storeObjectIndex;

	/**
	 * max amount of objects that are referenced objects in this stream
	 * this value is supplied by the client (regarding the 'unmarshal' process).
	 */
	const uint32 expectedObjectsCount;

	/**
	 * pointer container to keep track of the pointers...
	 * and of course: "we do not own the pointers in this list"
	 */
	PyRep** mReferenceObjects;
};

#endif//UNMARSHAL_REFERENCE_MAP_H

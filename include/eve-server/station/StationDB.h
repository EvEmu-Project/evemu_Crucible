/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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
	Author:		Zhur
*/


#ifndef __STATIONDB_H_INCL__
#define __STATIONDB_H_INCL__

#include "ServiceDB.h"

class PyRep;

/**
 * a example of a storage class for static db data. not exactly doxygen commented.
 */
class storage
{
public:
	storage() : mLoaded(false) {}

	bool load()
	{
        /* check if its already loaded */
		if (mLoaded == true)
			return true;

		DBQueryResult res;
		if(!sDatabase.RunQuery(res,	"SELECT "
			" solarSystemID,"					// int
			" solarSystemName,"					// string
			" x, y, z,"							// double
			" radius,"							// double
			" security,"						// double
			" constellationID,"					// int
			" factionID,"						// int
			" sunTypeID,"						// int
			" regionID,"                        // int

			//Hack: I think this is dynamic data.... meaning it should be stored somewhere else in the db...
			" NULL AS allianceID,"				// int
			" 0 AS sovereigntyLevel,"			// int
			" 0 AS constellationSovereignty"	// int
			" FROM mapSolarSystems"))
		{
            sLog.Error("Station DB", "Error in storage GetSolarSystem query: %s", res.error.c_str());
			return false;
		}

		/* I am aware of the fact that the next piece of code is spamming the console */
        sLog.Log("Station DB", "Loading Solar systems:");

		DBResultRow row;
		for(unsigned int i = 0; res.GetRow(row); i++)
		{
            mStorageContainer.insert(std::make_pair(row.GetUInt(0), DBRowToRow(row)));

			if ((i % 200) == 0)
				printf(".");
		}        
        printf("\n");
        sLog.Log("Station DB", "Storing solar system data Done");
		mLoaded = true;

		return true;
	}

	// returned pointer doesn't have to be const, but I don't think
	// we would like anyone to change static db data, so ...
	const PyObject* find(uint32 id) const
	{
		DataContainerConstItr Itr = mStorageContainer.find(id);
		if (Itr != mStorageContainer.end())
		{
			return Itr->second;
		}
		return NULL;
	}

	~storage()
	{
		DataContainerItr Itr = mStorageContainer.begin();
		for (; Itr != mStorageContainer.end(); Itr++)
		{
			PyDecRef( Itr->second );
		}
	}

protected:
	typedef std::tr1::unordered_map<uint32, PyObject*>	DataContainer;
	typedef DataContainer::iterator							DataContainerItr;
	typedef DataContainer::const_iterator					DataContainerConstItr;

	DataContainer mStorageContainer;
	bool mLoaded;
};


class StationDB : public ServiceDB
{
public:
	StationDB();

	PyRep *GetSolarSystem(uint32 ssid);
	PyRep *DoGetStation(uint32 ssid);
	PyRep *GetStationItemBits(uint32 sid);

protected:
	static storage thingy;
};

#endif

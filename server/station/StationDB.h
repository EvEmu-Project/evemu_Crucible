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
	Author:		Zhur
*/


#ifndef __STATIONDB_H_INCL__
#define __STATIONDB_H_INCL__

#include "../ServiceDB.h"

#include "dbcore.h"

class PyRep;

class storage;
extern storage * mmthingy;


/**
 * a example of a storage class for static db data. not exactly doxygen commented.
 */
class storage
{
public:
	storage()
	{
		// this should never happen
		if (mStorageContainer.size() != 0)
		{
			printf("double create fuckored whoo get the hell out of here\n");
			assert(false);
		}

		DBQueryResult res;

		if(!GetDatabase.RunQuery(res,	"SELECT "
			" solarSystemID,"					// int
			" solarSystemName,"					// string
			" x, y, z,"							// double
			" radius,"							// double
			" security,"						// double
			" constellationID,"					// int
			" factionID,"						// int
			" sunTypeID,"						// int
			" regionID,"

			//haxor: I think this is dynamic data.... meaning it should be stored somewhere else in the db...
			" NULL AS allianceID,"				// int
			" 0 AS sovereigntyLevel,"			// int
			" 0 AS constellationSovereignty"	// int
			" FROM mapSolarSystems"))
		{
			_log(SERVICE__ERROR, "Error in storage GetSolarSystem query: %s", res.error.c_str());
			return NULL;
		}

		/* I am aware of the fact that the next piece of code is spamming the console */
		printf("Loading Solar systems:.");
		DBResultRow row;
		unsigned int i = 0;
		while(res.GetRow(row))
		{
			mStorageContainer[row.GetUInt(0)] = DBRowToRow(row);

			if (i % 10)
				printf(".");
			i++;
		}
		printf("\nStoring solar system data Done\n");
	}

	PyRepObject* find(uint32 id)
	{
		DataContainerConstItr Itr = mStorageContainer.find(id);
		if (Itr != mStorageContainer.end())
		{
			return (PyRepObject*)((PyRepObject*)Itr->second)->Clone(); // HELL FIRE TALKING ABOUT STATEMENTS I HATE---->>>> Clone?:S:S::S
		}
		return NULL;
	}

	~storage()
	{
		DataContainerItr Itr = mStorageContainer.begin();
		for (; Itr != mStorageContainer.end(); Itr++)
		{
			delete Itr->second;
		}
	}

	typedef std::tr1::unordered_map<uint32, PyRepObject*>	DataContainer;
	typedef DataContainer::iterator							DataContainerItr;
	typedef DataContainer::const_iterator					DataContainerConstItr;

	DataContainer mStorageContainer;
};



class StationDB : public ServiceDB {
public:
	StationDB(DBcore *db);
	virtual ~StationDB();

	PyRep *GetSolarSystem(uint32 ssid);
	PyRep *DoGetStation(uint32 ssid);
	PyRep *GetStationItemBits(uint32 sid);

//protected:
	storage * thingy;
};

#endif

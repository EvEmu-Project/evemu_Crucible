/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

ConfigDB::ConfigDB(DBcore *db)
: ServiceDB(db)
{
}

ConfigDB::~ConfigDB() {
}


PyRep *ConfigDB::GetMultiOwnersEx(const std::vector<uint32> &entityIDs) {
#ifndef WIN32
#warning we need to deal with corporations!
#endif
	//im not sure how this query is supposed to work, as far as what table
	//we use to get the fields from.
	//we only get called for items which are not already sent in the
	// eveStaticOwners cachable object.

	std::string ids;
	ListToINString(entityIDs, ids, "-1");
	
	DBQueryResult res;
	DBResultRow row;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" entity.itemID as ownerID,"
		" entity.itemName as ownerName,"
		" entity.typeID"
		" FROM entity "
		" WHERE itemID in (%s)", ids.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	//this is pretty hackish... will NOT work if they mix things...
	//this was only put in to deal with "new" statics, like corporations.
	if(!res.GetRow(row)) {
		if(!m_db->RunQuery(res,
			"SELECT "
			" ownerID,ownerName,typeID"
			" FROM eveStaticOwners "
			" WHERE ownerID in (%s)", ids.c_str()))
		{
			codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
			return(NULL);
		}
	} else {
		res.Reset();
	}
	
	return(DBResultToTupleSet(res));
}

PyRep *ConfigDB::GetMultiAllianceShortNamesEx(const std::vector<uint32> &entityIDs) {
	
	//im not sure how this query is supposed to work, as far as what table
	//we use to get the fields from.

	std::string ids;
	ListToINString(entityIDs, ids, "-1");
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" entity.itemID as allianceID,"
		" entity.itemName as shortName"	//we likely need to use customInfo or something for this.
		" FROM entity "
		" WHERE entity.typeID = %d"
		"	AND itemID in (%s)", 
			AllianceTypeID,
			ids.c_str()
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToTupleSet(res));
}


PyRep *ConfigDB::GetMultiLocationsEx(const std::vector<uint32> &entityIDs) {

	//im not sure how this query is supposed to work, as far as what table
	//we use to get the fields from.

	bool use_map = false;
	if(!entityIDs.empty()) {
		//this is a big of a hack at this point... basically
		//we are assuming that they only query locations for map items
		// and non-map items disjointly....
		use_map = IsStaticMapItem(entityIDs[0]);
	}
	
	std::string ids;
	ListToINString(entityIDs, ids, "-1");
	
	DBQueryResult res;

	if(use_map) {
		if(!m_db->RunQuery(res,
			"SELECT "
			" mapDenormalize.itemID AS locationID,"
			" mapDenormalize.itemName as locationName,"
			" mapDenormalize.x,"
			" mapDenormalize.y,"
			" mapDenormalize.z"
			" FROM mapDenormalize "
			" WHERE itemID in (%s)", ids.c_str()))
		{
			codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
			return(NULL);
		}
	} else {
		if(!m_db->RunQuery(res,
			"SELECT "
			" entity.itemID AS locationID,"
			" entity.itemName as locationName,"
			" entity.x,"
			" entity.y,"
			" entity.z"
			" FROM entity "
			" WHERE itemID in (%s)", ids.c_str()))
		{
			codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
			return(NULL);
		}
	}

	return(DBResultToTupleSet(res));
}


PyRep *ConfigDB::GetMultiCorpTickerNamesEx(const std::vector<uint32> &entityIDs) {

	std::string ids;
	ListToINString(entityIDs, ids, "-1");
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		"	corporationID, tickerName, "
		"	shape1, shape2, shape3,"
		"	color1, color2, color3 "
		" FROM corporation "
		" WHERE corporationID in (%s)", ids.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowList(res));
}


PyRep *ConfigDB::GetMultiGraphicsEx(const std::vector<uint32> &entityIDs) {

	std::string ids;
	ListToINString(entityIDs, ids, "-1");
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT"
		"	graphicID,url3D,urlWeb,icon,urlSound,explosionID"
		" FROM eveGraphics "
		" WHERE graphicID in (%s)", ids.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowList(res));
}

PyRepObject *ConfigDB::GetUnits() {

	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT "
		" unitID,unitName,displayName"
		" FROM eveUnits "))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToIndexRowset("unitID", res));
}

PyRepObject *ConfigDB::GetMapObjects(uint32 entityID, bool wantRegions, 
	bool wantConstellations, bool wantSystems, bool wantStations)
{
	const char *key = "solarSystemID";
	if(wantRegions) {
		entityID = 3;	/* a little hackish... */
		key = "typeID";
	} else if(wantConstellations) {
		key = "regionID";
	} else if(wantSystems) {
		key = "constellationID";
	} else if(wantStations) {
		key = "solarSystemID";
	}

	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT "
		"	groupID,typeID,itemID,itemName,solarSystemID AS locationID,"
		"	orbitID,"
#ifndef WIN32
#warning hacked 'connector' field in GetMapObjects
#endif
		"	0 as connector,"
		"	x,y,z"
		" FROM mapDenormalize"
		" WHERE %s=%lu", key, entityID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *ConfigDB::GetMap(uint32 solarSystemID) {
	DBQueryResult res;

#ifndef WIN32
#warning a lot of missing data in GetMap
#endif
	//how in the world do they get a list in the freakin rowset for destinations???
	if(!m_db->RunQuery(res,
		"SELECT "
		"	itemID,"
		"	itemName,"
		"	typeID,"
		"	x,y,z,"
		"	solarSystemID AS locationID,"
		"	NULL AS orbitID,"
		"	NULL AS destinations,"
		"	NULL AS xMin,"
		"	NULL AS xMax,"
		"	NULL AS yMin,"
		"	NULL AS yMax,"
		"	NULL AS zMin,"
		"	NULL AS zMax,"
		"	NULL AS luminosity"
		" FROM mapDenormalize"
		" WHERE solarSystemID=%lu", solarSystemID
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}

PyRepObject *ConfigDB::ListLanguages() {
	DBQueryResult res;
	
	//how in the world do they get a list in the freakin rowset for destinations???
	if(!m_db->RunQuery(res,
		"SELECT "
		"	languageID,languageName,translatedLanguageName"
		" FROM languages"
	))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowset(res));
}


PyRep *ConfigDB::GetMultiInvTypesEx(const std::vector<uint32> &entityIDs) {

	std::string ids;
	ListToINString(entityIDs, ids, "-1");
	
	DBQueryResult res;
	
	if(!m_db->RunQuery(res,
		"SELECT"
		"	typeID,groupID,typeName,description,graphicID,radius,"
		"	mass,volume,capacity,portionSize,raceID,basePrice,"
		"	published,marketGroupID,chanceOfDuplicating "
		" FROM invTypes "
		" WHERE typeID in (%s)", ids.c_str()))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}
	
	return(DBResultToRowList(res));
}

PyRep *ConfigDB::GetStationSolarSystemsByOwner(uint32 ownerID) {
	DBQueryResult res;

	if (!m_db->RunQuery(res,
		" SELECT "
		" corporationID, solarSystemID "
		" FROM staStations "
		" WHERE corporationID = %lu ", ownerID
		))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return DBResultToRowset(res);
}

PyRep *ConfigDB::GetCelestialStatistic(uint32 celestialID) {
	DBQueryResult res;
	DBResultRow row;

	if (!m_db->RunQuery(res,
		" SELECT "
		" groupID "
		" FROM eveNames "
		" WHERE itemID = %lu ", celestialID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	if (!res.GetRow(row)) {
		codelog(SERVICE__ERROR, "Unable to find celestial object %lu", celestialID);
		return(NULL);
	}

	uint32 groupID = row.GetUInt(0);

	std::string query = "";

	switch (groupID) {
	case EVEDB::invGroups::Sun:
			query = " SELECT "
					"    CONCAT( FORMAT( temperature, 0 ) , \" K\" ) AS temperature, "
					"    spectralClass, "
					"    FORMAT( luminosity, 5 ) as luminosity, "
					"    CONCAT( FORMAT( CAST( age /60 /60 /24 /365 /1000000 AS UNSIGNED INTEGER ) *1000000, 0 ) , \" Years\" ) AS age, "
					"    CONCAT( FORMAT( radius /1000, 0 ) , \" km\" ) AS radius "
					" FROM mapCelestialStatistics "
					" WHERE celestialID = %lu ";
			break;
	case EVEDB::invGroups::Planet:
			query = " SELECT "
					"    CONCAT( FORMAT( temperature, 0 ) , \" K\" ) AS temperature, "
					"    CONCAT( FORMAT( orbitRadius / 1.49598E11, 3), \" AU\") AS \"Orbit Radius\", "
					"    FORMAT( eccentricity, 3) AS eccentricity, "
					"    CONCAT( FORMAT( massDust / POW(10, ROUND(LOG10(massDust), 0)), 1), \"e+0\", ROUND(LOG10(massDust), 0), \" kg\") AS mass, "
					"    CONCAT( FORMAT( density, 1), \" g/cm^3\") AS density, "
					"    CONCAT( FORMAT( surfaceGravity, 1), \" m/s^2\") AS \"Surface Gravity\", "
					"    CONCAT( FORMAT( escapeVelocity / 1000, 1), \" km/s\") AS \"Escape Velocity\", "
					"    CONCAT( FORMAT( orbitPeriod / 864000, 0), \" days\") AS \"Orbit Period\", "
					"    CONCAT( FORMAT( pressure / 100000, 0), \" kPa\") AS pressure, "
					"    CONCAT( FORMAT( radius /1000, 0), \" km\") AS radius "
					" FROM mapCelestialStatistics "
					" WHERE celestialID = %lu ";
			break;
	case EVEDB::invGroups::Moon:
			query = " SELECT "
					"    CONCAT( FORMAT( temperature, 0 ) , \" K\" ) AS temperature, "
					"    CONCAT( FORMAT( orbitRadius, 0), \" km\") AS \"Orbit Radius\", "
					"    CONCAT( FORMAT( massDust / POW(10, ROUND(LOG10(massDust), 0)), 1), \"e+0\", ROUND(LOG10(massDust), 0), \" kg\") AS mass, "
					"    CONCAT( FORMAT( density, 1), \" g/cm^3\") AS density, "
					"    CONCAT( FORMAT( surfaceGravity, 1), \" m/s^2\") AS \"Surface Gravity\", "
					"    CONCAT( FORMAT( escapeVelocity / 1000, 1), \" km/s\") AS \"Escape Velocity\", "
					"    CONCAT( FORMAT( orbitPeriod / 864000, 3), \" days\") AS \"Orbit Period\", "
					"    CONCAT( FORMAT( pressure / 100000, 0), \" kPa\") AS pressure, "
					"    CONCAT( FORMAT( radius /1000, 0), \" km\") AS radius "
					" FROM mapCelestialStatistics "
					" WHERE celestialID = %lu ";
			break;
	case EVEDB::invGroups::Asteroid_Belt:
			query = " SELECT "
					"    CONCAT( FORMAT( orbitRadius, 0), \" km\") AS \"Orbit Radius\", "
					"    FORMAT( eccentricity, 3) AS eccentricity, "
					"    CONCAT( FORMAT( massDust / POW(10, ROUND(LOG10(massDust), 0)), 1), \"e+0\", ROUND(LOG10(massDust), 0), \" kg\") AS mass, "
					"    CONCAT( FORMAT( density, 1), \" g/cm^3\") AS density, "
					"    CONCAT( FORMAT( orbitPeriod / 864000, 0), \" days\") AS \"Orbit Period\" "
					" FROM mapCelestialStatistics "
					" WHERE celestialID = %lu ";
			break;

	default:
			codelog(SERVICE__ERROR, "Invalid object groupID (%lu) for %lu", groupID, celestialID);
			return (NULL);
	}

	if (!m_db->RunQuery(res, query.c_str(), celestialID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return DBResultToRowset(res);
}

PyRep *ConfigDB::GetTextsForGroup(const std::string & langID, uint32 textgroup) {
	DBQueryResult res;
	if (!m_db->RunQuery(res, "SELECT textLabel, `text` FROM intro WHERE langID = '%s' AND textgroup = %lu", langID.c_str(), textgroup))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return DBResultToRowset(res);
}













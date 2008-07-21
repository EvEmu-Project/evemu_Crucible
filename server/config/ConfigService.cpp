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



#include "ConfigService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"
#include "../common/PyUtils.h"
#include "../packets/General.h"

PyCallable_Make_InnerDispatcher(ConfigService)


ConfigService::ConfigService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "config"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ConfigService, GetMultiOwnersEx)
	PyCallable_REG_CALL(ConfigService, GetMultiLocationsEx)
	PyCallable_REG_CALL(ConfigService, GetMultiAllianceShortNamesEx)
	PyCallable_REG_CALL(ConfigService, GetMultiCorpTickerNamesEx)
	PyCallable_REG_CALL(ConfigService, GetUnits)
	PyCallable_REG_CALL(ConfigService, GetMapObjects)
	PyCallable_REG_CALL(ConfigService, GetMap)
	PyCallable_REG_CALL(ConfigService, GetMapConnections)
	PyCallable_REG_CALL(ConfigService, GetMultiGraphicsEx)
	PyCallable_REG_CALL(ConfigService, GetMultiInvTypesEx)
	PyCallable_REG_CALL(ConfigService, GetStationSolarSystemsByOwner)
	PyCallable_REG_CALL(ConfigService, GetCelestialStatistic)
}

ConfigService::~ConfigService() {
	delete m_dispatch;
}

PyCallResult ConfigService::Handle_GetMultiOwnersEx(PyCallArgs &call) {
	PyRep *result = NULL;

	//parse the PyRep to get the list of IDs to query.
	std::vector<uint32> ids;
	if(ParseIntegerList(call.tuple, "GetMultiOwnersEx", ids)) {
		result = m_db.GetMultiOwnersEx(ids);
	}

	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}

PyCallResult ConfigService::Handle_GetMultiAllianceShortNamesEx(PyCallArgs &call) {
	PyRep *result = NULL;

	//parse the PyRep to get the list of IDs to query.
	std::vector<uint32> ids;
	if(ParseIntegerList(call.tuple, "GetMultiAllianceShortNamesEx", ids)) {
		result = m_db.GetMultiAllianceShortNamesEx(ids);
	}
	
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}


PyCallResult ConfigService::Handle_GetMultiLocationsEx(PyCallArgs &call) {
	PyRep *result = NULL;

	//parse the PyRep to get the list of IDs to query.
	std::vector<uint32> ids;
	if(ParseIntegerList(call.tuple, "GetMultiLocationsEx", ids)) {
		result = m_db.GetMultiLocationsEx(ids);
	}
	
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}

PyCallResult ConfigService::Handle_GetMultiCorpTickerNamesEx(PyCallArgs &call) {
	PyRep *result = NULL;

	//parse the PyRep to get the list of IDs to query.
	std::vector<uint32> ids;
	if(ParseIntegerList(call.tuple, "GetMultiCorpTickerNamesEx", ids)) {
		result = m_db.GetMultiCorpTickerNamesEx(ids);
	}
	
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}

PyCallResult ConfigService::Handle_GetMultiGraphicsEx(PyCallArgs &call) {
	PyRep *result = NULL;

	//parse the PyRep to get the list of IDs to query.
	std::vector<uint32> ids;
	if(ParseIntegerList(call.tuple, "GetMultiGraphicsEx", ids)) {
		result = m_db.GetMultiGraphicsEx(ids);
	}
	
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}



PyCallResult ConfigService::Handle_GetUnits(PyCallArgs &call) {
	PyRep *result = NULL;
	
	result = m_db.GetUnits();
	
	return(result);
}


PyCallResult ConfigService::Handle_GetMap(PyCallArgs &call) {
	Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		return(NULL);
	}
		
	PyRep *result = m_db.GetMap(args.arg);
	
	return(result);
}

PyCallResult ConfigService::Handle_GetMapObjects(PyCallArgs &call) {
/*
  args (entityID,
    wantRegions (given universe),
    wantConstellations (given region),
    wantSystems (given constellation),
    wantStations (given solarsystem),
    unknown (seen 0) )
*/

    /* parsing args the long way until I write a dynamic InlineTuple mechanism */
	
/*  Call_SingleIntegerArg args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Failed to decode arguments");
		return(NULL);
    }*/

	uint32 arg = 0;
	bool wantRegions = false;
	bool wantConstellations = false;
	bool wantSystems = false;
	bool wantStations = false;

	PyRepTuple *packet = call.tuple;
	call.tuple = NULL;
	if(packet->items.size() > 5) {
		//do nothing with this field, we do not understand it.
	}
	if(packet->items.size() > 4) {
		PyRep *v = packet->items[4];
		if(!v->CheckType(PyRep::Integer)) {
			_log(NET__PACKET_ERROR, "Decode Handle_GetMapObjects failed: arg 4 is not an int: %s", 
				v->TypeString());
			delete packet;
			return(NULL);
		}
		PyRepInteger *iii = (PyRepInteger *) v;
		wantStations = (iii->value==0)?false:true;
	}
	if(packet->items.size() > 3) {
		PyRep *v = packet->items[3];
		if(!v->CheckType(PyRep::Integer)) {
			_log(NET__PACKET_ERROR, "Decode Handle_GetMapObjects failed: arg 3 is not an int: %s", 
				v->TypeString());
			delete packet;
			return(false);
		}
		PyRepInteger *iii = (PyRepInteger *) v;
		wantSystems = (iii->value==0)?false:true;
	}
	if(packet->items.size() > 2) {
		PyRep *v = packet->items[2];
		if(!v->CheckType(PyRep::Integer)) {
			_log(NET__PACKET_ERROR, "Decode Handle_GetMapObjects failed: arg 2 is not an int: %s", 
				v->TypeString());
			delete packet;
			return(false);
		}
		PyRepInteger *iii = (PyRepInteger *) v;
		wantConstellations = (iii->value==0)?false:true;
	}
	if(packet->items.size() > 1) {
		PyRep *v = packet->items[1];
		if(!v->CheckType(PyRep::Integer)) {
			_log(NET__PACKET_ERROR, "Decode Handle_GetMapObjects failed: arg 1 is not an int: %s", 
				v->TypeString());
			delete packet;
			return(false);
		}
		PyRepInteger *iii = (PyRepInteger *) v;
		wantRegions = (iii->value==0)?false:true;
	}
	if(packet->items.size() > 0) {
		PyRep *v = packet->items[0];
		if(!v->CheckType(PyRep::Integer)) {
			_log(NET__PACKET_ERROR, "Decode Handle_GetMapObjects failed: arg 0 is not an int: %s", 
				v->TypeString());
			delete packet;
			return(false);
		}
		PyRepInteger *iii = (PyRepInteger *) v;
		arg = iii->value;
	}
	if(packet->items.size() == 0 || packet->items.size() > 6) {
		_log(NET__PACKET_ERROR, "Decode Handle_GetMapObjects failed: tuple0 is the wrong size: expected 1-6, but got %d", packet->items.size());
		delete packet;
		return(false);
	}

	delete packet;
	
	PyRep *result = m_db.GetMapObjects(arg,
		wantRegions, wantConstellations, wantSystems, wantStations);
	
	return(result);
}

PyCallResult ConfigService::Handle_GetMultiInvTypesEx(PyCallArgs &call) {
	PyRep *result = NULL;

	//parse the PyRep to get the list of IDs to query.
	std::vector<uint32> ids;
	if(ParseIntegerList(call.tuple, "GetMultiInvTypesEx", ids)) {
		result = m_db.GetMultiInvTypesEx(ids);
	}
	
	if(result == NULL)
		result = new PyRepNone();
	
	return(result);
}


PyCallResult ConfigService::Handle_GetMapConnections(PyCallArgs &call) {
_log(SERVICE__ERROR, "Unhandled ConfigService::GetMapConnections");
	/*
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 1] String: 'GetMapConnections'
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2] Tuple: 6 elements
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2]   [ 0] Integer field: 10000016 (Lonetrek (region))
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2]   [ 1] Boolean field: false
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2]   [ 2] Boolean field: true
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2]   [ 3] Boolean field: false
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2]   [ 4] Integer field: 0
[PyRep]   Args:   [ 4]   [ 0]   [ 1]     [ 2]   [ 5] Integer field: 1

CREATE TABLE GetMapConnections (
  connectionType INT UNSIGNED NOT NULL,
  fromRegionID INT UNSIGNED NOT NULL,
  fromConstellationID INT UNSIGNED NULL,
  fromSolarSystemID INT UNSIGNED NULL,
  stargateID INT UNSIGNED NULL,
  celestialID INT UNSIGNED NULL,
  toSolarSystemID INT UNSIGNED NULL,
  toConstellationID INT UNSIGNED NULL,
  toRegionID INT UNSIGNED NOT NULL,
  PRIMARY KEY()
);

*/

	return(NULL);
}
PyCallResult ConfigService::Handle_GetStationSolarSystemsByOwner(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Bad arguments");
		return NULL;
	}

	// No idea what to return... yet...
	// Similar to GetCorpInfo(corpID) / corpSvc
	return m_db.GetStationSolarSystemsByOwner(arg.arg);
}

PyCallResult ConfigService::Handle_GetCelestialStatistic(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Bad arguments");
		return NULL;
	}

	return m_db.GetCelestialStatistic(arg.arg);
}































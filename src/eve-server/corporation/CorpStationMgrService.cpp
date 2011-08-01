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

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(CorpStationMgrService)

class CorpStationMgrIMBound
: public PyBoundObject
{
public:
	PyCallable_Make_Dispatcher(CorpStationMgrIMBound)
	
	CorpStationMgrIMBound(PyServiceMgr *mgr, CorporationDB& db, uint32 station_id)
	: PyBoundObject(mgr),
	  m_dispatch(new Dispatcher(this)),
	  m_db(db),
	  m_stationID(station_id)
	{
		_SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "CorpStationMgrIMBound";
		
		PyCallable_REG_CALL(CorpStationMgrIMBound, GetEveOwners)
		PyCallable_REG_CALL(CorpStationMgrIMBound, GetCorporateStationInfo)
		PyCallable_REG_CALL(CorpStationMgrIMBound, DoStandingCheckForStationService)
		PyCallable_REG_CALL(CorpStationMgrIMBound, GetPotentialHomeStations)
		PyCallable_REG_CALL(CorpStationMgrIMBound, SetHomeStation)
		PyCallable_REG_CALL(CorpStationMgrIMBound, SetCloneTypeID)
		PyCallable_REG_CALL(CorpStationMgrIMBound, GetQuoteForRentingAnOffice)
		PyCallable_REG_CALL(CorpStationMgrIMBound, RentOffice)
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetStationOffices)
		PyCallable_REG_CALL(CorpStationMgrIMBound, GetNumberOfUnrentedOffices)
		//testing
		PyCallable_REG_CALL(CorpStationMgrIMBound, GetCorporateStationOffice)
	}
	virtual ~CorpStationMgrIMBound() { delete m_dispatch; }
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(GetEveOwners)
	PyCallable_DECL_CALL(GetCorporateStationInfo)
	PyCallable_DECL_CALL(DoStandingCheckForStationService)
	PyCallable_DECL_CALL(GetPotentialHomeStations)
	PyCallable_DECL_CALL(SetHomeStation)
	PyCallable_DECL_CALL(SetCloneTypeID)
	PyCallable_DECL_CALL(GetQuoteForRentingAnOffice)
	PyCallable_DECL_CALL(RentOffice)
    PyCallable_DECL_CALL(GetStationOffices)
	PyCallable_DECL_CALL(GetNumberOfUnrentedOffices)
	//testing
	PyCallable_DECL_CALL(GetCorporateStationOffice)

protected:
    Dispatcher *const m_dispatch;

    CorporationDB& m_db;	//we do not own this
	const uint32 m_stationID;
};


CorpStationMgrService::CorpStationMgrService(PyServiceMgr *mgr)
: PyService(mgr, "corpStationMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	//PyCallable_REG_CALL(CorpStationMgrService, MachoBindObject)
}

CorpStationMgrService::~CorpStationMgrService() {
	delete m_dispatch;
}


PyBoundObject *CorpStationMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	if(!bind_args->IsInt()) {
		codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
		return NULL;
	}
	return new CorpStationMgrIMBound( m_manager, m_db, bind_args->AsInt()->value() );
}


PyResult CorpStationMgrIMBound::Handle_GetEveOwners(PyCallArgs &call)
{
	util_Rowset rs;
    rs.lines = new PyList;

	rs.header.push_back( "ownerID" );
	rs.header.push_back( "ownerName" );
	rs.header.push_back( "typeID" );

	PyList* chardata = new PyList;
	chardata->AddItemInt( 3004349 );
	chardata->AddItemString( "Carbircelle Hatiniestan" );
	chardata->AddItemInt( 1378 );
	rs.lines->AddItem( chardata );

	return rs.Encode();
}


PyResult CorpStationMgrIMBound::Handle_GetCorporateStationInfo(PyCallArgs &call) {
    /* returns:
     *  list(
     *      eveowners:
     *          rowset: ownerID,ownerName,typeID
     *      corporations:
     *          rowset:corporationID,corporationName,description,shares,graphicID,memberCount,ceoID,stationID,raceID,corporationType,creatorID,hasPlayerPersonnelManager,tickerName,sendCharTerminationMessage,shape1,shape2,shape3,color1,color2,color3,typeface,memberLimit,allowedMemberRaceIDs,url,taxRate,minimumJoinStanding,division1,division2,division3,division4,division5,division6,division7,allianceID
     *      offices (may be None):
     *          rowset: corporationID,itemID,officeFolderID
     *  )
     */

	PyList *l = new PyList();

	PyRep *tmp;

	tmp = m_db.ListStationOwners(m_stationID);
	if(tmp == NULL) {
		codelog(SERVICE__ERROR, "Failed to get owners.");
		return NULL;
	}
	l->AddItem( tmp );
	
	tmp = m_db.ListStationCorps(m_stationID);
	if(tmp == NULL) {
		codelog(SERVICE__ERROR, "Failed to get corps");
		return NULL;
	}
	l->AddItem( tmp );
	
	tmp = m_db.ListStationOffices(m_stationID);
	if(tmp == NULL) {
		codelog(SERVICE__ERROR, "Failed to get offices.");
		return NULL;
	}
	l->AddItem( tmp );

	return(l);

/*
#warning still using a hacked cache file here!

	std::string abs_fname = "../data/cache/fgAAAAAsLBAOY29ycFN0YXRpb25NZ3IERJiTAxAXR2V0Q29ycG9yYXRlU3RhdGlvbkluZm8.cache";
	
	PySubStream *ss = new PySubStream();

	if(!m_manager->GetCache()->LoadCachedFile(abs_fname.c_str(), "GetCorporateStationInfo", ss)) {
		_log(CLIENT__ERROR, "GetCorporateStationInfo Failed to load cache file '%s'", abs_fname.c_str());
		ss->decoded = new PyNone();
	} else {
		//hack:
		ss->length -= 82;
		uint8 *d = new uint8[ss->length];
		memcpy(d, ss->data+82, ss->length);
		delete ss->data;
		delete ss->decoded;
		ss->data = d;
		ss->decoded = NULL;
	}

	return(ss);*/
}


PyResult CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(PyCallArgs &call) {
	
	// takes an int (seen 512 and 1024 and 2048)
	//seems to return None, or throw an exception
	PyRep *result = new PyNone();

	return result;
}

PyResult CorpStationMgrIMBound::Handle_GetPotentialHomeStations(PyCallArgs &call) {
	PyRep *result = NULL;
	//returns a rowset: stationID, typeID

	_log(CLIENT__ERROR, "Hacking GetPotentialHomeStations");
	result = m_db.ListCorpStations(call.client->GetCorporationID());

	return result;
}

PyResult CorpStationMgrIMBound::Handle_SetHomeStation(PyCallArgs &call) {

	//this takes an integer: stationID
	//price is prompted for on the client side.

    sLog.Debug( "CorpStationMgrIMBound", "Called SetHomeStation stub." );

    return new PyNone;
}

PyResult CorpStationMgrIMBound::Handle_SetCloneTypeID(PyCallArgs &call) {

	//this takes an integer: cloneTypeID
	//price is prompted for on the client side.

	Call_SetCloneTypeID arg;
	if(!arg.Decode(&call.tuple)){
		sLog.Debug("CoporationMgrIMBound","Failed to determine Clone Type");
	}

	//Get cost of clone
	int cost = m_db.GetCloneTypeCostByID(arg.CloneTypeID);

	//Check if player has enough money
	if(call.client->GetBalance() > cost) {
		//subtract amount
		call.client->AddBalance(-cost);
	}

	//update type of clone
	m_db.ChangeCloneType(call.client->GetCharacterID(),arg.CloneTypeID);
	

    //sLog.Debug( "CorpStationMgrIMBound", "Called SetCloneTypeID stub." );

    return new PyNone;
}

PyResult CorpStationMgrIMBound::Handle_GetQuoteForRentingAnOffice(PyCallArgs &call) {
	// No incoming params...
	uint32 stationID = call.client->GetStationID();

	// Unless I produce an invalid ISK value (probably a NAN), this won't fail,
	// the dialog box will be displayed... have to make sure this doesn't fail
	return (new PyInt(m_db.GetQuoteForRentingAnOffice(stationID)));
}
PyResult CorpStationMgrIMBound::Handle_RentOffice(PyCallArgs &call) {
	// 1 param, corp rent price	//TODO: check against what we think it should cost.
	Call_SingleIntegerArg arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "Wrong incoming param in RentOffice");
		return NULL;
	}

	uint32 location = call.client->GetLocationID();

	// check if the corp has enough money
	double corpBalance = m_db.GetCorpBalance(call.client->GetCorporationID());
	if (corpBalance < arg.arg) {
		_log(SERVICE__ERROR, "%s: Corp doesn't have enough money to rent an office.", call.client->GetName());
		return (new PyInt(0));
	}


	// We should also check if the station has a free office atm...
	OfficeInfo oInfo(call.client->GetCorporationID(), call.client->GetStationID());
	oInfo.officeID = m_db.ReserveOffice(oInfo);
	// should we also put this into the entity table?


	if (!oInfo.officeID) {
		codelog(SERVICE__ERROR, "%s: Error at renting a new office", call.client->GetName());
		return new PyInt(0);
	}
	// Now we have the new office, let's update the officelist... if we have to...

		Notify_OnObjectPublicAttributesUpdated N_pau;
		OfficeAttributeUpdate change;

		// This way we can get the current bounded object's boundID
		N_pau.realRowCount = 2;
		N_pau.bindID = GetBindStr();
		N_pau.changePKIndexValue = oInfo.stationID;

		change.newOfficeFolderID = oInfo.officeFolderID;
		change.newOfficeID = oInfo.officeID;
		change.newStationID = oInfo.stationID;
		change.newTypeID = oInfo.typeID;
		N_pau.changes = change.Encode();
		
		PyTuple * res1 = N_pau.Encode(); // This is good enough as there are no old values atm
		// Who has to know about this public object's update?

		// This has to be sent to everyone in the station
		// For now, broadcast it
		m_manager->entity_list.Multicast("OnObjectPublicAttributesUpdated", "objectID", &res1, NOTIF_DEST__LOCATION, location, false);

		// This was the first broadcast...
	//}


	// remove the money
	m_db.AddBalanceToCorp(oInfo.corporationID, -double(arg.arg));
	corpBalance -= arg.arg;	// This is the new corp money. Do I have to make a casting here?
	// record the transaction
	m_db.GiveCash(oInfo.corporationID, RefType_officeRentalFee, oInfo.corporationID, oInfo.stationID, "unknown", call.client->GetAccountID(), accountCash, -double(arg.arg), corpBalance, "Renting office for 30 days"); 

	MulticastTarget mct;
	mct.corporations.insert(oInfo.corporationID);
	OnAccountChange ac;
	ac.accountKey = "cash";
	ac.ownerid = oInfo.corporationID; //call.client->GetCharacterID();
	ac.balance = corpBalance;
	PyTuple *res2 = ac.Encode();
	m_manager->entity_list.Multicast("OnAccountChange", "*corpid&corpAccountKey", &res2, mct);

	// This was the second notification


	// Now comes an OnItemChange notification, no need to do anything server-side
    util_Row Noic_row;

    Noic_row.header.push_back( "itemID" );
    Noic_row.header.push_back( "typeID" );
    Noic_row.header.push_back( "ownerID" );
    Noic_row.header.push_back( "locationID" );
    Noic_row.header.push_back( "flag" );
    Noic_row.header.push_back( "contraband" );
    Noic_row.header.push_back( "singleton" );
    Noic_row.header.push_back( "quantity" );
    Noic_row.header.push_back( "groupID" );
    Noic_row.header.push_back( "categoryID" );
    Noic_row.header.push_back( "customInfo" );

    Noic_row.line = new PyList;
    Noic_row.line->AddItemInt( oInfo.officeID );
    Noic_row.line->AddItemInt( 27 );
    Noic_row.line->AddItemInt( ac.ownerid );
    Noic_row.line->AddItemInt( oInfo.officeFolderID );
    Noic_row.line->AddItemInt( flagOfficeSlotFirst );
    Noic_row.line->AddItemInt( 0 );
    Noic_row.line->AddItemInt( 1 );
    Noic_row.line->AddItemInt( 1 );
    Noic_row.line->AddItemInt( EVEDB::invGroups::Station_Services );
    Noic_row.line->AddItemInt( EVEDB::invCategories::Station );
    Noic_row.line->AddItem( new PyNone );

	NotifyOnItemChange Noic;
    Noic.itemRow = Noic_row.Encode();
    Noic.changes[ixOwnerID] = new PyInt( 4 );

	PyTuple* res3 = Noic.Encode();
	// This is a possible broadcast-candidate
	m_manager->entity_list.Multicast("OnItemChange", "*stationid&corpid", &res3, NOTIF_DEST__LOCATION, location, false);

	// End of the third notification


	// Next is OnOfficeRentalChange, still no job on server-side

	Notify_OnOfficeRentalChanged N_oorc;
	N_oorc.ownerID = ac.ownerid;
	N_oorc.officeID = oInfo.officeID;
	N_oorc.officeFolderID = oInfo.officeFolderID;

	PyTuple * res4 = N_oorc.Encode(); // No need for fastencode, no null values
	// This is definately a broadcast-candidate
	m_manager->entity_list.Multicast("OnOfficeRentalChanged", "stationid", &res4, NOTIF_DEST__LOCATION, location);

	// End of the fourth notification


	// OnBillReceived, an essentially empty tuple, just to tell the client that there is something,
	// maybe for blinking purpose?

	Notify_OnBillReceived N_obr;
	PyTuple * res5 = N_obr.Encode();
	call.client->SendNotification("OnBillReceived", "*corpid&corprole", &res5, false);
	// Why do we create a bill, when the office is already paid? Maybe that's why it's empty...


	// End of the fifth notification


	// OnMessage notification, the LSC packet NotifyOnMessage can be used, along with the StoreNewEVEMail
	// Who to send notification? corpRoleJuniorAccountant and equiv? atm it's enough to send it to the renter
	// TODO: get the correct evemail content from somewhere
	// TODO: send it to every corp member who's affected by it. corpRoleAccountant, corpRoleJuniorAccountant or equiv
	m_manager->lsc_service->SendMail(
		m_db.GetStationCorporationCEO(oInfo.stationID), 
		call.client->GetCharacterID(), 
		"Bill issued", 
		"Bill issued for renting an office");

	// End of the sixth notification, so far so good...


	// One last thing: create that damn bill somewhere soon... an example would be nice...

	return (new PyInt(oInfo.officeID));
}

	/*
	Broadcast #1
	bcID: OnObjectPublicAttributesUpdated, idtype: objectID

	Unmarshaled rep:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: 4 elements
      [ 1]   [ 1]   [ 0] String: 'N=218038:21194'
      [ 1]   [ 1]   [ 1] Dictionary: 1 entries
      [ 1]   [ 1]   [ 1]   [ 0] Key: String: 'realRowCount'
      [ 1]   [ 1]   [ 1]   [ 0] Value: Integer field: 2
      [ 1]   [ 1]   [ 2] Tuple: Empty
      [ 1]   [ 1]   [ 3] Dictionary: 4 entries
      [ 1]   [ 1]   [ 3]   [ 0] Key: String: 'partial'
      [ 1]   [ 1]   [ 3]   [ 0] Value: List: 1 elements
      [ 1]   [ 1]   [ 3]   [ 0] Value:   [ 0] String: 'realRowCount'
      [ 1]   [ 1]   [ 3]   [ 1] Key: String: 'changePKIndexValue'
      [ 1]   [ 1]   [ 3]   [ 1] Value: Integer field: 60008743
      [ 1]   [ 1]   [ 3]   [ 2] Key: String: 'notificationParams'
      [ 1]   [ 1]   [ 3]   [ 2] Value: Dictionary: 0 entries
      [ 1]   [ 1]   [ 3]   [ 3] Key: String: 'change'
      [ 1]   [ 1]   [ 3]   [ 3] Value: Dictionary: 4 entries
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 0] Key: String: 'typeID'
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 0] Value: Tuple: 2 elements
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 0] Value:   [ 0] (None)
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 0] Value:   [ 1] Integer field: 1932
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 1] Key: String: 'Invalid String Table Item 155'
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 1] Value: Tuple: 2 elements
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 1] Value:   [ 0] (None)
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 1] Value:   [ 1] Integer field: 60008743
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 2] Key: String: 'officeFolderID'
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 2] Value: Tuple: 2 elements
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 2] Value:   [ 0] (None)
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 2] Value:   [ 1] Integer field: 66008744
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 3] Key: String: 'officeID'
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 3] Value: Tuple: 2 elements
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 3] Value:   [ 0] (None)
      [ 1]   [ 1]   [ 3]   [ 3] Value:   [ 3] Value:   [ 1] Integer field: 170947028
	  
	  ScatterEvent( OnObjectPublicAttributesUpdated ,*args= ('N=218038:21194', {'realRowCount': 2}, (), {'partial': ['realRowCount'], 'notificationParams': {}, 'change': {'typeID': (None, 1932), 'stationID': (None, 60008743), 'officeFolderID': (None, 66008744)
	*/
	/*
	Broadcast #2
	bcid: OnAccountChange, idtype: *corpid&corpAccountKey

	Unmarshaled rep:
	Tuple: 2 elements
	  [ 0] Integer field: 0
	  [ 1] Tuple: 2 elements
	  [ 1]   [ 0] Integer field: 1
	  [ 1]   [ 1] Tuple: 3 elements
	  [ 1]   [ 1]   [ 0] String: 'cash'
	  [ 1]   [ 1]   [ 1] Integer field: 5555555555
	  [ 1]   [ 1]   [ 2] Real Field: 2990000.000000

		ScatterEvent( OnAccountChange ,*args= ('cash', 5555555555, 2990000.0) ,**kw= {} )
	*/
	/*
	Broadcast #3
	bcID: OnItemChange, idtype: *stationid&corpid

	Unmarshaled rep:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: 2 elements
      [ 1]   [ 1]   [ 0] Object:
      [ 1]   [ 1]   [ 0]   Type: util.Row
      [ 1]   [ 1]   [ 0]   Args: Dictionary: 2 entries
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Key: String: 'header'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value: List: 11 elements
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 0] String: 'itemID'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 1] String: 'typeID'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 2] String: 'ownerID'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 3] String: 'locationID'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 4] String: 'flag'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 5] String: 'contraband'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 6] String: 'singleton'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 7] String: 'quantity'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 8] String: 'groupID'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [ 9] String: 'categoryID'
      [ 1]   [ 1]   [ 0]   Args:   [ 0] Value:   [10] String: 'customInfo'
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Key: String: 'line'
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value: List: 11 elements
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 0] Integer field: 170947000	<- officeID?
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 1] Integer field: 27
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 2] Integer field: 5555555555	<- corporation?
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 3] Integer field: 66008744	<- officeFolderID
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 4] Integer field: 84
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 5] Integer field: 0
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 6] Integer field: 1
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 7] Integer field: 1
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 8] Integer field: 16
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [ 9] Integer field: 3
      [ 1]   [ 1]   [ 0]   Args:   [ 1] Value:   [10] (None)
      [ 1]   [ 1]   [ 1] Dictionary: 1 entries
      [ 1]   [ 1]   [ 1]   [ 0] Key: Integer field: 2
      [ 1]   [ 1]   [ 1]   [ 0] Value: Integer field: 4

		ScatterEvent( OnItemChange ,*args= (Row(itemID: 170947028,typeID: 27,ownerID: 497751594,locationID: 66008744,flag: 84,contraband: 0,singleton: 1,quantity: 1,groupID: 16,categoryID: 3,customInfo: None), {2: 4}) ,**kw= {} )
	*/
	/*
	Broadcast #4
	bcID: OnOfficeRentalChanged, idtype: stationid

	Unmarshaled rep:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: 3 elements
      [ 1]   [ 1]   [ 0] Integer field: 5555555555
      [ 1]   [ 1]   [ 1] Integer field: 170947000
      [ 1]   [ 1]   [ 2] Integer field: 66008744

	  ScatterEvent( OnOfficeRentalChanged ,*args= (5555555555, 170947000, 66008744) ,**kw= {} )
	*/
	/*
	Broadcast #5
	bcID: OnBillReceived, idtype: *corpid&corprole

	Unmarshaled rep:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: Empty

	  ScatterEvent( OnBillReceived ,*args= () ,**kw= {} )
	*/
	/*
	Broadcast #6
	bcID: OnMessage, idType: multicastID

	Unmarshaled rep:
    Tuple: 2 elements
      [ 0] Integer field: 0
      [ 1] Tuple: 2 elements
      [ 1]   [ 0] Integer field: 1
      [ 1]   [ 1] Tuple: 5 elements
      [ 1]   [ 1]   [ 0] List: 2 elements
      [ 1]   [ 1]   [ 0]   [ 0] Integer field: 22222222	<- reciever #1, not current char
      [ 1]   [ 1]   [ 0]   [ 1] Integer field: 33333333	<- reciever #2, not current char
      [ 1]   [ 1]   [ 1] Integer field: 444444444			<- messageID
      [ 1]   [ 1]   [ 2] Integer field: 1000090				<- sender, corpID / station owner
      [ 1]   [ 1]   [ 3] String: 'Bill issued'				<- subject
      [ 1]   [ 1]   [ 4] Integer field: 128291836200000000	<- current time

	  ScatterEvent( OnMessage ,*args= ([22222222, 33333333], 444444444, 1000090, 'Bill issued', 128291836200000000L) ,**kw= {} )
	*/

PyResult CorpStationMgrIMBound::Handle_GetStationOffices( PyCallArgs& call )
{
	//Hack: Just passing the client an empty PyList to stop it throwing an exception.
	//TODO: Fid out what needs to be in the PyList and when to send it.
    sLog.Debug( "CorpStationMgrIMBound", "Called GetStationOffices stub." );
	/*
	[PySubStream 99 bytes]
        [PyObjectData Name: objectCaching.CachedMethodCallResult]
          [PyTuple 3 items]
            [PyDict 1 kvp]
              [PyString "versionCheck"]
              [PyTuple 3 items]
                [PyString "always"]
                [PyNone]
                [PyNone]
            [PySubStream 6 bytes]
              [PyList 0 items]
            [PyList 2 items] //Cache Information
              [PyIntegerVar 129533580031608440] //Timestamp (fileTime)
              [PyInt 52428965] //Hash??
			  */
	PyTuple * arg_tuple = new PyTuple(3);



	PyDict* itr_1 = new PyDict();
	itr_1->SetItem("versionCheck", new_tuple("always", new PyNone, new PyNone));
	
	arg_tuple->SetItem(0, itr_1);
	arg_tuple->SetItem(1, new PySubStream( new PyList() ) );
	arg_tuple->SetItem(2, new_tuple(129533580031608440, 52428965));
    return new PyObject( "objectCaching.CachedMethodCallResult", arg_tuple );
}

PyResult CorpStationMgrIMBound::Handle_GetNumberOfUnrentedOffices( PyCallArgs &call ) 
{
	//Extremely hackish and temporary.  Just getting some number working to pass to the client.
	//TODO: add handler that queries the data from the StationType struct.  Not exactly sure how to do this,
	//		but will involve call.client->GetStationID as the arguments to StationType.officeSlots() (hopefully)
	const int office_hack = 1;

	return new PyInt(office_hack);
}

PyResult CorpStationMgrIMBound::Handle_GetCorporateStationOffice(PyCallArgs &call)
{
	sLog.Debug("Server","Called GetCorporateStationOffice Stub");

	return NULL;
}













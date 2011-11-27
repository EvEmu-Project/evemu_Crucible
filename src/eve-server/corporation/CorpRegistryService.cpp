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
    Author:     Zhur
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(CorpRegistryService)

class CorpRegistryBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(CorpRegistryBound)

    CorpRegistryBound(PyServiceMgr *mgr, CorporationDB& db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "CorpRegistryBound";

        //PyCall_REG_CALL
        PyCallable_REG_CALL(CorpRegistryBound, GetEveOwners)
        PyCallable_REG_CALL(CorpRegistryBound, GetCorporation)
        PyCallable_REG_CALL(CorpRegistryBound, GetCorporations)
        PyCallable_REG_CALL(CorpRegistryBound, GetInfoWindowDataForChar)
        PyCallable_REG_CALL(CorpRegistryBound, GetLockedItemLocations)
        PyCallable_REG_CALL(CorpRegistryBound, AddCorporation)

        PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedTickerNames)
        PyCallable_REG_CALL(CorpRegistryBound, GetOffices)
        PyCallable_REG_CALL(CorpRegistryBound, GetStations)
        PyCallable_REG_CALL(CorpRegistryBound, GetMyApplications)
        PyCallable_REG_CALL(CorpRegistryBound, InsertApplication)
        PyCallable_REG_CALL(CorpRegistryBound, GetApplications)
        PyCallable_REG_CALL(CorpRegistryBound, UpdateApplicationOffer)
        PyCallable_REG_CALL(CorpRegistryBound, DeleteApplication)
        PyCallable_REG_CALL(CorpRegistryBound, UpdateApplication)
        PyCallable_REG_CALL(CorpRegistryBound, UpdateDivisionNames)
        PyCallable_REG_CALL(CorpRegistryBound, UpdateCorporation)
        PyCallable_REG_CALL(CorpRegistryBound, UpdateLogo)

        // STUBBS
        PyCallable_REG_CALL(CorpRegistryBound, GetMember)
        PyCallable_REG_CALL(CorpRegistryBound, GetMembers)

        
    }
    virtual ~CorpRegistryBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetEveOwners)
    PyCallable_DECL_CALL(GetCorporation)
    PyCallable_DECL_CALL(GetCorporations)
    PyCallable_DECL_CALL(GetInfoWindowDataForChar)
    PyCallable_DECL_CALL(GetLockedItemLocations)
    PyCallable_DECL_CALL(AddCorporation)
    //PyCallable_DECL_CALL(GetMember)
    PyCallable_DECL_CALL(GetSuggestedTickerNames)
    PyCallable_DECL_CALL(GetOffices)
    PyCallable_DECL_CALL(GetStations)
    PyCallable_DECL_CALL(GetMyApplications)
    PyCallable_DECL_CALL(InsertApplication)
    PyCallable_DECL_CALL(GetApplications)
    PyCallable_DECL_CALL(UpdateApplicationOffer)
    PyCallable_DECL_CALL(DeleteApplication)
    PyCallable_DECL_CALL(UpdateApplication)
    PyCallable_DECL_CALL(UpdateDivisionNames)
    PyCallable_DECL_CALL(UpdateCorporation)
    PyCallable_DECL_CALL(UpdateLogo)

    // STUBBS
    PyCallable_DECL_CALL(GetMember)
    PyCallable_DECL_CALL(GetMembers)

    

protected:
    bool JoinCorporation(Client *who, uint32 newCorpID, const CorpMemberInfo &roles);
    static void FillOCApplicationChange(Notify_OnCorporationApplicationChanged & OCAC, const ApplicationInfo & Old, const ApplicationInfo & New);

    CorporationDB& m_db;

    Dispatcher *const m_dispatch;
};

class SparseCorpOfficeListBound
: public PyBoundObject
{
public:
    // or CorpRegistryBound?
    PyCallable_Make_Dispatcher(SparseCorpOfficeListBound)

    SparseCorpOfficeListBound(PyServiceMgr *mgr, CorporationDB& db)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)),
      m_db(db)
    {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(SparseCorpOfficeListBound, Fetch)
        //PyCallable_REG_CALL(SparseCorpOfficeListBound, FetchByKey)
        //PyCallable_REG_CALL(SparseCorpOfficeListBound, GetByKey)
    }
    virtual ~SparseCorpOfficeListBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(Fetch) //(startPos, fetchSize)
    //PyCallable_DECL_CALL(FetchByKey) //([keys])
    //PyCallable_DECL_CALL(GetByKey) //(key)


protected:
    Dispatcher *const m_dispatch;

    CorporationDB& m_db;
};



CorpRegistryService::CorpRegistryService(PyServiceMgr *mgr)
: PyService(mgr, "corpRegistry"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(CorpRegistryService, GetEveOwners)
}

CorpRegistryService::~CorpRegistryService() {
    delete m_dispatch;
}



PyBoundObject* CorpRegistryService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "CorpRegistryService bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, "    " );

    return new CorpRegistryBound( m_manager, m_db );
}


PyResult CorpRegistryBound::Handle_GetEveOwners(PyCallArgs &call) {
    return (m_db.GetEveOwners());
}

PyResult CorpRegistryBound::Handle_GetInfoWindowDataForChar( PyCallArgs& call )
{
    //takes characterID

    sLog.Debug( "CorpRegistryBound", "Called GetInfoWindowDataForChar stub." );

    return new PyNone;
}

PyResult CorpRegistryBound::Handle_GetLockedItemLocations( PyCallArgs& call )
{
    //takes characterID

    sLog.Debug( "CorpRegistryBound", "Called GetLockedItemLocations stub." );

    //this returns an empty list for me on live.
    return new PyList;
}

PyResult CorpRegistryBound::Handle_GetCorporation(PyCallArgs &call) {
    return(m_db.GetCorporation(call.client->GetCorporationID()));
}
PyResult CorpRegistryBound::Handle_GetCorporations(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Bad incoming params.");
        return NULL;
    }

    return(m_db.GetCorporation(arg.arg));
}


PyResult CorpRegistryBound::Handle_AddCorporation(PyCallArgs &call) {
    Call_AddCorporation args;

    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return(new PyInt(0));
    }

    //first make sure the char can even afford it.
    uint32 corp_costu;
    if(!m_db.GetConstant("corporationStartupCost", corp_costu)) {
        codelog(SERVICE__ERROR, "%s: Failed to determine corporation costs.", call.client->GetName());
        return(new PyInt(0));
    }
    int32 corp_cost = corp_costu;

    if(call.client->GetBalance() < double(corp_cost)) {
        _log(SERVICE__ERROR, "%s: Cannot afford corporation startup costs!", call.client->GetName());
        return(new PyInt(0));
    }

    // Register new corp
    uint32 corpID;
    if (!m_db.AddCorporation(args, call.client->GetCharacterID(), call.client->GetStationID(), corpID)) {
        codelog(SERVICE__ERROR, "New corporation creation failed...");
        return (new PyInt(0));
    }
    //adding a corporation might affect eveStaticOwners, so we gotta invalidate the cache...
    PyString* cache_name = new PyString( "config.StaticOwners" );
    m_manager->cache_service->InvalidateCache( cache_name );
    PySafeDecRef( cache_name );

    //take the money out of their wallet (sends wallet blink event)
    // The amount has to be double!!!
    if(!call.client->AddBalance(double(-corp_cost))) {
        codelog(SERVICE__ERROR, "%s: Failed to take money for corp startup!", call.client->GetName());
        //TODO: do something? I don't wanna deal with deleting the corp right now...
    }

    //record the transaction in their journal.
    std::string reason = "Creating new corporation: ";
    reason += args.corpName;
    if(!m_db.GiveCash(
        call.client->GetCharacterID(),
        RefType_corpRegFee,
        call.client->GetCharacterID(),  //eve system
        1,
        "1",
        call.client->GetAccountID(),
        accountCash,
        -corp_cost,
        call.client->GetBalance(),
        reason.c_str()
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record corp creation transaction.");
        //no good reason to return... the money has actually been moved.
    }

    uint32 location = call.client->GetLocationID();

    // Here we send a notification about creating a new corporation...
    Notify_OnCorporaionChanged cc;
    cc.corpID = corpID;
    if (!m_db.CreateCorporationCreatePacket(cc, call.client->GetCorporationID(), corpID)) {
        codelog(SERVICE__ERROR, "Failed to create OnCorpChanged notification stream.");
        // This is a big problem, because this way we won't be able to see the difference...
        call.client->SendErrorMsg("Unable to notify about corp creation. Try logging in again.");
        return (new PyInt(0));
    }
    PyTuple* a1 = cc.Encode();
    PyTuple* a2 = cc.Encode();
    m_manager->entity_list.Multicast("OnCorporationChanged", "clientID", &a1, NOTIF_DEST__LOCATION, location);
    m_manager->entity_list.Multicast("OnCorporationChanged", "stationid", &a2, NOTIF_DEST__LOCATION, location);

    // Set char's roles in corp
    CorpMemberInfo roles;
    roles.corpRole = corpRoleAll;
    roles.rolesAtAll = corpRoleAll;
    roles.rolesAtBase = corpRoleAll;
    roles.rolesAtHQ = corpRoleAll;
    roles.rolesAtOther = corpRoleAll;

    //loads up roles and alters session.
    if(!JoinCorporation(call.client, corpID, roles)) {
        codelog(CLIENT__ERROR, "Failed to force char '%s' to join new corporation %u. This will be interesting.", call.client->GetName(), corpID);
        return (new PyInt(0));
    }

    return (new PyInt(corpID));
}

bool CorpRegistryBound::JoinCorporation(Client *who, uint32 newCorpID, const CorpMemberInfo &roles) {

    // Change current user's corporation in DB (incl. member counts)
    if(!m_db.JoinCorporation(
            who->GetCharacterID(),
            newCorpID,
            who->GetCorporationID(),
            roles)
    ) {
        codelog(CLIENT__ERROR, "Failed to update database for corporation join of '%s' to corp %u", who->GetName(), newCorpID);
        who->SendErrorMsg("Unable to notify about corp creation. Try logging in again.");
        return false;
    }

    who->JoinCorporationUpdate(newCorpID);
    return true;
}

/* this is a stubb */
PyResult CorpRegistryBound::Handle_GetMember(PyCallArgs &call) {
//    uint32 item = 0;
    //item += 1;
    return NULL;
}

/*
PyClass
  PyString:"macho.CallRsp"
  PyTuple:6
    itr[0]:0x7
    itr[1]:PyClass
      PyString:"macho.MachoAddress"
      PyTuple:4
        itr[0]:0x1
        itr[1]:0xACA03
        itr[2]:None
        itr[3]:None
    itr[2]:PyClass
      PyString:"macho.MachoAddress"
      PyTuple:4
        itr[0]:0x2
        itr[1]:0x18BD39F1529E
        itr[2]:0x41
        itr[3]:None
    itr[3]:0x77F625
    itr[4]:PyTuple:1
      itr[0]:PySubStream
        PyClass
          PyString:"util.SparseRowset"
          PyTuple:3
            itr[0]:PyList:19
              itr:"characterID"
              itr:"corporationID"
              itr:"divisionID"
              itr:"squadronID"
              itr:"title"
              itr:"roles"
              itr:"grantableRoles"
              itr:"startDateTime"
              itr:"baseID"
              itr:"rolesAtHQ"
              itr:"grantableRolesAtHQ"
              itr:"rolesAtBase"
              itr:"grantableRolesAtBase"
              itr:"rolesAtOther"
              itr:"grantableRolesAtOther"
              itr:"titleMask"
              itr:"accountKey"
              itr:"rowDate"
              itr:"blockRoles"
            itr[1]:PySubStruct
              PySubStream
                  PyTuple:3
                    itr[0]:"N=707075:302"
                    itr[1]:PyDict:1
                      dict["realRowCount"]=0x1E
                    itr[2]:0x1CC2383E961BFA8
            itr[2]:0x1E
    itr[5]:PyDict:1
      dict["OID+"]=PyDict:1
        dict["N=707075:302"]=0x1CC2383E961BFA8
*/
PyResult CorpRegistryBound::Handle_GetMembers(PyCallArgs &call) {
    return NULL;
}

PyResult CorpRegistryBound::Handle_GetSuggestedTickerNames(PyCallArgs &call) {
    Call_SingleStringArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    PyList * result = new PyList;
    Item_GetSuggestedTickerNames sTN;
    sTN.tN = "";
    uint32 cnLen = arg.arg.length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0;i<cnLen;i++) {
        if (arg.arg[i] >= 'A' && arg.arg[i] <= 'Z') {
            sTN.tN += arg.arg[i];
        }
    }
    result->AddItem( sTN.Encode() );

    return result;
}

PyResult CorpRegistryBound::Handle_GetStations(PyCallArgs &call) {
    // No param

    // Need to fetch stations of current corporation...
    return m_db.GetStations(call.client->GetCorporationID());
}

PyResult CorpRegistryBound::Handle_GetOffices(PyCallArgs &call) {

    _log(CLIENT__MESSAGE, "CorpRegistryBound bind request for OfficeList of corp %u", call.client->GetCorporationID());

    // First create the boundable object

    PyBoundObject *bObj;
    bObj = new SparseCorpOfficeListBound(m_manager, m_db);
    if(bObj == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to create bound object for:", call.client->GetName());
        return NULL;
    }

    CorpOfficeSparseRowset ret;

    //now we register
    PyDict *dict = new PyDict();

    // First time we only need the number of rows, not the data itself
    // Data will be fetched from the SparseRowset
    uint32 officeN = m_db.GetOffices(call.client->GetCorporationID());

    // No idea what this is
    dict->SetItemString("realRowCount", new PyInt(officeN));
    // But this one holds the real row number
    ret.officeNumber = officeN;

    ret.bindedObject = m_manager->BindObject(call.client, bObj, &dict);

    //call.client->temp_hack_officeLists[call.client->GetCorporationID()] = bindID; //m_manager->FindBoundObject(bObj);

    PyObject * res = ret.Encode();
    return res;
}

PyResult SparseCorpOfficeListBound::Handle_Fetch(PyCallArgs &call) {
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    return m_db.Fetch(call.client->GetCorporationID(), args.arg1, args.arg2);
}

PyResult CorpRegistryBound::Handle_GetMyApplications(PyCallArgs &call) {
    /// We have a dict
    /// With an STI and an integer
    /// Ignore them for now
    return m_db.GetMyApplications(call.client->GetCharacterID());
}

PyResult CorpRegistryBound::Handle_InsertApplication(PyCallArgs &call) {
    /** Incoming:
     *  Integer: 777777777 <- corp id
     *  String: "Ignore me" <- text that was entered into the box
     */

    Call_InsertApplication res;
    if (!res.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    /// Insert query into the db
    ApplicationInfo aInfo;
    aInfo.charID = call.client->GetCharacterID();
    aInfo.corpID = res.corpID;
    aInfo.appText = res.message;
    if (!m_db.InsertApplication(aInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to insert application request", call.client->GetName());
        return NULL;
    }

    /// BroadcastStuff::Notify( OnCorporationApplicationChanged ,...)
    Notify_OnCorporationApplicationChanged OCAC;
    ApplicationInfo oldInfo(false);
    FillOCApplicationChange(OCAC, oldInfo, aInfo);
    OCAC.corpID = res.corpID;
    OCAC.charID = aInfo.charID;

    PyTuple* notif = OCAC.Encode();
    // Who needs to know this?
    // Everyone who's in that corporation, right?

    MulticastTarget mct;
    mct.characters.insert(OCAC.charID);
    mct.corporations.insert(OCAC.corpID);
    m_manager->entity_list.Multicast(
        "OnCorporationApplicationChanged",
        "clientID", &notif, mct);


    /// need to find out what happens on the other side
    /// if there's anything at all on the other side

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string
        subject = std::string("New application from ") + call.client->GetName(),
        body = res.message;
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(res.corpID));
    m_manager->lsc_service->SendMail(call.client->GetCharacterID(), recipients, subject, body);

    /// Reply: ~\x00\x00\x00\x00\x01
    return NULL;
}

void CorpRegistryBound::FillOCApplicationChange(Notify_OnCorporationApplicationChanged & OCAC, const ApplicationInfo & Old, const ApplicationInfo & New) {
    if (Old.valid) {
        OCAC.applicationDateTimeOld = new PyLong(Old.appTime);
        OCAC.applicationTextOld = new PyString(Old.appText);
        OCAC.characterIDOld = new PyInt(Old.charID);
        OCAC.corporationIDOld = new PyInt(Old.corpID);
        OCAC.deletedOld = new PyInt(Old.deleted);
        OCAC.grantableRolesOld = new PyLong(Old.grantRole);
        if (Old.lastCID) {
            OCAC.lastCorpUpdaterIDOld = new PyInt(Old.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDOld = new PyNone();
        }
        OCAC.rolesOld = new PyLong(Old.role);
        OCAC.statusOld = new PyInt(Old.status);
    } else {
        OCAC.applicationDateTimeOld = new PyNone();
        OCAC.applicationTextOld = new PyNone();
        OCAC.characterIDOld = new PyNone();
        OCAC.corporationIDOld = new PyNone();
        OCAC.deletedOld = new PyNone();
        OCAC.grantableRolesOld = new PyNone();
        OCAC.lastCorpUpdaterIDOld = new PyNone();
        OCAC.rolesOld = new PyNone();
        OCAC.statusOld = new PyNone();
    }

    if (New.valid) {
        OCAC.applicationDateTimeNew = new PyLong(New.appTime);
        OCAC.applicationTextNew = new PyString(New.appText);
        OCAC.characterIDNew = new PyInt(New.charID);
        OCAC.corporationIDNew = new PyInt(New.corpID);
        OCAC.deletedNew = new PyInt(New.deleted);
        OCAC.grantableRolesNew = new PyLong(New.grantRole);
        if (New.lastCID) {
            OCAC.lastCorpUpdaterIDNew = new PyInt(New.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDNew = new PyNone();
        }
        OCAC.rolesNew = new PyLong(New.role);
        OCAC.statusNew = new PyInt(New.status);
    } else {
        OCAC.applicationDateTimeNew = new PyNone();
        OCAC.applicationTextNew = new PyNone();
        OCAC.characterIDNew = new PyNone();
        OCAC.corporationIDNew = new PyNone();
        OCAC.deletedNew = new PyNone();
        OCAC.grantableRolesNew = new PyNone();
        OCAC.lastCorpUpdaterIDNew = new PyNone();
        OCAC.rolesNew = new PyNone();
        OCAC.statusNew = new PyNone();
    }
}

PyResult CorpRegistryBound::Handle_GetApplications(PyCallArgs &call) {
    /** Incoming:
     *  Nothing at all
     */
    return m_db.GetApplications(call.client->GetCorporationID());
}

/** AppInfo:
 *  status / corp side / user side
 *    0        new         applied
 *    1        update      reneg
 *    2        accepted    accepted
 *    4        error       reject
 *    6        offer       offer
 */
typedef enum {  //from eveConstants
    crpApplicationAppliedByCharacter = 0,
    crpApplicationRenegotiatedByCharacter = 1,
    crpApplicationAcceptedByCharacter = 2,
    crpApplicationRejectedByCharacter = 3,
    crpApplicationRejectedByCorporation = 4,
    crpApplicationRenegotiatedByCorporation = 5,
    crpApplicationAcceptedByCorporation = 6
} CorpApplicationStatus;
PyResult CorpRegistryBound::Handle_UpdateApplicationOffer(PyCallArgs &call) {
    /** Incoming:
     *  Tuple
     *   - int 140000017    <- this is the charID, whose app should be handled
     *   - string message
     *   - int decision
     *      4: rejection
     *      6: acception
     *   - (none), so far
     */

    Call_UpdateApplicationOffer args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    // OnCorporationApplicationChanged event, probably be good to make it two (or more) times, independently, depending on update type

    Notify_OnCorporationApplicationChanged OCAC;
    PyTuple * answer;


    switch (args.newStatus) {
    case crpApplicationRejectedByCorporation:
    {
        ApplicationInfo newInfo(true);
        ApplicationInfo oldInfo(true);
        ApplicationInfo invalidInfo(false);
        OCAC.charID = args.charID;
        OCAC.corpID = call.client->GetCorporationID();
        if(!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
            codelog(SERVICE__ERROR, "%s: Failed to query application for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
            return NULL;
        }
        newInfo = oldInfo;
        newInfo.status = crpApplicationRejectedByCorporation;
        newInfo.lastCID = call.client->GetCharacterID();

        if(!m_db.UpdateApplication(newInfo)) {
            codelog(SERVICE__ERROR, "%s: Failed to update application", call.client->GetName());
            return NULL;
        }

        FillOCApplicationChange(OCAC, oldInfo, newInfo);
        answer = OCAC.Encode();
        m_manager->entity_list.Unicast(OCAC.charID,
            "OnCorporationApplicationChanged",
            "*corpid&corprole", &answer);

        FillOCApplicationChange(OCAC, oldInfo, invalidInfo);
        answer = OCAC.Encode();
        // Maybe this will remove the app from the corp
        m_manager->entity_list.Multicast(
            "OnCorporationApplicationChanged",
            "*corpid&corprole", &answer,
            NOTIF_DEST__CORPORATION, OCAC.corpID);
        break;
    }
    case crpApplicationAcceptedByCorporation: /// accepted
    {
        // the acceptor corporation MUST have free space!!


        /// OnCorporationApplicationChanged
        ApplicationInfo newInfo(true);
        ApplicationInfo oldInfo(true);
        OCAC.charID = args.charID;
        OCAC.corpID = call.client->GetCorporationID();
        if(!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
            codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
            return NULL;
        }
        newInfo = oldInfo;
        newInfo.status = crpApplicationAcceptedByCharacter;
        newInfo.lastCID = call.client->GetCharacterID();

        if(!m_db.UpdateApplication(newInfo)) {
            codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
            return NULL;
        }

        FillOCApplicationChange(OCAC, oldInfo, newInfo);

        answer = OCAC.Encode();
        MulticastTarget mct;
        mct.characters.insert(OCAC.charID);
        mct.corporations.insert(OCAC.corpID);
        m_manager->entity_list.Multicast(
            "OnCorporationApplicationChanged",
            "*corpid&corprole", &answer, mct);

//TODO: should probably put this into a function, since there may be other
//places (gm commands at a minimum) where we want to change corp.
/** TODO: Update employment history object, if present
 */
        // OnObjectPublicAttributesUpdated event
        Notify_OnObjectPublicAttributesUpdated N_pau;
        MemberAttributeUpdate change;

        N_pau.realRowCount = 4;
        N_pau.bindID = GetBindStr();
        N_pau.changePKIndexValue = args.charID;

        if (!m_db.CreateMemberAttributeUpdate(change, oldInfo.corpID, args.charID)) {
            codelog(SERVICE__ERROR, "Couldn't get data from the character. Sorry.");
            return NULL;
        }

        N_pau.changes = change.Encode();

        answer = N_pau.Encode();
        m_manager->entity_list.Multicast(
            "OnObjectPublicAttributesUpdated",
            "objectID", &answer,
            NOTIF_DEST__CORPORATION, OCAC.corpID);

        // OnCorporationMemberChanged event
        Notify_OnCorpMemberChange ocmc;

        ocmc.charID = args.charID;
        ocmc.newCorpID = change.corporationIDNew->AsInt()->value();
        ocmc.oldCorpID = change.corporationIDOld->AsInt()->value();
        ocmc.newDate = OCAC.applicationDateTimeNew->AsInt()->value();
        ocmc.oldDate = OCAC.applicationDateTimeOld->AsInt()->value();


        // both corporations' members will be notified about the change
        MulticastTarget both_corps;
        both_corps.corporations.insert(ocmc.newCorpID);
        both_corps.corporations.insert(ocmc.oldCorpID);
        answer = ocmc.Encode();
        m_manager->entity_list.Multicast(
            "OnCorporationMemberChanged", "corpid",
            &answer, both_corps);


        // OnCorporationMemberChanged event again
        // this is the same as the first one
        // and goes twice
        // HAHA: just because they do it doesn't mean we need to...
        Notify_IntRaw notif;
        notif.key = args.charID;
        notif.data = change.Encode();

        answer = notif.Encode();
        m_manager->entity_list.Multicast(
            "OnCorporationMemberChanged", "corpid",
            &answer, both_corps);

        answer = notif.Encode();
        m_manager->entity_list.Multicast(
            "OnCorporationMemberChanged", "corpid",
            &answer, both_corps);

        //NOTE: this really should happen sooner, in case it fails.
        if(!m_db.JoinCorporation(args.charID, ocmc.newCorpID, ocmc.oldCorpID, CorpMemberInfo())) {
            codelog(SERVICE__ERROR, "%s: Failed to record corp join for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
            return NULL;
        }

        Client *recruit = m_manager->entity_list.FindCharacter(ocmc.charID);
        if(recruit != NULL) {
            recruit->JoinCorporationUpdate(ocmc.newCorpID);
        }

        break;
        }
    }
    return new PyInt(1);
}

PyResult CorpRegistryBound::Handle_DeleteApplication(PyCallArgs & call) {
    /** Incoming:
     *  tuple of 2 elements, corpID and charID
     */
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return(new PyInt(0));
    }

    Notify_OnCorporationApplicationChanged OCAC;

    ApplicationInfo newInfo(false);
    ApplicationInfo oldInfo(true);
    OCAC.corpID = args.arg1;
    OCAC.charID = args.arg2;
    if(!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return(new PyInt(0));
    }

    FillOCApplicationChange(OCAC, oldInfo, newInfo);

    if(!m_db.DeleteApplication(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to delete application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return(new PyInt(0));
    }

    PyTuple * answer = OCAC.Encode();
    MulticastTarget mct;
    mct.characters.insert(OCAC.charID);
    mct.corporations.insert(OCAC.corpID);
    m_manager->entity_list.Multicast(
        "OnCorporationApplicationChanged",
        "*corpid&corprole", &answer, mct);

    return new PyInt(1);
}

PyResult CorpRegistryBound::Handle_UpdateApplication(PyCallArgs &call) {
    Call_UpdateApplication args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    ApplicationInfo oldInfo(true);
    ApplicationInfo newInfo;
    Notify_OnCorporationApplicationChanged OCAC;
    OCAC.charID = call.client->GetCharacterID();
    OCAC.corpID = args.corpID;
    if(!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return(new PyInt(0));
    }
    newInfo = oldInfo;
    newInfo.appText = args.message;
    newInfo.status = args.status;

    if(!m_db.UpdateApplication(newInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to update application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return(new PyInt(0));
    }

    FillOCApplicationChange(OCAC, oldInfo, newInfo);

    PyTuple* notif = OCAC.Encode();
    m_manager->entity_list.Unicast(OCAC.charID,
        "OnCorporationApplicationChanged",
        "clientID", &notif);

    FillOCApplicationChange(OCAC, ApplicationInfo(false), newInfo);
    notif = OCAC.Encode();
    m_manager->entity_list.Multicast(
        "OnCorporationApplicationChanged",
        "clientID", &notif,
        NOTIF_DEST__CORPORATION, OCAC.corpID);

    return NULL;
}

PyResult CorpRegistryBound::Handle_UpdateDivisionNames(PyCallArgs &call) {
    Call_UpdateDivisionNames divs;

    if (!divs.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    Notify_IntRaw notif;
    notif.data = new PyDict();
    notif.key = call.client->GetCorporationID();

    if (!m_db.UpdateDivisionNames(notif.key, divs, (PyDict *)notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update division names for corp %u", call.client->GetName(), notif.key);
        return(new PyNone());
    }

    MulticastTarget mct;
    mct.corporations.insert(notif.key);
    PyTuple * answer = notif.Encode();
    m_manager->entity_list.Multicast("OnCorporationChanged", "corpid", &answer, mct);
    answer = notif.Encode();
    m_manager->entity_list.Multicast("OnCorporationChanged", "clientID", &answer, mct);
    answer = notif.Encode();
    m_manager->entity_list.Multicast("OnCorporationChanged", "clientID", &answer, mct);

    return(new PyNone());
}

PyResult CorpRegistryBound::Handle_UpdateCorporation(PyCallArgs &call) {
    Call_UpdateCorporation upd;

    if (!upd.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    Notify_IntRaw notif;
    notif.key = call.client->GetCorporationID();
    notif.data = new PyDict();

    if (!m_db.UpdateCorporation(notif.key, upd, (PyDict*)notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update corporation data for corp %u", call.client->GetName(), notif.key);
        return new PyNone();
    }

    // Only send notification if it is needed...
    if (((PyDict*)notif.data)->items.size()) {
        MulticastTarget mct;
        mct.corporations.insert(notif.key);
        PyTuple * answer = notif.Encode();
        m_manager->entity_list.Multicast("OnCorporationChanged", "corpid", &answer, mct);
    }

    return new PyNone();
}

PyResult CorpRegistryBound::Handle_UpdateLogo(PyCallArgs &call) {
    Call_UpdateLogo upd;

    if (!upd.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    // Check if we have enough money
    uint32 logo_changeu;
    double logo_change;
    if(!m_db.GetConstant("corpLogoChangeCost", logo_changeu)) {
        codelog(SERVICE__ERROR, "%s: Failed to determine logo change costs.", call.client->GetName());
        return(new PyNone());
    }
    logo_change = logo_changeu;


    // It's here, to avoid callign GetCorporationID all the time
    Notify_IntRaw notif;
    notif.key = call.client->GetCorporationID();
    notif.data = new PyDict();

    double corp_orig = m_db.GetCorpBalance(notif.key);
    if( corp_orig < logo_change )
    {
        _log( SERVICE__ERROR, "%s: Cannot afford corporation logo change costs!", call.client->GetName() );
        call.client->SendErrorMsg( "Your corporation doesn't have enough money (%u ISK) to change it's logo!", logo_changeu );

        PyDecRef( notif.data );
        return new PyNone;
    }

    // Try to do the update. If it fails, we won't take the money.
    if( !m_db.UpdateLogo( notif.key, upd, (PyDict*)notif.data ) )
    {
        codelog( SERVICE__ERROR, "Corporation logo change failed..." );

        PyDecRef( notif.data );
        return new PyNone;
    }

    //take the money out of their wallet (sends wallet blink event)
    // The amount has to be double!!!
    if( !m_db.AddBalanceToCorp( notif.key, -logo_change ) )
    {
        codelog( SERVICE__ERROR, "%s: Failed to take money for corp logo change!", call.client->GetName() );

        PyDecRef( notif.data );
        return new PyNone;
    }

    double corp_new = m_db.GetCorpBalance(notif.key);

    //record the transaction in the journal.
    if(!m_db.GiveCash(
        notif.key,
        RefType_corpLogoChange,
        notif.key,
        1,      // who should this one be? hq's station's owner?
        "1",
        notif.key,
        accountCash,
        -logo_change,
        corp_new,
        "Changing own corporation logo."
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record corp logo change transaction.");
        //no good reason to return... the money has actually been moved.
    }

    // Send notification about the cash change
    OnAccountChange oac;
    oac.accountKey = "cash";
    oac.balance = corp_new;
    oac.ownerid = notif.key;
    PyTuple * answer = oac.Encode();

    MulticastTarget mct;
    mct.corporations.insert(notif.key);
    m_manager->entity_list.Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

    // for those in the station
    mct.locations.insert(call.client->GetLocationID());
    answer = notif.Encode();
    m_manager->entity_list.Multicast("OnCorporationChanged", "corpid", &answer, mct);

    return m_db.GetCorporation(notif.key);
}

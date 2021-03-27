/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "../../eve-common/EVE_Character.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "EntityList.h"
#include "account/AccountService.h"
#include "character/CharMgrService.h"

class CharMgrBound
: public PyBoundObject
{
public:
	PyCallable_Make_Dispatcher(CharMgrBound);

    CharMgrBound(PyServiceMgr *mgr, uint32 ownerID, uint16 contFlag)
		: PyBoundObject(mgr),
		m_dispatch(new Dispatcher(this)),
		m_ownerID(ownerID),
		m_containerFlag(contFlag)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "CharMgrBound";

        PyCallable_REG_CALL(CharMgrBound, List);
        PyCallable_REG_CALL(CharMgrBound, ListStations);
        PyCallable_REG_CALL(CharMgrBound, ListStationItems);
        PyCallable_REG_CALL(CharMgrBound, ListStationBlueprintItems);
	}

    virtual ~CharMgrBound() {delete m_dispatch;}

	virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(List);
    PyCallable_DECL_CALL(ListStations);
    PyCallable_DECL_CALL(ListStationItems);
    PyCallable_DECL_CALL(ListStationBlueprintItems);

protected:
    Dispatcher *const m_dispatch;

    uint32 m_ownerID;
    uint16 m_containerFlag;     // this is EVEContainerType defined in EVE_Inventory.h
};


PyResult CharMgrBound::Handle_List( PyCallArgs& call )
{
    return CharacterDB::List(m_ownerID);
}

PyResult CharMgrBound::Handle_ListStationItems( PyCallArgs& call )
{
    // this is the assets window
    return CharacterDB::ListStationItems(m_ownerID, PyRep::IntegerValue(call.tuple->GetItem(0)));
}

PyResult CharMgrBound::Handle_ListStations( PyCallArgs& call )
{
    //stations = sm.GetService('invCache').GetInventory(const.containerGlobal).ListStations(blueprintOnly, isCorp)

    call.Dump(CHARACTER__DEBUG);
    std::ostringstream flagIDs;
    flagIDs << flagHangar;
    /** @todo  test m_containerFlag to determine correct flag here and append to flagIDs string? */

    uint32 ownerID(m_ownerID);
    bool bpOnly(PyRep::IntegerValue(call.tuple->GetItem(0)));
    bool isCorp(PyRep::IntegerValue(call.tuple->GetItem(1)));

    if (isCorp) {
        Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
        if (pClient == nullptr)
            return nullptr; // make error here
        ownerID = pClient->GetCorporationID();
        //  add corp hangar flags
        flagIDs << "," << flagCorpHangar2 << "," << flagCorpHangar3 << "," << flagCorpHangar4 << ",";
        flagIDs << flagCorpHangar5 << "," << flagCorpHangar6 << "," << flagCorpHangar7;
    }

    return CharacterDB::ListStations(ownerID, flagIDs, isCorp, bpOnly);
}

PyResult CharMgrBound::Handle_ListStationBlueprintItems( PyCallArgs& call )
{
    // this is the BP tab of the S&I window
    call.Dump(CHARACTER__DEBUG);

    /** @todo whats diff between stationID and locationID?
     *  none that i see so far...
     * this could be diff between station and pos   will need to check later (locationID == stationID)
     */
    uint32 ownerID(m_ownerID);

    uint32 locationID(PyRep::IntegerValueU32(call.tuple->GetItem(0)));
    uint32 stationID(PyRep::IntegerValueU32(call.tuple->GetItem(1)));
    bool forCorp(PyRep::IntegerValueU32(call.tuple->GetItem(2)));

    if (forCorp) { //forCorp
        Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
        if (pClient == nullptr)
            return nullptr; // make error here
        ownerID = pClient->GetCorporationID();
    }
    return CharacterDB::ListStationBlueprintItems(ownerID, stationID, forCorp);
}


PyCallable_Make_InnerDispatcher(CharMgrService)

CharMgrService::CharMgrService(PyServiceMgr *mgr)
: PyService(mgr, "charMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CharMgrService, GetPublicInfo);
    PyCallable_REG_CALL(CharMgrService, GetPublicInfo3);
    PyCallable_REG_CALL(CharMgrService, GetPrivateInfo);
    PyCallable_REG_CALL(CharMgrService, GetTopBounties);
    PyCallable_REG_CALL(CharMgrService, AddToBounty);
    PyCallable_REG_CALL(CharMgrService, GetContactList);
    PyCallable_REG_CALL(CharMgrService, GetCloneTypeID);
    PyCallable_REG_CALL(CharMgrService, GetHomeStation);
    PyCallable_REG_CALL(CharMgrService, GetFactions);
    PyCallable_REG_CALL(CharMgrService, SetActivityStatus);
    PyCallable_REG_CALL(CharMgrService, GetSettingsInfo);
    PyCallable_REG_CALL(CharMgrService, LogSettings);
    PyCallable_REG_CALL(CharMgrService, GetCharacterDescription);
    PyCallable_REG_CALL(CharMgrService, SetCharacterDescription);
    PyCallable_REG_CALL(CharMgrService, GetPaperdollState);
    PyCallable_REG_CALL(CharMgrService, GetNote);
    PyCallable_REG_CALL(CharMgrService, SetNote);
    PyCallable_REG_CALL(CharMgrService, AddOwnerNote);
    PyCallable_REG_CALL(CharMgrService, GetOwnerNote);
    PyCallable_REG_CALL(CharMgrService, GetOwnerNoteLabels);
    PyCallable_REG_CALL(CharMgrService, AddContact);
    PyCallable_REG_CALL(CharMgrService, EditContact);
    PyCallable_REG_CALL(CharMgrService, DeleteContacts);
    PyCallable_REG_CALL(CharMgrService, GetRecentShipKillsAndLosses);
    PyCallable_REG_CALL(CharMgrService, BlockOwners);
    PyCallable_REG_CALL(CharMgrService, UnblockOwners);
    PyCallable_REG_CALL(CharMgrService, EditContactsRelationshipID);
    PyCallable_REG_CALL(CharMgrService, GetImageServerLink);
    //these 2 are for labels in PnP window;
    PyCallable_REG_CALL(CharMgrService, GetLabels);
    PyCallable_REG_CALL(CharMgrService, CreateLabel);
}

CharMgrService::~CharMgrService() {
    delete m_dispatch;
}

PyBoundObject *CharMgrService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    _log(CHARACTER__BIND, "CharMgrService bind request:");
    bind_args->Dump(CHARACTER__BIND, "    ");
    Call_TwoIntegerArgs args;
    //crap
    PyRep* tmp(bind_args->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return new CharMgrBound(m_manager, args.arg1, args.arg2);
}

PyResult CharMgrService::Handle_GetImageServerLink( PyCallArgs& call )
{
    // only called by billboard service for bounties...
    //  serverLink = sm.RemoteSvc('charMgr').GetImageServerLink()
    std::stringstream urlBuilder;
    urlBuilder << "http://" << sConfig.net.imageServer << ":" << sConfig.net.imageServerPort << "/";

    return new PyString(urlBuilder.str());
}

PyResult CharMgrService::Handle_GetRecentShipKillsAndLosses( PyCallArgs& call )
{   /* cached object - can return db object as DBResultToCRowset*/
    return m_db.GetKillOrLoss(call.client->GetCharacterID());
}

PyResult CharMgrService::Handle_GetTopBounties( PyCallArgs& call )
{
    return m_db.GetTopBounties();
}

PyResult CharMgrService::Handle_GetLabels( PyCallArgs& call )
{
    return m_db.GetLabels(call.client->GetCharacterID());
}

PyResult CharMgrService::Handle_GetPaperdollState( PyCallArgs& call )
{
    return new PyInt(Char::PDState::NoRecustomization);
}

PyResult CharMgrService::Handle_GetPublicInfo3(PyCallArgs &call)
{
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetCharPublicInfo3(arg.arg);
}

PyResult CharMgrService::Handle_GetContactList(PyCallArgs &call)
{
    PyDict* dict = new PyDict();
        dict->SetItemString("addresses", m_db.GetContacts(call.client->GetCharacterID(), false));
        dict->SetItemString("blocked", m_db.GetContacts(call.client->GetCharacterID(), true));
    PyObject* args = new PyObject("util.KeyVal", dict);
    if (is_log_enabled(CLIENT__RSP_DUMP))
        args->Dump(CLIENT__RSP_DUMP, "");
    return args;
}

PyResult CharMgrService::Handle_GetPrivateInfo( PyCallArgs& call )
{
    // self.memberinfo = self.charMgr.GetPrivateInfo(self.charID)
    // this is called by corp/editMember

    // single int arg: charid
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyRep* args(m_db.GetCharPrivateInfo(arg.arg));
    if (is_log_enabled(CLIENT__RSP_DUMP))
        args->Dump(CLIENT__RSP_DUMP, "");
    return args;
}

PyResult CharMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
    //single int arg: char id or corp id
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    /*if(IsAgent(args.arg)) {
        //handle agents special right now...
        PyRep *result = m_db.GetAgentPublicInfo(args.arg);
        if(result == nullptr) {
            codelog(CLIENT__ERROR, "%s: Failed to find agent %u", call.client->GetName(), args.arg);
            return nullptr;
        }
        return result;
    }*/

    PyRep *result = m_db.GetCharPublicInfo(arg.arg);
    if (result == nullptr)
        codelog(CHARACTER__ERROR, "%s: Failed to find char %u", call.client->GetName(), arg.arg);

    return result;
}

PyResult CharMgrService::Handle_AddToBounty( PyCallArgs& call )
{
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (call.client->GetCharacterID() == args.arg1){
        call.client->SendErrorMsg("You cannot put a bounty on yourself.");
        return nullptr;
    }

    if (args.arg2 < call.client->GetBalance()) {
        std::string reason = "Placing Bounty on ";
        reason += m_db.GetCharName(args.arg1);
        AccountService::TranserFunds(call.client->GetCharacterID(), corpCONCORD, args.arg2, reason, Journal::EntryType::Bounty, args.arg1);
        m_db.AddBounty(args.arg1, call.client->GetCharacterID(), args.arg2);
        // new system gives target a mail from concord about placement of bounty and char name placing it.
    } else {
        std::map<std::string, PyRep *> res;
        res["amount"] = new PyFloat(args.arg2);
        res["balance"] = new PyFloat(call.client->GetBalance());
        throw(PyException(MakeUserError("NotEnoughMoney", res)));
    }

    return PyStatic.NewNone();
}

PyResult CharMgrService::Handle_GetCloneTypeID( PyCallArgs& call )
{
	uint32 typeID;
	if (!m_db.GetActiveCloneType(call.client->GetCharacterID(), typeID ) )
	{
		// This should not happen, because a clone is created at char creation.
		// We don't have a clone, so return a basic one. cloneTypeID = 9917 (Clone Grade Delta)
		typeID = 9917;
		sLog.Debug( "CharMgrService", "Returning a basic clone for Char %u of type %u", call.client->GetCharacterID(), typeID );
	}
    return new PyInt(typeID);
}

PyResult CharMgrService::Handle_GetHomeStation( PyCallArgs& call )
{
	uint32 stationID = 0;
    if (!CharacterDB::GetCharHomeStation(call.client->GetCharacterID(), stationID) ) {
		sLog.Error( "CharMgrService", "Could't get the home station for Char %u", call.client->GetCharacterID() );
		return PyStatic.NewNone();
	}
    return new PyInt(stationID);
}

PyResult CharMgrService::Handle_SetActivityStatus( PyCallArgs& call ) {
    Call_TwoIntegerArgs args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    sLog.Cyan("CharMgrService::SetActivityStatus()", "Player %s(%u) AFK:%s, time:%i.", \
            call.client->GetName(), call.client->GetCharacterID(), (args.arg1 ? "true" : "false"), args.arg2);

    if (args.arg1) {
        call.client->SetAFK(true);
    } else {
        call.client->SetAFK(false);
    }

    // call code here to set an AFK watchdog?  config option?
    // returns nothing
    return nullptr;
}

PyResult CharMgrService::Handle_GetCharacterDescription(PyCallArgs &call)
{
    //takes characterID
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    sItemFactory.SetUsingClient(call.client);
    CharacterRef c = sItemFactory.GetCharacter(args.arg);
    if (!c ) {
        _log(CHARACTER__ERROR, "GetCharacterDescription failed to load character %u.", args.arg);
        return nullptr;
    }

    return new PyString(c->description());
}

PyResult CharMgrService::Handle_SetCharacterDescription(PyCallArgs &call)
{
    //takes WString of bio
    Call_SingleStringArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef c = call.client->GetChar();
    if (!c ) {
        _log(CHARACTER__ERROR, "SetCharacterDescription called with no char!");
        return nullptr;
    }
    c->SetDescription(args.arg.c_str());

    return nullptr;
}

/** ***********************************************************************
 * @note   these below are partially coded
 */

PyResult CharMgrService::Handle_GetSettingsInfo( PyCallArgs& call ) {
    /**
     *    def UpdateSettingsStatistics(self):
     *        code, verified = macho.Verify(sm.RemoteSvc('charMgr').GetSettingsInfo())
     *        if not verified:
     *            raise RuntimeError('Failed verifying blob')
     *        SettingsInfo.func_code = marshal.loads(code)
     *        ret = SettingsInfo()
     *        if len(ret) > 0:
     *            sm.RemoteSvc('charMgr').LogSettings(ret)
     */

    // Called in file "carbon/client/script/ui/services/settingsSvc.py"
    // This should return a marshaled python function.
    // It returns a tuple containing a dict that is then sent to
    // charMgr::LogSettings if the tuple has a length greater than zero.
    PyTuple* res = new PyTuple( 2 );
    // This returns an empty tuple
    unsigned char code[] = {
        0x63,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x43,0x00,0x00,
        0x00,0x73,0x0a,0x00,0x00,0x00,0x64,0x01,0x00,0x7d,0x00,0x00,0x7c,0x00,0x00,0x53,
        0x28,0x02,0x00,0x00,0x00,0x4e,0x28,0x00,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x00,
        0x28,0x01,0x00,0x00,0x00,0x74,0x03,0x00,0x00,0x00,0x74,0x75,0x70,0x28,0x00,0x00,
        0x00,0x00,0x28,0x00,0x00,0x00,0x00,0x73,0x10,0x00,0x00,0x00,0x2e,0x2f,0x6d,0x61,
        0x6b,0x65,0x5a,0x65,0x72,0x6f,0x52,0x65,0x74,0x2e,0x70,0x79,0x74,0x08,0x00,0x00,
        0x00,0x72,0x65,0x74,0x54,0x75,0x70,0x6c,0x65,0x0c,0x00,0x00,0x00,0x73,0x04,0x00,
        0x00,0x00,0x00,0x01,0x06,0x02
    };
    int codeLen = sizeof(code) / sizeof(*code);
    std::string codeString(code, code + codeLen);
    res->items[ 0 ] = new PyString(codeString);

    // error code? 0 = no error
    // if called with any value other than zero the exception output will show 'Verified = False'
    // if called with zero 'Verified = True'
    /*  Just a note
     * due to the client being in placebo mode, the verification code in evecrypto just checks if the signature is 0.
     * when the client is in cryptoapi mode it verifies the signature is signed by CCP's rsa key.
     */

    res->items[ 1 ] = new PyInt( 0 );
    return res;
}

//  this is a return call from client after GetSettingsInfo
PyResult CharMgrService::Handle_LogSettings( PyCallArgs& call ) {
    /*
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 734 bytes]
     *          [PyTuple 4 items]
     *            [PyInt 1]
     *            [PyString "LogSettings"]
     *            [PyTuple 1 items]
     *              [PyDict 36 kvp]
     *                [PyString "locale"]
     *                [PyString "en_US"]
     *                [PyString "shadowquality"]
     *                [PyInt 1]
     *                [PyString "video_vendorid"]
     *                [PyIntegerVar 4318]
     *                [PyString "dronemodelsenabled"]
     *                [PyInt 1]
     *                [PyString "loadstationenv2"]
     *                [PyInt 1]
     *                [PyString "video_deviceid"]
     *                [PyIntegerVar 3619]
     *                [PyString "camerashakesenabled"]
     *                [PyInt 1]
     *                [PyString "interiorshaderquality"]
     *                [PyInt 0]
     *                [PyString "presentation_interval"]
     *                [PyInt 1]
     *                [PyString "windowed_resolution"]
     *                [PyString "1440x900"]
     *                [PyString "explosioneffectssenabled"]
     *                [PyInt 1]
     *                [PyString "uiscalingfullscreen"]
     *                [PyFloat 1]
     *                [PyString "lod"]
     *                [PyInt 1]
     *                [PyString "audioenabled"]
     *                [PyInt 1]
     *                [PyString "voiceenabled"]
     *                [PyInt 1]
     *                [PyString "autodepth_stencilformat"]
     *                [PyInt 75]
     *                [PyString "hdrenabled"]
     *                [PyInt 1]
     *                [PyString "backbuffer_format"]
     *                [PyInt 22]
     *                [PyString "effectssenabled"]
     *                [PyInt 1]
     *                [PyString "breakpadUpload"]
     *                [PyInt 1]
     *                [PyString "windowed"]
     *                [PyBool True]
     *                [PyString "transgaming"]
     *                [PyBool False]
     *                [PyString "missilesenabled"]
     *                [PyInt 1]
     *                [PyString "sunocclusion"]
     *                [PyInt 1]
     *                [PyString "optionalupgrade"]
     *                [PyInt 0]
     *                [PyString "uiscalingwindowed"]
     *                [PyFloat 1]
     *                [PyString "textureqality"]
     *                [PyInt 0]
     *                [PyString "interiorgraphicsquality"]
     *                [PyInt 0]
     *                [PyString "antialiasing"]
     *                [PyInt 2]
     *                [PyString "defaultDockingView"]
     *                [PyString "hangar"]
     *                [PyString "turretsenabled"]
     *                [PyInt 1]
     *                [PyString "advancedcamera"]
     *                [PyInt 0]
     *                [PyString "postprocessing"]
     *                [PyInt 1]
     *                [PyString "fixedwindow"]
     *                [PyBool False]
     *                [PyString "shaderquality"]
     *                [PyInt 3]
     *                [PyString "fullscreen_resolution"]
     *                [PyString "1440x900"]
     *            [PyDict 1 kvp]
     *              [PyString "machoVersion"]
     *              [PyInt 1]
     */
    sLog.Warning( "CharMgrService::Handle_LogSettings()", "size= %u", call.tuple->size() );
    call.Dump(CHARACTER__TRACE);
    return nullptr;
}

//17:09:10 L CharMgrService::Handle_GetNote(): size= 1
PyResult CharMgrService::Handle_GetNote( PyCallArgs& call )
{
    uint32 ownerID = call.client->GetCharacterID();
    uint32 itemID = call.tuple->GetItem(0)->AsInt()->value();

	PyString *str = m_db.GetNote(ownerID, itemID);
    if(!str)
        str = new PyString("");

    return str;
}

PyResult CharMgrService::Handle_SetNote(PyCallArgs &call)
{
    Call_SetNote args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    m_db.SetNote(call.client->GetCharacterID(), args.itemID, args.note.c_str());

    return PyStatic.NewNone();
}

PyResult CharMgrService::Handle_AddOwnerNote( PyCallArgs& call ) {
    /*
    15:51:12 Server: AddOwnerNote call made to charMgr
    15:51:12 [SvcCall] Service charMgr: calling AddOwnerNote
    15:51:12 CharMgrService::Handle_AddOwnerNote(): size=2
    15:51:12 [SvcCall]   Call Arguments:
    15:51:12 [SvcCall]       Tuple: 2 elements
    15:51:12 [SvcCall]         [ 0] String: 'S:Folders'
    15:51:12 [SvcCall]         [ 1] WString: '1::F::0::Main|'
    15:51:12 [SvcCall]   Call Named Arguments:
    15:51:12 [SvcCall]     Argument 'machoVersion':
    15:51:12 [SvcCall]         Integer field: 1

    15:54:45 Server: AddOwnerNote call made to charMgr
    15:54:45 [SvcCall] Service charMgr: calling AddOwnerNote
    15:54:45 CharMgrService::Handle_AddOwnerNote(): size=2
    15:54:45 [SvcCall]   Call Arguments:
    15:54:45 [SvcCall]       Tuple: 2 elements
    15:54:45 [SvcCall]         [ 0] String: 'S:Folders'
    15:54:45 [SvcCall]         [ 1] WString: '1::F::0::Main|2::F::0::test|'
    15:54:45 [SvcCall]   Call Named Arguments:
    15:54:45 [SvcCall]     Argument 'machoVersion':
    15:54:45 [SvcCall]         Integer field: 1

    10:41:26 W CharMgrService::Handle_GetOwnerNoteLabels(): size= 0
    10:41:26 [CharDebug]   Call Arguments:
    10:41:26 [CharDebug]      Tuple: Empty
    10:41:26 [CharDebug]  Named Arguments:
    10:41:26 [CharDebug]   machoVersion
    10:41:26 [CharDebug]        Integer: 1
    10:41:39 [Service] charMgr::AddOwnerNote()
    10:41:39 W CharMgrService::Handle_AddOwnerNote(): size=2
    10:41:39 [CharDebug]   Call Arguments:
    10:41:39 [CharDebug]      Tuple: 2 elements
    10:41:39 [CharDebug]       [ 0]    WString: 'N:testing this shit'
    10:41:39 [CharDebug]       [ 1]     String: '<br>'
    10:41:39 [CharDebug]  Named Arguments:
    10:41:39 [CharDebug]   machoVersion
    10:41:39 [CharDebug]        Integer: 1

    /../carbon/client/script/ui/control/buttons.py(245) OnClick
    /client/script/ui/shared/neocom/notepad.py(706) NewNote
    /client/script/ui/shared/neocom/notepad.py(581) ShowNote
    id = 'None'
    self = form.Notepad object at 0x4052ac30, name=notepad, destroyed=False>
    force = 0
        t = 'N'
        noteID = ['N', 'None']
        ValueError: invalid literal for int() with base 10: 'None'

    /client/script/ui/shared/neocom/notepad.py(802) OnClick
    /client/script/ui/shared/neocom/notepad.py(581) ShowNote
    id = 'None'
    self = form.Notepad object at 0x4052ac30, name=notepad, destroyed=False>
    force = 0
        t = 'N'
        noteID = ['N', 'None']
        ValueError: invalid literal for int() with base 10: 'None'

    10:57:20 W CharMgrService::Handle_AddOwnerNote(): size=2
    10:57:20 [CharDebug]   Call Arguments:
    10:57:20 [CharDebug]      Tuple: 2 elements
    10:57:20 [CharDebug]       [ 0]    WString: 'N:new note in new folder'
    10:57:20 [CharDebug]       [ 1]     String: '<br>'
    10:57:20 [CharDebug]  Named Arguments:
    10:57:20 [CharDebug]   machoVersion
    10:57:20 [CharDebug]        Integer: 1

    /client/script/ui/shared/neocom/notepad.py(706) NewNote
    /client/script/ui/shared/neocom/notepad.py(581) ShowNote
    id = 'None'
    self = form.Notepad object at 0x4052ac30, name=notepad, destroyed=False>
    force = 0
        t = 'N'
        noteID = ['N', 'None']
        ValueError: invalid literal for int() with base 10: 'None'

    on notepad close....
    10:58:09 [Service] charMgr::AddOwnerNote()
    10:58:09 W CharMgrService::Handle_AddOwnerNote(): size=2
    10:58:09 [CharDebug]   Call Arguments:
    10:58:09 [CharDebug]      Tuple: 2 elements
    10:58:09 [CharDebug]       [ 0]     String: 'S:Folders'
    10:58:09 [CharDebug]       [ 1]    WString: '1::F::0::Main|2::F::0::added folder|3::N::2::None|'
    10:58:09 [CharDebug]  Named Arguments:
    10:58:09 [CharDebug]   machoVersion
    10:58:09 [CharDebug]        Integer: 1


    */

  sLog.Warning( "CharMgrService::Handle_AddOwnerNote()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}


PyResult CharMgrService::Handle_GetOwnerNote(PyCallArgs &call)
{  /*
        [PyObjectEx Type2]
          [PyTuple 2 items]
            [PyTuple 1 items]
              [PyToken dbutil.CRowset]
            [PyDict 1 kvp]
              [PyString "header"]
              [PyObjectEx Normal]
                [PyTuple 2 items]
                  [PyToken blue.DBRowDescriptor]
                  [PyTuple 1 items]
                    [PyTuple 6 items]
                      [PyTuple 2 items]
                        [PyString "noteDate"]
                        [PyInt 64]
                      [PyTuple 2 items]
                        [PyString "typeID"]
                        [PyInt 2]
                      [PyTuple 2 items]
                        [PyString "referenceID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "note"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "userID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "label"]
                        [PyInt 130]
          [PyPackedRow 19 bytes]
            ["noteDate" => <129041092800000000> [FileTime]]
            ["typeID" => <1> [I2]]
            ["referenceID" => <1661059544> [I4]]
            ["note" => <1::F::0::Main|> [WStr]]
            ["userID" => <0> [I4]]
            ["label" => <S:Folders> [WStr]]
            */

    sLog.Warning( "CharMgrService::Handle_GetOwnerNote()", "size= %u", call.tuple->size() );
    call.Dump(CHARACTER__DEBUG);
    return nullptr;
    //return m_db.GetOwnerNote(call.client->GetCharacterID());
}

PyResult CharMgrService::Handle_GetOwnerNoteLabels(PyCallArgs &call)
{  /*
        [PyObjectEx Type2]
          [PyTuple 2 items]
            [PyTuple 1 items]
              [PyToken dbutil.CRowset]
            [PyDict 1 kvp]
              [PyString "header"]
              [PyObjectEx Normal]
                [PyTuple 2 items]
                  [PyToken blue.DBRowDescriptor]
                  [PyTuple 1 items]
                    [PyTuple 2 items]
                      [PyTuple 2 items]
                        [PyString "noteID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "label"]
                        [PyInt 130]
          [PyPackedRow 5 bytes]
            ["noteID" => <4424362> [I4]]
            ["label" => <S:Folders> [WStr]]
          [PyPackedRow 5 bytes]
            ["noteID" => <4424363> [I4]]
            ["label" => <N:Ratting - Chaining Rules> [WStr]]
          [PyPackedRow 5 bytes]
            ["noteID" => <4424364> [I4]]
            ["label" => <N:Harbinger Training Schedule> [WStr]]
          [PyPackedRow 5 bytes]
            ["noteID" => <4424365> [I4]]
            ["label" => <N:Damage Types for Races and Rats> [WStr]]
          [PyPackedRow 5 bytes]
            ["noteID" => <5332889> [I4]]
            ["label" => <N:KES Link> [WStr]]
          [PyPackedRow 5 bytes]
            ["noteID" => <5536321> [I4]]
            ["label" => <N:Pelorn's PvP Route> [WStr]]
    [PyNone]
*/
  sLog.Warning( "CharMgrService::Handle_GetOwnerNoteLabels()", "size= %u", call.tuple->size() );
  call.Dump(CHARACTER__DEBUG);

    return m_db.GetOwnerNoteLabels(call.client->GetCharacterID());
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

//18:07:30 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(2784)
//18:07:35 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(63177)
PyResult CharMgrService::Handle_AddContact( PyCallArgs& call )
{
  /**
00:26:29 [SvcCall] Service charMgr: calling AddContact      *** adding agent
00:26:29 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(3017440)
00:26:29 [SvcCall]   Call Arguments:
00:26:29 [SvcCall]       Tuple: 1 elements
00:26:29 [SvcCall]         [ 0] Integer field: 3017440
00:26:29 [SvcCall]   Call Named Arguments:
00:26:29 [SvcCall]     Argument 'machoVersion':
00:26:29 [SvcCall]         Integer field: 1

PyTuple* payload = new PyTuple(1);
payload->SetItem(0, new PyInt(agentID));
pClient->SendNotification("OnAgentAdded", "charid", payload, false);    // i *think* this is unsequenced
//OnAgentAdded(self, entityID)

15:48:32 [SvcCall] Service charMgr: calling AddContact      *** adding player
15:48:32 L CharMgrService::Handle_AddContact(): size=5, 0=Integer(140000212)
15:48:32 [SvcCall]   Call Arguments:
15:48:32 [SvcCall]       Tuple: 5 elements
15:48:32 [SvcCall]         [ 0] Integer field: 140000212
15:48:32 [SvcCall]         [ 1] Integer field: 10
15:48:32 [SvcCall]         [ 2] Integer field: 1
15:48:32 [SvcCall]         [ 3] Integer field: 1
15:48:32 [SvcCall]         [ 4] String: ''
15:48:32 [SvcCall]   Call Named Arguments:
15:48:32 [SvcCall]     Argument 'machoVersion':
15:48:32 [SvcCall]         Integer field: 1
*/
  sLog.Warning( "CharMgrService::Handle_AddContact()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  // make db call to save contact.  will have to find the call to get contact list....
  return nullptr;
}

PyResult CharMgrService::Handle_EditContact( PyCallArgs& call )
{
  sLog.Warning( "CharMgrService::Handle_EditContact()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::Handle_CreateLabel( PyCallArgs& call )
{
  sLog.Warning( "CharMgrService::Handle_CreateLabel()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::Handle_DeleteContacts( PyCallArgs& call )
{
  // sm.RemoteSvc('charMgr').DeleteContacts([contactIDs])

  sLog.Warning( "CharMgrService::Handle_DeleteContacts()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::Handle_BlockOwners( PyCallArgs& call )
{
  //        sm.RemoteSvc('charMgr').BlockOwners([ownerID])
  sLog.Warning( "CharMgrService::Handle_BlockOwners()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::Handle_UnblockOwners( PyCallArgs& call )
{
  //            sm.RemoteSvc('charMgr').UnblockOwners(blocked)
  sLog.Warning( "CharMgrService::Handle_UnblockOwners()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::Handle_EditContactsRelationshipID( PyCallArgs& call )
{
  /*
            sm.RemoteSvc('charMgr').EditContactsRelationshipID(contactIDs, relationshipID)
 */
  sLog.Warning( "CharMgrService::Handle_EditContactsRelationshipID()", "size=%u ", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::Handle_GetFactions( PyCallArgs& call )
{
    sLog.Warning( "CharMgrService::Handle_GetFactions()", "size= %u", call.tuple->size() );
    call.Dump(CHARACTER__TRACE);
    return nullptr;
}

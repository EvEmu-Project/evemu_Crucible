/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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



#include "EntityList.h"
#include "account/AccountService.h"
#include "character/CharMgrService.h"
#include "packets/CorporationPkts.h"

CharMgrBound::CharMgrBound(EVEServiceManager& mgr, CharMgrService& parent, uint32 ownerID, uint16 contFlag) :
    EVEBoundObject(mgr, parent),
    m_ownerID(ownerID),
    m_containerFlag(contFlag)
{
    this->Add("List", &CharMgrBound::List);
    this->Add("ListStations", &CharMgrBound::ListStations);
    this->Add("ListStationItems", &CharMgrBound::ListStationItems);
    this->Add("ListStationBlueprintItems", &CharMgrBound::ListStationBlueprintItems);
}

PyResult CharMgrBound::List(PyCallArgs& call)
{
    return CharacterDB::List(m_ownerID);
}

PyResult CharMgrBound::ListStationItems(PyCallArgs& call, PyInt* stationID)
{
    // this is the assets window
    return CharacterDB::ListStationItems(m_ownerID, stationID->value());
}

PyResult CharMgrBound::ListStations(PyCallArgs& call, PyInt* blueprintOnly, PyInt* isCorporation)
{
    //stations = sm.GetService('invCache').GetInventory(const.containerGlobal).ListStations(blueprintOnly, isCorp)

    call.Dump(CHARACTER__DEBUG);
    std::ostringstream flagIDs;
    flagIDs << flagHangar;
    /** @todo  test m_containerFlag to determine correct flag here and append to flagIDs string? */

    uint32 ownerID(m_ownerID);

    if (isCorporation->value()) {
        Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
        if (pClient == nullptr)
            return nullptr; // make error here
        ownerID = pClient->GetCorporationID();
        //  add corp hangar flags
        flagIDs << "," << flagCorpHangar2 << "," << flagCorpHangar3 << "," << flagCorpHangar4 << ",";
        flagIDs << flagCorpHangar5 << "," << flagCorpHangar6 << "," << flagCorpHangar7;
    }

    return CharacterDB::ListStations(ownerID, flagIDs, isCorporation->value(), blueprintOnly->value());
}

PyResult CharMgrBound::ListStationBlueprintItems(PyCallArgs& call, PyInt* locationID, PyInt* stationID, PyInt* forCorporation)
{
    // this is the BP tab of the S&I window
    call.Dump(CHARACTER__DEBUG);

    /** @todo whats diff between stationID and locationID?
     *  none that i see so far...
     * this could be diff between station and pos   will need to check later (locationID == stationID)
     */
    uint32 ownerID(m_ownerID);

    if (forCorporation->value()) { //forCorp
        Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
        if (pClient == nullptr)
            return nullptr; // make error here
        ownerID = pClient->GetCorporationID();
    }
    return CharacterDB::ListStationBlueprintItems(ownerID, stationID->value(), forCorporation->value());
}

CharMgrService::CharMgrService(EVEServiceManager& mgr) :
    BindableService("charMgr", mgr)
{
    this->Add("GetPublicInfo", &CharMgrService::GetPublicInfo);
    this->Add("GetPublicInfo3", &CharMgrService::GetPublicInfo3);
    this->Add("GetPrivateInfo", &CharMgrService::GetPrivateInfo);
    this->Add("GetTopBounties", &CharMgrService::GetTopBounties);
    this->Add("AddToBounty", &CharMgrService::AddToBounty);
    this->Add("GetContactList", &CharMgrService::GetContactList);
    this->Add("GetCloneTypeID", &CharMgrService::GetCloneTypeID);
    this->Add("GetHomeStation", &CharMgrService::GetHomeStation);
    this->Add("GetFactions", &CharMgrService::GetFactions);
    this->Add("SetActivityStatus", &CharMgrService::SetActivityStatus);
    this->Add("GetSettingsInfo", &CharMgrService::GetSettingsInfo);
    this->Add("LogSettings", &CharMgrService::LogSettings);
    this->Add("GetCharacterDescription", &CharMgrService::GetCharacterDescription);
    this->Add("SetCharacterDescription", &CharMgrService::SetCharacterDescription);
    this->Add("GetPaperdollState", &CharMgrService::GetPaperdollState);
    this->Add("GetNote", &CharMgrService::GetNote);
    this->Add("SetNote", &CharMgrService::SetNote);
    this->Add("AddOwnerNote", &CharMgrService::AddOwnerNote);
    this->Add("GetOwnerNote", &CharMgrService::GetOwnerNote);
    this->Add("GetOwnerNoteLabels", &CharMgrService::GetOwnerNoteLabels);
    this->Add("AddContact", &CharMgrService::AddContact);
    this->Add("EditContact", &CharMgrService::EditContact);
    this->Add("DeleteContacts", &CharMgrService::DeleteContacts);
    this->Add("GetRecentShipKillsAndLosses", &CharMgrService::GetRecentShipKillsAndLosses);
    this->Add("BlockOwners", &CharMgrService::BlockOwners);
    this->Add("UnblockOwners", &CharMgrService::UnblockOwners);
    this->Add("EditContactsRelationshipID", &CharMgrService::EditContactsRelationshipID);
    this->Add("GetImageServerLink", &CharMgrService::GetImageServerLink);
    //these 2 are for labels in PnP window;
    this->Add("GetLabels", &CharMgrService::GetLabels);
    this->Add("CreateLabel", &CharMgrService::CreateLabel);
}

BoundDispatcher* CharMgrService::BindObject(Client *client, PyRep* bindParameters) {
    _log(CHARACTER__BIND, "CharMgrService bind request:");
    bindParameters->Dump(CHARACTER__BIND, "    ");
    Call_TwoIntegerArgs args;
    //crap
    PyRep* tmp(bindParameters->Clone());
    if (!args.Decode(&tmp)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return new CharMgrBound(this->GetServiceManager(), *this, args.arg1, args.arg2);
}

void CharMgrService::BoundReleased(CharMgrBound *bound) {
    // nothing to be done here, this bound service is a singleton
}

PyResult CharMgrService::GetImageServerLink(PyCallArgs& call)
{
    // only called by billboard service for bounties...
    //  serverLink = sm.RemoteSvc('charMgr').GetImageServerLink()
    std::stringstream urlBuilder;
    urlBuilder << "http://" << sConfig.net.imageServer << ":" << sConfig.net.imageServerPort << "/";

    return new PyString(urlBuilder.str());
}

PyResult CharMgrService::GetRecentShipKillsAndLosses(PyCallArgs& call, PyInt* num, std::optional<PyInt*> startIndex)
{   /* cached object - can return db object as DBResultToCRowset*/
    return m_db.GetKillOrLoss(call.client->GetCharacterID());
}

PyResult CharMgrService::GetTopBounties(PyCallArgs& call)
{
    return m_db.GetTopBounties();
}

PyResult CharMgrService::GetLabels(PyCallArgs& call)
{
    return m_db.GetLabels(call.client->GetCharacterID());
}

PyResult CharMgrService::GetPaperdollState(PyCallArgs& call)
{
    return new PyInt(Char::PDState::NoRecustomization);
}

PyResult CharMgrService::GetPublicInfo3(PyCallArgs &call, PyInt* characterID)
{
    return m_db.GetCharPublicInfo3(characterID->value());
}

PyResult CharMgrService::GetContactList(PyCallArgs &call)
{
    PyDict* dict = new PyDict();
        dict->SetItemString("addresses", m_db.GetContacts(call.client->GetCharacterID(), false));
        dict->SetItemString("blocked", m_db.GetContacts(call.client->GetCharacterID(), true));
    PyObject* args = new PyObject("util.KeyVal", dict);
    if (is_log_enabled(CLIENT__RSP_DUMP))
        args->Dump(CLIENT__RSP_DUMP, "");
    return args;
}

PyResult CharMgrService::GetPrivateInfo(PyCallArgs& call, PyInt* characterID)
{
    // self.memberinfo = self.charMgr.GetPrivateInfo(self.charID)
    // this is called by corp/editMember
    PyRep* args(m_db.GetCharPrivateInfo(characterID->value()));
    if (is_log_enabled(CLIENT__RSP_DUMP))
        args->Dump(CLIENT__RSP_DUMP, "");
    return args;
}

PyResult CharMgrService::GetPublicInfo(PyCallArgs &call, PyInt* ownerID) {
    //single int arg: char id or corp id
    /*if (IsAgent(args.arg)) {
        //handle agents special right now...
        PyRep *result = m_db.GetAgentPublicInfo(args.arg);
        if (result == nullptr) {
            codelog(CLIENT__ERROR, "%s: Failed to find agent %u", call.client->GetName(), args.arg);
            return nullptr;
        }
        return result;
    }*/

    PyRep *result = m_db.GetCharPublicInfo(ownerID->value());
    if (result == nullptr)
        codelog(CHARACTER__ERROR, "%s: Failed to find char %u", call.client->GetName(), ownerID->value());

    return result;
}

PyResult CharMgrService::AddToBounty(PyCallArgs& call, PyInt* characterID, PyInt* amount)
{
    if (call.client->GetCharacterID() == characterID->value()){
        call.client->SendErrorMsg("You cannot put a bounty on yourself.");
        return nullptr;
    }

    if (amount->value() < call.client->GetBalance()) {
        std::string reason = "Placing Bounty on ";
        reason += m_db.GetCharName(characterID->value());
        AccountService::TranserFunds(call.client->GetCharacterID(), corpCONCORD, amount->value(), reason, Journal::EntryType::Bounty, characterID->value());
        m_db.AddBounty(characterID->value(), call.client->GetCharacterID(), amount->value());
        // new system gives target a mail from concord about placement of bounty and char name placing it.
    } else {
        std::map<std::string, PyRep *> res;
        res["amount"] = new PyFloat(amount->value());
        res["balance"] = new PyFloat(call.client->GetBalance());
        throw UserError ("NotEnoughMoney")
                .AddISK ("amount", amount->value())
                .AddISK ("balance", call.client->GetBalance ());
    }

    return PyStatic.NewNone();
}

PyResult CharMgrService::GetCloneTypeID(PyCallArgs& call)
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

PyResult CharMgrService::GetHomeStation(PyCallArgs& call)
{
	uint32 stationID = 0;
    if (!CharacterDB::GetCharHomeStation(call.client->GetCharacterID(), stationID) ) {
		sLog.Error( "CharMgrService", "Could't get the home station for Char %u", call.client->GetCharacterID() );
		return PyStatic.NewNone();
	}
    return new PyInt(stationID);
}

PyResult CharMgrService::SetActivityStatus(PyCallArgs& call, PyInt* afk, PyInt* secondsAFK) {
    sLog.Cyan("CharMgrService::SetActivityStatus()", "Player %s(%u) AFK:%s, time:%i.", \
            call.client->GetName(), call.client->GetCharacterID(), (afk->value() ? "true" : "false"), secondsAFK->value());

    if (afk->value()) {
        call.client->SetAFK(true);
    } else {
        call.client->SetAFK(false);
    }

    // call code here to set an AFK watchdog?  config option?
    // returns nothing
    return nullptr;
}

PyResult CharMgrService::GetCharacterDescription(PyCallArgs &call, PyInt* characterID)
{
    sItemFactory.SetUsingClient(call.client);
    CharacterRef c = sItemFactory.GetCharacterRef(characterID->value());
    if (!c ) {
        _log(CHARACTER__ERROR, "GetCharacterDescription failed to load character %u.", characterID->value());
        return nullptr;
    }

    return new PyString(c->description());
}

PyResult CharMgrService::SetCharacterDescription(PyCallArgs &call, PyWString* description)
{
    CharacterRef c = call.client->GetChar();
    if (!c ) {
        _log(CHARACTER__ERROR, "SetCharacterDescription called with no char!");
        return nullptr;
    }
    c->SetDescription(description->content().c_str());

    return nullptr;
}

/** ***********************************************************************
 * @note   these below are partially coded
 */

PyResult CharMgrService::GetSettingsInfo(PyCallArgs& call) {
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

// this takes in the value returned from the function we return in GetSettingsInfo
// based on the captured data, this is used to gather info about game settings the players use
PyResult CharMgrService::LogSettings(PyCallArgs& call, PyRep* settingsInfoRet) {
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
    sLog.Warning( "CharMgrService::Handle_LogSettings()", "size= %li", call.tuple->size());
    call.Dump(CHARACTER__TRACE);
    return nullptr;
}

//17:09:10 L CharMgrService::Handle_GetNote(): size= 1
PyResult CharMgrService::GetNote(PyCallArgs& call, PyInt* itemID)
{
    uint32 ownerID = call.client->GetCharacterID();

	PyString *str = m_db.GetNote(ownerID, itemID->value());
    if (!str)
        str = new PyString("");

    return str;
}

PyResult CharMgrService::SetNote(PyCallArgs &call, PyInt* itemID, PyString* note)
{
    m_db.SetNote(call.client->GetCharacterID(), itemID->value(), note->content().c_str());

    return PyStatic.NewNone();
}

PyResult CharMgrService::AddOwnerNote(PyCallArgs& call, PyString* idStr, PyWString* part) {
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

  sLog.Warning( "CharMgrService::Handle_AddOwnerNote()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}


PyResult CharMgrService::GetOwnerNote(PyCallArgs &call, PyInt* noteID)
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

    sLog.Warning( "CharMgrService::Handle_GetOwnerNote()", "size= %li", call.tuple->size());
    call.Dump(CHARACTER__DEBUG);
    return nullptr;
    //return m_db.GetOwnerNote(call.client->GetCharacterID());
}

PyResult CharMgrService::GetOwnerNoteLabels(PyCallArgs &call)
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
  sLog.Warning( "CharMgrService::Handle_GetOwnerNoteLabels()", "size= %li", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

    return m_db.GetOwnerNoteLabels(call.client->GetCharacterID());
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

//18:07:30 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(2784)
//18:07:35 L CharMgrService::Handle_AddContact(): size=1, 0=Integer(63177)
PyResult CharMgrService::AddContact(PyCallArgs& call, PyInt* characterID, PyFloat* standing, PyInt* inWatchlist, PyBool* notify, std::optional<PyWString*> note)
{
  sLog.Warning( "CharMgrService::Handle_AddContact()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

    //TODO: Notify char that they have been added as a contact if notify is True

    m_db.AddContact(call.client->GetCharacterID(), characterID->value(), standing->value(), inWatchlist->value());

  return nullptr;
}

PyResult CharMgrService::EditContact(PyCallArgs& call, PyInt* characterID, PyFloat* standing, PyInt* inWatchlist, PyBool* notify, std::optional<PyWString*> note)
{
  sLog.Warning( "CharMgrService::Handle_EditContact()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  m_db.UpdateContact(standing->value(), characterID->value(), call.client->GetCharacterID());
  return nullptr;
}

PyResult CharMgrService::CreateLabel(PyCallArgs& call)
{
  sLog.Warning( "CharMgrService::Handle_CreateLabel()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  return nullptr;
}

PyResult CharMgrService::DeleteContacts(PyCallArgs& call, PyList* contactIDs)
{
  // sm.RemoteSvc('charMgr').DeleteContacts([contactIDs])

  sLog.Warning( "CharMgrService::Handle_DeleteContacts()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  for (PyList::const_iterator itr = contactIDs->begin(); itr != contactIDs->end(); ++itr) {
      m_db.RemoveContact(PyRep::IntegerValueU32(*itr), call.client->GetCharacterID());
  }

  return nullptr;
}

PyResult CharMgrService::BlockOwners(PyCallArgs& call, PyList* ownerIDs)
{
  //        sm.RemoteSvc('charMgr').BlockOwners([ownerID])
  sLog.Warning( "CharMgrService::Handle_BlockOwners()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  for (PyList::const_iterator itr = ownerIDs->begin(); itr != ownerIDs->end(); ++itr) {
      m_db.SetBlockContact(PyRep::IntegerValueU32(*itr), call.client->GetCharacterID(), true);
  }

  return nullptr;
}

PyResult CharMgrService::UnblockOwners(PyCallArgs& call, PyList* ownerIDs)
{
  //            sm.RemoteSvc('charMgr').UnblockOwners(blocked)
  sLog.Warning( "CharMgrService::Handle_UnblockOwners()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  for (PyList::const_iterator itr = ownerIDs->begin(); itr != ownerIDs->end(); ++itr) {
      m_db.SetBlockContact(PyRep::IntegerValueU32(*itr), call.client->GetCharacterID(), false);
  }

  return nullptr;
}

PyResult CharMgrService::EditContactsRelationshipID(PyCallArgs& call, PyList* contactIDs, PyInt* relationshipID)
{
  /*
            sm.RemoteSvc('charMgr').EditContactsRelationshipID(contactIDs, relationshipID)
 */
  sLog.Warning( "CharMgrService::Handle_EditContactsRelationshipID()", "size=%lu", call.tuple->size());
  call.Dump(CHARACTER__DEBUG);

  for (PyList::const_iterator itr = contactIDs->begin(); itr != contactIDs->end(); ++itr) {
      m_db.UpdateContact(relationshipID->value(), PyRep::IntegerValueU32(*itr), call.client->GetCharacterID());
  }

  return nullptr;
}

PyResult CharMgrService::GetFactions(PyCallArgs& call)
{
    sLog.Warning( "CharMgrService::Handle_GetFactions()", "size= %li", call.tuple->size());
    call.Dump(CHARACTER__TRACE);
    return nullptr;
}

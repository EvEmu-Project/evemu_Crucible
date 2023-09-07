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


#ifndef __CHARMGR_SERVICE_H_INCL__
#define __CHARMGR_SERVICE_H_INCL__

#include "character/CharacterDB.h"
#include "services/BoundService.h"

class CharMgrBound;

class CharMgrService : public BindableService <CharMgrService, CharMgrBound> {
public:
    CharMgrService(EVEServiceManager& mgr);

    void BoundReleased (CharMgrBound* bound) override;

protected:
    CharacterDB m_db;    //using this for now until we decide if we need to split them. Might be bad since we actually have two instances of it, but so far it has no member data.

    PyResult GetPublicInfo(PyCallArgs& call, PyInt* ownerID);
    PyResult GetPublicInfo3(PyCallArgs& call, PyInt* characterID);
    PyResult GetPrivateInfo(PyCallArgs& call, PyInt* characterID);
    PyResult AddToBounty(PyCallArgs& call, PyInt* characterID, PyInt* amount);
    PyResult GetTopBounties(PyCallArgs& call);
    PyResult AddOwnerNote(PyCallArgs& call, PyString* idStr, PyWString* part);
    PyResult GetOwnerNote(PyCallArgs& call, PyInt* noteID);
    PyResult GetOwnerNoteLabels(PyCallArgs& call);
    PyResult GetContactList(PyCallArgs& call);
    PyResult GetCloneTypeID(PyCallArgs& call);
    PyResult GetHomeStation(PyCallArgs& call);
    PyResult GetFactions(PyCallArgs& call);
    PyResult SetActivityStatus(PyCallArgs& call, PyInt* afk, PyInt* secondsAFK);
    PyResult GetSettingsInfo(PyCallArgs& call);
    PyResult LogSettings(PyCallArgs& call, PyRep* settingsInfoRet);
    PyResult GetCharacterDescription(PyCallArgs& call, PyInt* characterID);
    PyResult SetCharacterDescription(PyCallArgs& call, PyWString* description);
    PyResult GetPaperdollState(PyCallArgs& call);
    PyResult GetNote(PyCallArgs& call, PyInt* itemID);
    PyResult SetNote(PyCallArgs& call, PyInt* itemID, PyString* note);
    PyResult AddContact(PyCallArgs& call, PyInt* characterID, PyInt* standing, PyInt* inWatchlist, PyInt* notify, std::optional<PyString*> note);
    PyResult AddContact(PyCallArgs& call, PyInt* characterID, PyFloat* standing, PyInt* inWatchlist, PyBool* notify, std::optional<PyWString*> note);
    PyResult EditContact(PyCallArgs& call, PyInt* characterID, PyInt* standing, PyInt* inWatchlist, PyInt* notify, std::optional<PyString*> note);
    PyResult EditContact(PyCallArgs& call, PyInt* characterID, PyFloat* standing, PyInt* inWatchlist, PyBool* notify, std::optional<PyWString*> note);
    PyResult GetRecentShipKillsAndLosses(PyCallArgs& call, PyInt* num, std::optional<PyInt*> startIndex);
    PyResult GetLabels(PyCallArgs& call);
    PyResult CreateLabel(PyCallArgs& call);
    PyResult DeleteContacts(PyCallArgs& call, PyList* contactIDs);
    PyResult BlockOwners(PyCallArgs& call, PyList* ownerIDs);
    PyResult UnblockOwners(PyCallArgs& call, PyList* ownerIDs);
    PyResult EditContactsRelationshipID(PyCallArgs& call, PyList* contactIDs, PyInt* relationshipID);
    PyResult GetImageServerLink(PyCallArgs& call);

	//overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters) override;
};


class CharMgrBound : public EVEBoundObject <CharMgrBound>
{
public:
    CharMgrBound(EVEServiceManager& mgr, CharMgrService& parent, uint32 ownerID, uint16 contFlag);

protected:
    PyResult List(PyCallArgs& call);
    PyResult ListStations(PyCallArgs& call, PyInt* blueprintOnly, PyBool* isCorporation);
    PyResult ListStations(PyCallArgs& call, PyInt* blueprintOnly, PyInt* isCorporation);
    PyResult ListStationItems(PyCallArgs& call, PyInt* stationID);
    PyResult ListStationBlueprintItems(PyCallArgs& call, PyInt* locationID, PyInt* stationID, PyInt* forCorporation);

private:
    uint32 m_ownerID;
    uint16 m_containerFlag;     // this is EVEContainerType defined in EVE_Inventory.h
};


#endif

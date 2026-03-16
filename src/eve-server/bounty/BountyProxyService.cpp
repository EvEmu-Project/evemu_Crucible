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
    Author:     Zhur
*/

#include "eve-server.h"

#include "bounty/BountyProxyService.h"

BountyProxyService::BountyProxyService() :
    Service<BountyProxyService>("bountyProxy")
{
    this->Add("GetBounties", &BountyProxyService::GetBounties);
    this->Add("GetMyBounties", &BountyProxyService::GetMyBounties);
    this->Add("GetTopPilotBounties", &BountyProxyService::GetTopPilotBounties);
    this->Add("GetTopCorpBounties", &BountyProxyService::GetTopCorpBounties);
    this->Add("GetTopAllianceBounties", &BountyProxyService::GetTopAllianceBounties);
    this->Add("GetTopPilotBountyHunters", &BountyProxyService::GetTopPilotBountyHunters);
    this->Add("GetTopCorporationBountyHunters", &BountyProxyService::GetTopCorporationBountyHunters);
    this->Add("GetTopAllianceBountyHunters", &BountyProxyService::GetTopAllianceBountyHunters);
    this->Add("AddToBounty", &BountyProxyService::AddToBounty);
    this->Add("SearchCharBounties", &BountyProxyService::SearchCharBounties);
    this->Add("SearchCorpBounties", &BountyProxyService::SearchCorpBounties);
    this->Add("SearchAllianceBounties", &BountyProxyService::SearchAllianceBounties);
    this->Add("GetBountiesAndKillRights", &BountyProxyService::GetBountiesAndKillRights);
    this->Add("SellKillRight", &BountyProxyService::SellKillRight);
    this->Add("CancelSellKillRight", &BountyProxyService::CancelSellKillRight);
    this->Add("GMReimburseBounties", &BountyProxyService::GMReimburseBounties);
    this->Add("GMClearBountyCache", &BountyProxyService::GMClearBountyCache);
}

BountyProxyService::~BountyProxyService()
{
}

PyResult BountyProxyService::GetBounties(PyCallArgs& call, PyRep* ownerIDs)
{
    sLog.Debug("BountyProxyService", "GetBounties called");
    
    PyDict* result = new PyDict();
    
    if (ownerIDs != nullptr) {
        PyList* idList = nullptr;
        
        // Try to convert to list (handles both list and set)
        if (ownerIDs->IsList()) {
            idList = ownerIDs->AsList();
        } else if (ownerIDs->IsDict()) {
            // set might be serialized as dict
            PyDict* idDict = ownerIDs->AsDict();
            if (idDict != nullptr) {
                // Create a list from dict keys
                idList = new PyList();
                for (PyDict::const_iterator itr = idDict->begin(); itr != idDict->end(); ++itr) {
                    idList->AddItem(itr->first->Clone());
                }
            }
        } else if (ownerIDs->IsTuple()) {
            // Handle tuple as well
            PyTuple* idTuple = ownerIDs->AsTuple();
            if (idTuple != nullptr) {
                idList = new PyList();
                for (size_t i = 0; i < idTuple->size(); ++i) {
                    idList->AddItem(idTuple->GetItem(i)->Clone());
                }
            }
        }
        
        if (idList != nullptr) {
            for (size_t i = 0; i < idList->size(); ++i) {
                PyInt* ownerID = idList->GetItem(i)->AsInt();
                if (ownerID != nullptr) {
                    // Create a list containing a dict with bounty information
                    PyList* bountyList = new PyList();
                    
                    // Create a bounty object (util.KeyVal with bounty field)
                    PyDict* bountyObj = new PyDict();
                    bountyObj->SetItemString("bounty", new PyFloat(0.0));
                    bountyObj->SetItemString("ownerID", new PyInt(ownerID->value()));
                    
                    PyObject* bountyKeyVal = new PyObject("util.KeyVal", bountyObj);
                    bountyList->AddItem(bountyKeyVal);
                    
                    result->SetItem(new PyInt(ownerID->value()), bountyList);
                }
            }
        }
    }
    
    return result;
}

PyResult BountyProxyService::GetMyBounties(PyCallArgs& call)
{
    sLog.Debug("BountyProxyService", "GetMyBounties called");
    PyList* result = new PyList();
    return result;
}

PyResult BountyProxyService::GetTopPilotBounties(PyCallArgs& call, PyInt* page)
{
    sLog.Debug("BountyProxyService", "GetTopPilotBounties called with page %d", page ? page->value() : 0);
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(GetFileTimeNow()));
    
    return result;
}

PyResult BountyProxyService::GetTopCorpBounties(PyCallArgs& call, PyInt* page)
{
    sLog.Debug("BountyProxyService", "GetTopCorpBounties called with page %d", page ? page->value() : 0);
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(GetFileTimeNow()));
    
    return result;
}

PyResult BountyProxyService::GetTopAllianceBounties(PyCallArgs& call, PyInt* page)
{
    sLog.Debug("BountyProxyService", "GetTopAllianceBounties called with page %d", page ? page->value() : 0);
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(GetFileTimeNow()));
    
    return result;
}

PyResult BountyProxyService::GetTopPilotBountyHunters(PyCallArgs& call, PyInt* page)
{
    sLog.Debug("BountyProxyService", "GetTopPilotBountyHunters called with page %d", page ? page->value() : 0);
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(GetFileTimeNow()));
    
    return result;
}

PyResult BountyProxyService::GetTopCorporationBountyHunters(PyCallArgs& call, PyInt* page)
{
    sLog.Debug("BountyProxyService", "GetTopCorporationBountyHunters called with page %d", page ? page->value() : 0);
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(GetFileTimeNow()));
    
    return result;
}

PyResult BountyProxyService::GetTopAllianceBountyHunters(PyCallArgs& call, PyInt* page)
{
    sLog.Debug("BountyProxyService", "GetTopAllianceBountyHunters called with page %d", page ? page->value() : 0);
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyLong(GetFileTimeNow()));
    
    return result;
}

PyResult BountyProxyService::AddToBounty(PyCallArgs& call, PyInt* ownerID, PyInt* amount)
{
    sLog.Debug("BountyProxyService", "AddToBounty called for owner %u amount %d", 
               ownerID ? ownerID->value() : 0, amount ? amount->value() : 0);
    
    return new PyFloat(0.0);
}

PyResult BountyProxyService::SearchCharBounties(PyCallArgs& call, PyInt* charID)
{
    sLog.Debug("BountyProxyService", "SearchCharBounties called for char %u", charID ? charID->value() : 0);
    
    PyList* result = new PyList();
    return result;
}

PyResult BountyProxyService::SearchCorpBounties(PyCallArgs& call, PyInt* corpID)
{
    sLog.Debug("BountyProxyService", "SearchCorpBounties called for corp %u", corpID ? corpID->value() : 0);
    
    PyList* result = new PyList();
    return result;
}

PyResult BountyProxyService::SearchAllianceBounties(PyCallArgs& call, PyInt* allianceID)
{
    sLog.Debug("BountyProxyService", "SearchAllianceBounties called for alliance %u", allianceID ? allianceID->value() : 0);
    
    PyList* result = new PyList();
    return result;
}

PyResult BountyProxyService::GetBountiesAndKillRights(PyCallArgs& call, PyList* bountiesToFetch, PyList* killRightsToFetch)
{
    sLog.Debug("BountyProxyService", "GetBountiesAndKillRights called");
    
    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyList());
    result->SetItem(1, new PyList());
    
    return result;
}

PyResult BountyProxyService::SellKillRight(PyCallArgs& call, PyInt* killRightID, PyInt* price, PyRep* restrictedTo)
{
    sLog.Debug("BountyProxyService", "SellKillRight called for killRight %u price %d", 
               killRightID ? killRightID->value() : 0, price ? price->value() : 0);
    
    return PyStatic.NewNone();
}

PyResult BountyProxyService::CancelSellKillRight(PyCallArgs& call, PyInt* killRightID, PyInt* toID)
{
    sLog.Debug("BountyProxyService", "CancelSellKillRight called for killRight %u to %u", 
               killRightID ? killRightID->value() : 0, toID ? toID->value() : 0);
    
    return PyStatic.NewNone();
}

PyResult BountyProxyService::GMReimburseBounties(PyCallArgs& call)
{
    sLog.Debug("BountyProxyService", "GMReimburseBounties called");
    return PyStatic.NewNone();
}

PyResult BountyProxyService::GMClearBountyCache(PyCallArgs& call)
{
    sLog.Debug("BountyProxyService", "GMClearBountyCache called");
    return PyStatic.NewNone();
}

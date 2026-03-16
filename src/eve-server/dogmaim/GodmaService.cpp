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
    Author:        EVEmu Team
*/

#include "eve-server.h"

#include "dogmaim/GodmaService.h"
#include "character/Character.h"
#include "Client.h"

#include <set>

GodmaService::GodmaService() :
    Service("godma")
{
    this->Add("GetItem", &GodmaService::GetItem);
    this->Add("GetType", &GodmaService::GetType);
    this->Add("GetStateManager", &GodmaService::GetStateManager);
}

PyResult GodmaService::GetItem(PyCallArgs& call, PyInt* itemID)
{
    sLog.Debug("GodmaService", "GetItem called for itemID %u", itemID->value());
    
    CharacterRef charRef = call.client->GetChar();
    if (charRef.get() == nullptr) {
        sLog.Error("GodmaService", "GetItem: Character not found");
        return PyStatic.NewNone();
    }
    
    if (itemID->value() != charRef->itemID()) {
        sLog.Error("GodmaService", "GetItem: Requested itemID %u does not match character ID %u", 
                   itemID->value(), charRef->itemID());
        return PyStatic.NewNone();
    }
    
    int64 itemTime = GetFileTimeNow();
    int64 wallclockTime = GetFileTimeNow();
    
    PyDict* result = new PyDict();
    
    result->SetItemString("itemID", new PyInt(charRef->itemID()));
    result->SetItemString("time", new PyLong(itemTime));
    result->SetItemString("wallclockTime", new PyLong(wallclockTime));
    
    PyObject* invItem = new PyObject("util.KeyVal", new PyDict());
    PyDict* invItemDict = invItem->AsDict();
    invItemDict->SetItemString("itemID", new PyInt(charRef->itemID()));
    invItemDict->SetItemString("typeID", new PyInt(charRef->typeID()));
    invItemDict->SetItemString("ownerID", new PyInt(charRef->ownerID()));
    invItemDict->SetItemString("locationID", new PyInt(charRef->locationID()));
    invItemDict->SetItemString("flagID", new PyInt(charRef->flag()));
    invItemDict->SetItemString("quantity", new PyInt(1));
    invItemDict->SetItemString("singleton", new PyInt(1));
    result->SetItemString("invItem", invItem);
    
    result->SetItemString("description", new PyString(charRef->description().c_str()));
    
    PyDict* activeEffects = new PyDict();
    result->SetItemString("activeEffects", activeEffects);
    
    PyDict* attributes = new PyDict();
    
    AttributeMap* attrMap = charRef->GetAttributeMap();
    if (attrMap != nullptr) {
        DBQueryResult res;
        if (sDatabase.RunQuery(res, "SELECT attributeID FROM dgmAttributeTypes")) {
            std::set<uint32> validAttributes;
            DBResultRow row;
            while (res.GetRow(row)) {
                validAttributes.insert(row.GetUInt(0));
            }
            
            for (auto attr = attrMap->begin(); attr != attrMap->end(); ++attr) {
                uint32 attributeID = attr->first;
                if (validAttributes.find(attributeID) != validAttributes.end()) {
                    attributes->SetItem(new PyInt(attributeID), attr->second.GetPyObject());
                } else {
                    sLog.Warning("GodmaService", "GetItem: Attribute %u not found in dgmAttributeTypes, skipping", attributeID);
                }
            }
        }
    }
    result->SetItemString("attributes", attributes);
    
    PyDict* boosters = new PyDict();
    result->SetItemString("boosters", boosters);
    
    return result;
}

PyResult GodmaService::GetType(PyCallArgs& call, PyInt* typeID)
{
    sLog.Debug("GodmaService", "GetType called for typeID %u", typeID->value());
    return PyStatic.NewNone();
}

PyResult GodmaService::GetStateManager(PyCallArgs& call)
{
    sLog.Debug("GodmaService", "GetStateManager called");
    return PyStatic.NewNone();
}

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
    Author:        Bloody.Rabbit
*/

#include "eve-server.h"

#include "inventory/Inventory.h"
#include "system/SolarSystem.h"

/** @note  NOTE::  this class is ONLY for the inventory item and associated references..... need to update this...  */


/*
    Border = Borders another Region or Constellation
    Fringe = 1 connection to this system (dead end system)
    Corridor = 2 connections to this system (in one side and out the other)
    Hub = 3+ connections to this system
    International = always has Border/Constellation, almost always Regional
    Regional = always has Border/Constellation
    Constellation = always the same as Border
    Security = If it is positive, floor to nearest 1/10th gives the in-game security level. 0 or lower are 0.0 in-game.
    */


/*
 * SolarSystem
 */
SolarSystem::SolarSystem(
    uint32 _solarSystemID,
    const ItemType &_type,
    const ItemData &_data,
    const CelestialObjectData &_cData,
    const SolarSystemData &_ssData)
: CelestialObject(_solarSystemID, _type, _data, _cData),
m_data(_ssData)
{
    pInventory = new Inventory(InventoryItemRef(this));

    m_security = _ssData.security;
    m_radius = _ssData.radius;

    _log(ITEM__TRACE, "Created SolarSystem Item %p for %s (%u).", this, name(), m_itemID);
}

SolarSystem::~SolarSystem() {
    if (pInventory != nullptr)
        pInventory->Unload();
    SafeDelete(pInventory);
}

SolarSystemRef SolarSystem::Load( uint32 solarSystemID)
{
    return InventoryItem::Load<SolarSystem>(solarSystemID);
}

bool SolarSystem::_Load()
{
    return CelestialObject::_Load();
}

void SolarSystem::AddItemToInventory(InventoryItemRef iRef)
{
    _log(ITEM__TRACE, "SS::AddItemToInventory() - adding %s(%u) to inventory of %s(%u)",\
            iRef->name(), iRef->itemID(), name(), m_itemID);
    AddItem( iRef );
}

void SolarSystem::RemoveItemFromInventory( InventoryItemRef iRef )
{
    _log(ITEM__TRACE, "SS::RemoveItemFromInventory() - removing item %s(%u) from inventory of %s(%u)",\
            iRef->name(), iRef->itemID(), name(), m_itemID);
    RemoveItem( iRef );
}

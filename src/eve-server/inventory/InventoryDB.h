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
*/

#ifndef __INVENTORYDB_H_INCL__
#define __INVENTORYDB_H_INCL__

#include "ServiceDB.h"
#include "inventory/ItemRef.h"


class CharacterTypeData;
class ShipTypeData;
class CharacterAppearance;
class CelestialObjectData;
class SolarSystemData;

class InventoryDB
: public ServiceDB
{
public:
    /**
     * Loads character type data.
     *
     * @param[in] bloodlineID Bloodline to be loaded.
     * @param[out] into Where loaded data should be stored.
     * @return True on success, false on failure.
     */
    bool GetCharacterType(uint8 bloodlineID, CharacterTypeData &into);
    /**
     * Obtains ID of character type based on bloodline.
     *
     * @param[in] bloodlineID ID of bloodline.
     * @param[out] characterTypeID Resulting ID of character type.
     * @return True on success, false on failure.
     */
    static bool GetCharacterTypeByBloodline(uint8 bloodlineID, uint16 &characterTypeID);
    /**
     * Obtains ID of bloodline based on character type.
     *
     * @param[in] characterTypeID ID of character type.
     * @param[out] bloodlineID Resulting ID of bloodline.
     * @return True on success, false on failure.
     */
    bool GetBloodlineByCharacterType(uint16 characterTypeID, uint8& bloodlineID);

    /**
     * Obtains bloodline and loads character type data.
     *
     * @param[in] characterTypeID ID of character type to be loaded.
     * @param[out] bloodlineID Resulting bloodline.
     * @param[out] into Where character type data should be stored.
     * @return True on success, false on failure.
     */
    bool GetCharacterType(uint16 characterTypeID, uint8 &bloodlineID, CharacterTypeData &into);
    /**
     * Obtains ID of character type and loads it.
     *
     * @param[in] bloodlineID ID of bloodline to be loaded.
     * @param[out] characterTypeID Resulting character type.
     * @param[out] into Where loaded character type data should be stored.
     * @return True on success, false on failure.
     */
    bool GetCharacterTypeByBloodline(uint8 bloodlineID, uint16 &characterTypeID, CharacterTypeData &into);

    /**
     * Loads ship type data into given container.
     *
     * @param[in] shipTypeID ID of ship type.
     * @param[in] into Container to load data into.
     * @return True on success, false on failure.
     */
    bool GetShipType(uint16 shipTypeID, ShipTypeData &into);

    bool GetItemContents(OwnerData &od, std::vector<uint32> &into);
    bool GetItemContents(uint32 itemID, EVEItemFlags flag, std::vector<uint32> &into);
    bool GetItemContents(uint32 itemID, EVEItemFlags flag, uint32 ownerID, std::vector<uint32> &into);

    static void DeleteTrackingCans();

    /*
     * Character stuff also used by agents
     */
    bool GetCharacterData(uint32 characterID, CharacterData &into);
    bool GetCorpData(uint32 characterID, CorpData &into);

    /** @todo update these below to use static data manager */
    // get this shit outta here.....
    bool GetCelestialObject(uint32 celestialID, CelestialObjectData &into);
    bool GetSolarSystem(uint32 solarSystemID, SolarSystemData &into);
};

#endif

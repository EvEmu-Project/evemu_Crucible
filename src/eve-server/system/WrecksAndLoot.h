/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2014 The EVEmu Team
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
    Author:        Aknor Jaden
*/

#ifndef WRECKS_AND_LOOT_H
#define WRECKS_AND_LOOT_H


// //////////////// Permanent Memory Object Classes //////////////////////

// This class is a singleton object, containing all Effects loaded from dgmEffects table as memory objects of type MEffect:
class DGM_Types_to_Wrecks_Table
: public Singleton< DGM_Types_to_Wrecks_Table >
{
public:
    DGM_Types_to_Wrecks_Table();
    ~DGM_Types_to_Wrecks_Table();

    // Initializes the Table:
    int Initialize();

    // Returns typeID of wreck that is appropriate for the given typeID, returns 0 if no match
    uint32 GetWreckID(uint32 typeID);

protected:
    void _Populate();

    std::map<uint32, uint32> m_WrecksToTypesMap;
};

#define sDGM_Types_to_Wrecks_Table \
    ( DGM_Types_to_Wrecks_Table::get() )
// -----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////

#endif

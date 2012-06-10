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
    Author:        Luck
*/

#ifndef MODIFY_SHIP_ATTRIBUTES_COMPONENT_H
#define MODIFY_SHIP_ATTRIBUTES_COMPONENT_H

#include "inventory/ItemRef.h"
#include "ship/modules/ModuleDefs.h"

class GenericModule;

class ModifyShipAttributesComponent
{
public:
    ModifyShipAttributesComponent(GenericModule * mod, ShipRef ship);
    ~ModifyShipAttributesComponent();

    void ModifyShipAttribute(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type);

    void ModifyTargetShipAttribute(uint32 targetItemID, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type );

private:

    void _modifyShipAttributes(ShipRef ship, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type);

    EvilNumber _calculateNewValue(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type, std::vector<GenericModule *> mods);

    EvilNumber _calculateNewAttributeValue( EvilNumber sourceAttr, EvilNumber targetAttr, EVECalculationType type, int stackNumber );

    std::vector<GenericModule *> _sortModules(uint32 sortAttrID, std::vector<GenericModule *> mods);


    //internal access to owner
    GenericModule *m_Mod;
    ShipRef m_Ship;

};

#endif

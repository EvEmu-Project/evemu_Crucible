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

#ifndef __ITEM_REF__H__INCL__
#define __ITEM_REF__H__INCL__

//#include "eve-common.h"
#include "memory/RefPtr.h"

class AsteroidItem;
class InventoryItem;
class Blueprint;
class ShipItem;
class CelestialObject;
class Skill;
class ModuleItem;
class ProbeItem;
class Certificate;
class StructureItem;
class CargoContainer;
class WreckContainer;
class Character;
class SolarSystem;
class StationItem;
class StationOffice;

/*
 * Typedefs for all item classes we have:
 */
typedef RefPtr<AsteroidItem>             AsteroidItemRef;
typedef RefPtr<InventoryItem>            InventoryItemRef;
typedef RefPtr<Blueprint>                BlueprintRef;
typedef RefPtr<ShipItem>                 ShipItemRef;
typedef RefPtr<CelestialObject>          CelestialObjectRef;
typedef RefPtr<ProbeItem>                ProbeItemRef;
typedef RefPtr<Skill>                    SkillRef;
typedef RefPtr<ModuleItem>               ModuleItemRef;
typedef RefPtr<Certificate>              CertificateRef;
typedef RefPtr<StructureItem>            StructureItemRef;
typedef RefPtr<CargoContainer>           CargoContainerRef;
typedef RefPtr<WreckContainer>           WreckContainerRef;
typedef RefPtr<Character>                CharacterRef;
typedef RefPtr<const Character>          CharacterConstRef;
typedef RefPtr<SolarSystem>              SolarSystemRef;
typedef RefPtr<StationItem>              StationItemRef;
typedef RefPtr<StationOffice>            StationOfficeRef;

#endif /* !__ITEM_REF__H__INCL__ */


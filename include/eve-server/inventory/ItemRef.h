/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		Bloody.Rabbit
*/

#ifndef __ITEM_REF__H__INCL__
#define __ITEM_REF__H__INCL__

class InventoryItem;

class Blueprint;
class Ship;
class CelestialObject;
class Skill;
class Owner;

class Character;
class SolarSystem;
class Station;

/*
 * Typedefs for all item classes we have:
 */
typedef RefPtr<InventoryItem>            InventoryItemRef;
typedef RefPtr<const InventoryItem>      InventoryItemConstRef;

typedef RefPtr<Blueprint>                BlueprintRef;
typedef RefPtr<const Blueprint>          BlueprintConstRef;
typedef RefPtr<Ship>                     ShipRef;
typedef RefPtr<const Ship>               ShipConstRef;
typedef RefPtr<CelestialObject>          CelestialObjectRef;
typedef RefPtr<const CelestialObject>    CelestialObjectConstRef;
typedef RefPtr<Skill>                    SkillRef;
typedef RefPtr<const Skill>              SkillConstRef;
typedef RefPtr<Owner>                    OwnerRef;
typedef RefPtr<const Owner>              OwnerConstRef;

typedef RefPtr<Character>                CharacterRef;
typedef RefPtr<const Character>          CharacterConstRef;
typedef RefPtr<SolarSystem>              SolarSystemRef;
typedef RefPtr<const SolarSystem>        SolarSystemConstRef;
typedef RefPtr<Station>                  StationRef;
typedef RefPtr<const Station>            StationConstRef;

#endif /* !__ITEM_REF__H__INCL__ */


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
	Author:		Zhur
*/

#include "EvemuPCH.h"

NPC::NPC(
	SystemManager *system,
	PyServiceMgr *services,
	InventoryItem *self,
	uint32 corporationID,
	uint32 allianceID,
	const GPoint &position,
	SpawnEntry *spawner)
: DynamicSystemEntity(new DestinyManager(this, system), self),
  m_system(system),
  m_services(services),
  m_spawner(spawner),
//  m_itemID(self->itemID()),
//  m_typeID(self->typeID()),
//  m_ownerID(self->ownerID()),
  m_corporationID(corporationID),
  m_allianceID(allianceID),
  m_orbitingID(0)
{
	//NOTE: this is bad if we inherit NPC!
	m_AI = new NPCAIMgr(this);
	
	m_destiny->SetPosition(position, false);
}

NPC::~NPC() {
	//it is so dangerous to do this stuff in a destructor, with the
	//possibility of any of these things making virtual calls...
	// 
	// this makes inheriting NPC a bad idea (see constructor)
	m_system->RemoveNPC(this);
	if(m_spawner != NULL)
		m_spawner->SpawnDepoped(m_self->itemID());
	targets.DoDestruction();
	delete m_AI;
}

void NPC::Process() {
	SystemEntity::Process();
	m_AI->Process();
}

void NPC::Orbit(SystemEntity *who) {
	if(who == NULL) {
		m_orbitingID = 0;
	} else {
		m_orbitingID = who->GetID();
	}
}

void NPC::ForcedSetPosition(const GPoint &pt) {
	m_destiny->SetPosition(pt, false);
}

bool NPC::Load(ServiceDB *from) {
	//The old purpose for this was eliminated. But we might find
	//something else to stick in here eventually, so it stays for now.
	return true;
}

void NPC::TargetLost(SystemEntity *who) {
	m_AI->TargetLost(who);
}

void NPC::TargetedAdd(SystemEntity *who) {
	m_AI->Targeted(who);
}






















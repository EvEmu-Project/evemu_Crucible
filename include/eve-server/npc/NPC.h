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
	Author:		Zhur
*/


#ifndef __NPC_H_INCL__
#define __NPC_H_INCL__

#include "system/SystemEntity.h"

class PyServiceMgr;
class SpawnEntry;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;
class NPCAIMgr;

//Caution: do not inherit this, see constructor.
class NPC
: public DynamicSystemEntity {
public:
	NPC(
		SystemManager *system,
		PyServiceMgr &services,
		InventoryItemRef self,
		uint32 corporationID,
		uint32 allianceID,
		const GPoint &position,
		SpawnEntry *spawner = NULL);
	virtual ~NPC();

	bool Load(ServiceDB &from);

	void Orbit(SystemEntity *who);
	
	inline double x() const { return(GetPosition().x); }
	inline double y() const { return(GetPosition().y); }
	inline double z() const { return(GetPosition().z); }

	//SystemEntity interface:
	virtual EntityClass GetClass() const { return(ecNPC); }
	virtual bool IsNPC() const { return true; }
	virtual NPC *CastToNPC() { return(this); }
	virtual const NPC *CastToNPC() const { return(this); }
	virtual void Process();
	virtual void EncodeDestiny( Buffer& into ) const;
	virtual void TargetAdded(SystemEntity *who) {}
	virtual void TargetLost(SystemEntity *who);
	virtual void TargetedAdd(SystemEntity *who);
	virtual void TargetedLost(SystemEntity *who) {}
	virtual void TargetsCleared() {}
	virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
	virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}
	virtual uint32 GetCorporationID() const { return(m_corporationID); }
	virtual uint32 GetAllianceID() const { /* hack for now */ return(m_allianceID); }
	virtual void Killed(Damage &fatal_blow);
	virtual SystemManager *System() const { return(m_system); }
	
	void ForcedSetPosition(const GPoint &pt);
	

	virtual bool ApplyDamage(Damage &d);
	virtual void MakeDamageState(DoDestinyDamageState &into) const;

	void UseShieldRecharge();
	void UseArmorRepairer();
	
	/*
	
ATTR(248, entityLootValueMin, 0, int, false)
ATTR(249, entityLootValueMax, 0, int, false)
ATTR(250, entityLootCountMin, 0, int, false)
ATTR(251, entityLootCountMax, 0, int, false)
ATTR(252, entitySecurityStatusKillBonus, 0, int, false)
ATTR(253, entitySecurityStatusAggressionBonus, 0, int, false)
ATTR(254, minLootCount, 1, int, false)
ATTR(256, maxLootCount, 0, int, false)
ATTR(257, entityFollowRange, 0, int, false)
ATTR(258, minLootValue, 0, int, false)
ATTR(259, maxLootValue, 0, int, false)
ATTR(260, attackRange, 0, int, false)
ATTR(261, killStatusModifier, 0, int, false)
ATTR(262, attackStatusModifier, 0, int, false)
entityAttackDelayMin
entityAttackDelayMax
	*/
	

protected:
	void _AwardBounty(SystemEntity *who);
	void _DropLoot(SystemEntity *owner);

	SystemManager *const m_system;	//we do not own this
	PyServiceMgr &m_services;	//we do not own this
	SpawnEntry *const m_spawner;	//we do not own this, may be NULL
	uint32 m_corporationID;
	uint32 m_allianceID;

	uint32 m_orbitingID;

	NPCAIMgr *m_AI;	//never NULL


	/* Used to calculate the damages on NPCs
	 * I don't know why, npc->Set_shieldCharge does not work
	 * calling npc->shieldCharge() return the complete shield
	 * So we get the values on creation and use then instead.
	*/
	double m_shieldCharge;
	double m_armorDamage;
	double m_hullDamage;

	void _SendDamageStateChanged() const;

};

#endif




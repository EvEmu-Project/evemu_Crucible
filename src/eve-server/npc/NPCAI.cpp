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

#include "EVEServerPCH.h"

NPCAIMgr::NPCAIMgr(NPC *who)
: m_state(Idle),
  m_entityFlyRange2(who->Item()->GetAttribute(AttrEntityFlyRange)*who->Item()->GetAttribute(AttrEntityFlyRange)),
  m_entityChaseMaxDistance2(who->Item()->GetAttribute(AttrEntityChaseMaxDistance)*who->Item()->GetAttribute(AttrEntityChaseMaxDistance)),
  m_entityAttackRange2(who->Item()->GetAttribute(AttrEntityAttackRange)*who->Item()->GetAttribute(AttrEntityAttackRange)),
  m_npc(who),
  m_processTimer(50),	//arbitrary.
  m_mainAttackTimer(1),	//we want this to always trigger the first time through.
  m_shieldBoosterTimer(who->Item()->GetAttribute(AttrEntityShieldBoostDuration).get_int()),
  m_armorRepairTimer(who->Item()->GetAttribute(AttrEntityArmorRepairDuration).get_int())
{
	m_processTimer.Start();
	m_mainAttackTimer.Start();

	// This NPC uses Shield Booster
	if( who->Item()->GetAttribute(AttrEntityShieldBoostDuration) > 0 )
		m_shieldBoosterTimer.Start();
	// This NPC uses armor repairer
	if( who->Item()->GetAttribute(AttrEntityArmorRepairDuration) > 0 )
		m_armorRepairTimer.Start();
}
	
void NPCAIMgr::Process() {
	if(!m_processTimer.Check())
		return;

	// Test to see if we have a Shield Booster
	if( m_shieldBoosterTimer.Enabled() )
	{
		// It's time to recharge?
		if( m_shieldBoosterTimer.Check() )
		{
			m_npc->UseShieldRecharge();
		}
	}
	// Test to see if we have an Armor Repair
	if( m_armorRepairTimer.Enabled() )
	{
		// It's time to recharge?
		if( m_armorRepairTimer.Check() )
		{
			m_npc->UseArmorRepairer();
		}
	}

	switch(m_state) {
	case Idle:
		//TODO: wander around?
		//TODO: look around for shit to shoot at?
		// 		The parameter proximityRange tells us how far we "see"
		break;
		
	case Chasing: {
		//NOTE: getting our target like this is pretty weak...
		SystemEntity *target = m_npc->targets.GetFirstTarget(false);
		if(target == NULL) {
			//no valid target...
			if(m_npc->targets.HasNoTargets()) {
				_log(NPC__AI_TRACE, "[%u] Stopped chasing, no targets remain.", m_npc->GetID());
				m_state = Idle;
				return;
			}
			//else, still locking or something.
			return;
		}
		
        if(m_npc->DistanceTo2(target) < m_entityAttackRange2.get_float()) {
			//we caught up... off to follow mode. Should orbit, but that
			//isnt working yet.
			_log(NPC__AI_TRACE, "[%u] Was chasing %u, but they are close enough now. Following.", m_npc->GetID(), target->GetID());
			_EnterFollowing(target);
			return;
		}

		//else, we continue chasing... should we shoot?
		CheckAttacks(target);
	} break;
	
	case Following: {
		//NOTE: getting our target like this is pretty weak...
		SystemEntity *target = m_npc->targets.GetFirstTarget(false);
		if(target == NULL) {
			//no valid target...
			if(m_npc->targets.HasNoTargets()) {
				_log(NPC__AI_TRACE, "[%u] Stopped chasing, no targets remain.", m_npc->GetID());
				m_state = Idle;
				return;
			}
			//else, still locking or something.
			return;
		}
		
        if(m_npc->DistanceTo2(target) > m_entityChaseMaxDistance2.get_float()) {
			//they are too far away now...
			_log(NPC__AI_TRACE, "[%u] Was chasing with %u, but they are too far away now. Chasing.", m_npc->GetID(), target->GetID());
			_EnterChasing(target);
			return;
		}

		//ok, we are close enough...
		CheckAttacks(target);
	} break;
	
	case Engaged: {
		//NOTE: getting our target like this is pretty weak...
		SystemEntity *target = m_npc->targets.GetFirstTarget(false);
		if(target == NULL) {
			//no valid target...
			if(m_npc->targets.HasNoTargets()) {
				_log(NPC__AI_TRACE, "[%u] Stopped chasing, no targets remain.", m_npc->GetID());
				_EnterIdle();
				return;
			}
			//else, still locking or something.
			return;
		}
		
        if(m_npc->DistanceTo2(target) > m_entityAttackRange2.get_float()) {
			//they are too far away now...
			_log(NPC__AI_TRACE, "[%u] Was engaged with %u, but they are too far away now. Following.", m_npc->GetID(), target->GetID());
			_EnterFollowing(target);
			return;
		}

		//ok, we are close enough...
		CheckAttacks(target);
	} break;
	//no default on purpose
	}
}

void NPCAIMgr::_EnterChasing(SystemEntity *target) {
	//TODO: we actually use MWD if we have them...
	m_npc->Destiny()->Follow(target, m_npc->Item()->GetAttribute(AttrEntityFlyRange).get_float());
	m_state = Chasing;
}

void NPCAIMgr::_EnterFollowing(SystemEntity *target) {
	m_npc->Destiny()->Follow(target, m_npc->Item()->GetAttribute(AttrEntityFlyRange).get_float());
	m_state = Following;
}

void NPCAIMgr::_EnterIdle() {
	m_npc->Destiny()->Stop();
	m_state = Idle;
}

void NPCAIMgr::_EnterEngaged(SystemEntity *target) {
	//m_npc->Destiny()->Follow(target, m_npc->Item()->entityFlyRange());
	//not sure if we should use orbitRange or entityFlyRange...
	EvilNumber orbit_range = m_npc->Item()->GetAttribute(AttrOrbitRange);
	if(orbit_range > m_npc->Item()->GetAttribute(AttrEntityAttackRange)) {
		orbit_range = m_npc->Item()->GetAttribute(AttrEntityFlyRange);
	}
	m_npc->Destiny()->Orbit(target, orbit_range.get_float());
	m_state = Engaged;
}

void NPCAIMgr::Targeted(SystemEntity *by_who) {
	//TODO: determind lock speed.
	//TODO: obey maxLockedTargets
	//m_npc->targets.StartTargeting(by_who, 1000);
	if( m_npc->Item()->GetAttribute(AttrMaxLockedTargets) > m_npc->targets.GetTotalTargets())
		m_npc->targets.StartTargeting( by_who, 1000 );
	
	switch(m_state) {
	case Idle: {
		_log(NPC__AI_TRACE, "[%u] Targeted by %u in Idle. Attacking.", m_npc->GetID(), by_who->GetID());
		double dist = m_npc->DistanceTo2(by_who);
		if(dist < m_entityAttackRange2.get_float()) {
			_EnterEngaged(by_who);
		} else if(dist < m_entityChaseMaxDistance2.get_float()) {
			_EnterFollowing(by_who);
		} else {
			_EnterChasing(by_who);
		}
	} break;
	case Chasing:
		_log(NPC__AI_TRACE, "[%u] Targeted by %u while chasing.", m_npc->GetID(), by_who->GetID());
		break;
	case Following:
		_log(NPC__AI_TRACE, "[%u] Targeted by %u while following.", m_npc->GetID(), by_who->GetID());
		break;
	case Engaged:
		_log(NPC__AI_TRACE, "[%u] Targeted by %u while chasing.", m_npc->GetID(), by_who->GetID());
		break;
	//no default on purpose
	}
}

void NPCAIMgr::TargetLost(SystemEntity *by_who) {
	switch(m_state) {
	case Idle:
		//do nothing.
		break;
	case Chasing:
	case Following:
	case Engaged:
		if(m_npc->targets.HasNoTargets()) {
			_log(NPC__AI_TRACE, "[%u] Target of %u lost. No targets remain.", m_npc->GetID(), by_who->GetID());
			_EnterIdle();
		} else {
			_log(NPC__AI_TRACE, "[%u] Target of %u lost, but more targets remain.", m_npc->GetID(), by_who->GetID());
		}
		break;
	//no default on purpose
	}
}

void NPCAIMgr::CheckAttacks(SystemEntity *target) {
	if(m_mainAttackTimer.Check(false)) {
		_log(NPC__AI_TRACE, "[%u] Attack timer expired. Attacking %u.", m_npc->GetID(), target->GetID());
		
		InventoryItemRef self = m_npc->Item();

		//reset the attack timer.
		//NOTE: there is probably a more intelligent way to make this descision.
		//if(self->entityAttackDelayMax() <= 0) {
			//use speed field...
			m_mainAttackTimer.Start(self->GetAttribute(AttrSpeed).get_int());
		//} else {
			//I think this field is actually meant as a reaction time to the player showing up in range.
		//	m_mainAttackTimer.Start(MakeRandomInt(
		//		self->entityAttackDelayMin(), 
		//		self->entityAttackDelayMax() ));
		//}
		//Do main attack...

		//check our attack range...
		if(m_npc->DistanceTo2(target) > m_entityAttackRange2.get_float()) {
			_log(NPC__AI_TRACE, "[%u] Target (%u) is too far away (%.2f > %.2f)", m_npc->GetID(), target->GetID(), m_npc->DistanceTo2(target), m_entityAttackRange2.get_float());
			_EnterFollowing(target);
			return;
		}

		//TODO: check to-hit...
		
		//TODO: Need to consult dgmTypeEffects to determine what kind 
		// of effects to throw for this attack.
		_SendWeaponEffect("effects.Laser", target);
		
		Damage d(
			m_npc, (InventoryItemRef)self,
			effectTargetAttack);	//should get this from somewhere.
		m_npc->ApplyDamageModifiers(d, m_npc);
		
		target->ApplyDamage(d);
	}
}


//NOTE: duplicated from module manager code. They should share some day!
void NPCAIMgr::_SendWeaponEffect( const char*effect, SystemEntity *target )
{
	DoDestiny_OnSpecialFX13 sfx;
	sfx.entityID = m_npc->Item()->itemID();
	sfx.moduleID = m_npc->Item()->itemID();
	sfx.moduleTypeID = m_npc->Item()->typeID();
	sfx.targetID = target->GetID();
	sfx.otherTypeID = target->Item()->typeID();
	sfx.effect_type = effect;
	sfx.isOffensive = 1;
	sfx.start = 1;
	sfx.active = 1;
    
    //omit these for now, setting up the repeat might be a network optimization, but we don't need it right now.
	//sfx.duration_ms = 1960;	//no idea...
	//sfx.duration_ms = m_npc->Item()->speed();
    sfx.duration_ms = m_npc->Item()->GetAttribute(AttrSpeed).get_float();
	sfx.repeat = 1;
	sfx.startTime = Win32TimeNow();
	
	PyTuple* up = sfx.Encode();
	m_npc->Destiny()->SendSingleDestinyUpdate( &up );	//consumed
    PySafeDecRef( up );
}




















/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"

NPCAIMgr::NPCAIMgr(NPC *who)
: m_state(Idle),
  m_entityFlyRange2(who->Item()->entityFlyRange()*who->Item()->entityFlyRange()),
  m_entityChaseMaxDistance2(who->Item()->entityChaseMaxDistance()*who->Item()->entityChaseMaxDistance()),
  m_entityAttackRange2(who->Item()->entityAttackRange()*who->Item()->entityAttackRange()),
  m_npc(who),
  m_processTimer(50),	//arbitrary.
  m_mainAttackTimer(1)	//we want this to always trigger the first time through.
{
	m_processTimer.Start();
	m_mainAttackTimer.Start();
}
	
void NPCAIMgr::Process() {
	if(!m_processTimer.Check())
		return;
	
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
				_log(NPC__AI_TRACE, "[%lu] Stopped chasing, no targets remain.", m_npc->GetID());
				m_state = Idle;
				return;
			}
			//else, still locking or something.
			return;
		}
		
		if(m_npc->DistanceTo2(target) < m_entityAttackRange2) {
			//we caught up... off to follow mode. Should orbit, but that
			//isnt working yet.
			_log(NPC__AI_TRACE, "[%lu] Was chasing %lu, but they are close enough now. Following.", m_npc->GetID(), target->GetID());
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
				_log(NPC__AI_TRACE, "[%lu] Stopped chasing, no targets remain.", m_npc->GetID());
				m_state = Idle;
				return;
			}
			//else, still locking or something.
			return;
		}
		
		if(m_npc->DistanceTo2(target) > m_entityChaseMaxDistance2) {
			//they are too far away now...
			_log(NPC__AI_TRACE, "[%lu] Was chasing with %lu, but they are too far away now. Chasing.", m_npc->GetID(), target->GetID());
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
				_log(NPC__AI_TRACE, "[%lu] Stopped chasing, no targets remain.", m_npc->GetID());
				_EnterIdle();
				return;
			}
			//else, still locking or something.
			return;
		}
		
		if(m_npc->DistanceTo2(target) > m_entityAttackRange2) {
			//they are too far away now...
			_log(NPC__AI_TRACE, "[%lu] Was engaged with %lu, but they are too far away now. Following.", m_npc->GetID(), target->GetID());
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
	m_npc->Destiny()->Follow(target, m_npc->Item()->entityFlyRange());
	m_state = Chasing;
}

void NPCAIMgr::_EnterFollowing(SystemEntity *target) {
	m_npc->Destiny()->Follow(target, m_npc->Item()->entityFlyRange());
	m_state = Following;
}

void NPCAIMgr::_EnterIdle() {
	m_npc->Destiny()->Stop();
	m_state = Idle;
}

void NPCAIMgr::_EnterEngaged(SystemEntity *target) {
	//m_npc->Destiny()->Follow(target, m_npc->Item()->entityFlyRange());
	//not sure if we should use orbitRange or entityFlyRange...
	double orbit_range = m_npc->Item()->orbitRange();
	if(orbit_range > m_npc->Item()->entityAttackRange()) {
		orbit_range = m_npc->Item()->entityFlyRange();
	}
	m_npc->Destiny()->Orbit(target, orbit_range);
	m_state = Engaged;
}

void NPCAIMgr::Targeted(SystemEntity *by_who) {
	//TODO: determind lock speed.
	//TODO: obey maxLockedTargets
	m_npc->targets.StartTargeting(by_who, 1000);
	
	switch(m_state) {
	case Idle: {
		_log(NPC__AI_TRACE, "[%lu] Targeted by %lu in Idle. Attacking.", m_npc->GetID(), by_who->GetID());
		double dist = m_npc->DistanceTo2(by_who);
		if(dist < m_entityAttackRange2) {
			_EnterEngaged(by_who);
		} else if(dist < m_entityChaseMaxDistance2) {
			_EnterFollowing(by_who);
		} else {
			_EnterChasing(by_who);
		}
	} break;
	case Chasing:
		_log(NPC__AI_TRACE, "[%lu] Targeted by %lu while chasing.", m_npc->GetID(), by_who->GetID());
		break;
	case Following:
		_log(NPC__AI_TRACE, "[%lu] Targeted by %lu while following.", m_npc->GetID(), by_who->GetID());
		break;
	case Engaged:
		_log(NPC__AI_TRACE, "[%lu] Targeted by %lu while chasing.", m_npc->GetID(), by_who->GetID());
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
			_log(NPC__AI_TRACE, "[%lu] Target of %lu lost. No targets remain.", m_npc->GetID(), by_who->GetID());
			_EnterIdle();
		} else {
			_log(NPC__AI_TRACE, "[%lu] Target of %lu lost, but more targets remain.", m_npc->GetID(), by_who->GetID());
		}
		break;
	//no default on purpose
	}
}

void NPCAIMgr::CheckAttacks(SystemEntity *target) {
	if(m_mainAttackTimer.Check(false)) {
		_log(NPC__AI_TRACE, "[%lu] Attack timer expired. Attacking %lu.", m_npc->GetID(), target->GetID());
		
		InventoryItem *self = m_npc->Item();

		//reset the attack timer.
		//NOTE: there is probably a more intelligent way to make this descision.
		//if(self->entityAttackDelayMax() <= 0) {
			//use speed field...
			m_mainAttackTimer.Start(self->speed());
		//} else {
			//I think this field is actually meant as a reaction time to the player showing up in range.
		//	m_mainAttackTimer.Start(MakeRandomInt(
		//		self->entityAttackDelayMin(), 
		//		self->entityAttackDelayMax() ));
		//}
		//Do main attack...

		//check our attack range...
		if(m_npc->DistanceTo2(target) > m_entityAttackRange2) {
			_log(NPC__AI_TRACE, "[%lu] Target (%lu) is too far away (%.2f > %.2f)", m_npc->GetID(), target->GetID(), m_npc->DistanceTo2(target), m_entityAttackRange2);
			_EnterFollowing(target);
			return;
		}

		//TODO: check to-hit...
		
		//TODO: Need to consult dgmTypeEffects to determine what kind 
		// of effects to throw for this attack.
		_SendWeaponEffect("effects.Laser", target);
		
		Damage d(
			self, m_npc,
			effectTargetAttack);	//should get this from somewhere.
		m_npc->ApplyDamageModifiers(d, m_npc);
		
		target->ApplyDamage(d);
	}
}


//NOTE: duplicated from module manager code. They should share some day!
void NPCAIMgr::_SendWeaponEffect(const char *effect, SystemEntity *target) {
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
//omit these for now, setting up the repeat might be a network optimization, but we dont need it right now.
	sfx.duration_ms = 1960;	//no idea...
	sfx.repeat = 1;
	sfx.startTime = Win32TimeNow();
	
	PyRepTuple *up = sfx.FastEncode();
	m_npc->Destiny()->SendSingleDestinyUpdate(&up);	//consumed
	delete up;
}




















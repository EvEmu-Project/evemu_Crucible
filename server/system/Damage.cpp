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




Damage::Damage(
	double _kinetic,
	double _thermal,
	double _em,
	double _explosive,
	SystemEntity *_source,
	InventoryItem *_weapon,
	EVEEffectID _effect)
: kinetic(_kinetic),
  thermal(_thermal),
  em(_em),
  explosive(_explosive),
  source(_source),
  weapon(_weapon->Ref()),
  charge(NULL),
  effect(_effect)
{}

Damage::Damage(
	InventoryItem *_weapon,
	SystemEntity *_source,
	EVEEffectID _effect
	)
: kinetic(_weapon->kineticDamage()),
  thermal(_weapon->thermalDamage()),
  em(_weapon->emDamage()),
  explosive(_weapon->explosiveDamage()),
  source(_source),
  weapon(_weapon->Ref()),
  charge(NULL),
  effect(_effect)
{}

Damage::Damage(
	InventoryItem *_weapon,
	InventoryItem *_charge,
	SystemEntity *_source,
	EVEEffectID _effect
	)
: kinetic(_charge->kineticDamage() * _weapon->damageMultiplier()),
  thermal(_charge->thermalDamage() * _weapon->damageMultiplier()),
  em(_charge->emDamage() * _weapon->damageMultiplier()),
  explosive(_charge->explosiveDamage() * _weapon->damageMultiplier()),
  source(_source),
  weapon(_weapon->Ref()),
  charge(_charge->Ref()),
  effect(_effect)
{}

Damage::~Damage() {
	weapon->Release();
	if(charge != NULL)
		charge->Release();
}






void Client::ApplyDamageModifiers(Damage &d, SystemEntity *target) {
	
}

void Client::_ReduceDamage(Damage &d) {
	// armorEmDamageResonance
	// armorExplosiveDamageResonance
	// armorKineticDamageResonance
	// armorThermalDamageResonance
	// shieldEmDamageResonance
	// shieldExplosiveDamageResonance
	// shieldKineticDamageResonance
	// shieldThermalDamageResonance
}

static const char *DamageMessageIDs_Self[6] = {
	"AttackHit1R",	//barely scratches
	"AttackHit2R",	//lightly hits
	"AttackHit3R",	//hits
	"AttackHit4R",	//aims well at you
	"AttackHit5R",	//places an excellent hit
	"AttackHit6R"	//strikes you  perfectly, wrecking
};

static const char *DamageMessageIDs_SelfNamed[6] = {
	"AttackHit1RD",	//barely scratches
	"AttackHit2RD",	//lightly hits
	"AttackHit3RD",	//hits
	"AttackHit4RD",	//aims well at you
	"AttackHit5RD",	//places an excellent hit
	"AttackHit6RD"	//strikes you  perfectly, wrecking
};

static const char *DamageMessageIDs_Other[6] = {
	"AttackHit1",	//barely scratches
	"AttackHit2",	//lightly hits
	"AttackHit3",	//hits
	"AttackHit4",	//aims well at you
	"AttackHit5",	//places an excellent hit
	"AttackHit6"	//strikes you  perfectly, wrecking
};


//the notifications which this puts out prolly needs some work.
void Client::ApplyDamage(Damage &d) {
	_ReduceDamage(d);
	
	//shieldUniformity
	//structureUniformity
	
	double damage = d.GetTotal();
	
	Notify_OnEffectHit noeh;
	noeh.itemID = d.source->GetID();
	noeh.effectID = d.effect;
	noeh.targetID = GetID();
	noeh.damage = damage;

	PyRepTuple *up;
	
	//Notifications to ourself:
		up = noeh.Encode();
		QueueDestinyEvent(&up);
		delete up;
		
	
		//NOTE: could send out the RD version of this message instead of the R version, which
		//includes "weapon" and "owner" instead of "source".
		Notify_OnDamageMessage_Self ondam;
		ondam.messageID = "AttackHit2R";	//TODO: randomize/select this somehow.
		ondam.damage = damage;
		ondam.source = d.source->GetID();
		ondam.splash = "";
		up = ondam.FastEncode();
		QueueDestinyEvent(&up);
		delete up;

	//Notifications to others:
		//I am not sure what the correct scope of this broadcast
		//should be. For now, it goes to anybody targeting us.
		if(targets.IsTargetedBySomething()) {
			up = noeh.Encode();
			targets.QueueTBDestinyEvent(&up);
			delete up;
			
			Notify_OnDamageMessage_Other ondamo;
			ondamo.messageID = "AttackHit3";		//TODO: randomize/select this somehow.
			ondamo.format_type = fmtMapping_itemTypeName;
			ondamo.weaponType = d.weapon->typeID();
			ondamo.damage = damage;
			ondamo.target = GetID();
			ondamo.splash = "";
			up = ondamo.FastEncode();
			targets.QueueTBDestinyEvent(&up);
			delete up;
		}
}

//default implementation bases everything directly on our item.
void ItemSystemEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target) {
	//m_self->damageMultiplier()
	//these are straight additives to the damage.
	//emDamageBonus
	//explosiveDamageBonus
	//kineticDamageBonus
	//thermalDamageBonus
	
}

//default implementation bases everything directly on our item.
void ItemSystemEntity::ApplyDamage(Damage &d) {
	
	_log(ITEM__TRACE, "%lu: Applying %.1f total damage from %lu", GetID(), d.GetTotal(), d.source->GetID());
	
	double total_damage = 0;
	bool killed = false;
	
	//apply resistances...
	//damageResistance?
	
	//Shield:
	double available_shield = m_self->shieldCharge();
	Damage shield_damage = 
		d.MultiplyDup(
		 		  m_self->shieldKineticDamageResonance(),
				  m_self->shieldThermalDamageResonance(),
				  m_self->shieldEmDamageResonance(),
					m_self->shieldExplosiveDamageResonance() );
	//other:
	//emDamageResistanceBonus
	//explosiveDamageResistanceBonus
	//kineticDamageResistanceBonus
	//thermalDamageResistanceBonus
	// 
	
	//TODO: deal with seepage from shield into armor.
	//shieldUniformity
	//uniformity (chance of seeping through to armor)
	
	double total_shield_damage = shield_damage.GetTotal();
	if(total_shield_damage <= available_shield) {
		//we can take all this damage with our shield...
		double new_charge = m_self->shieldCharge() - total_shield_damage;
		m_self->Set_shieldCharge(new_charge);
		
		total_damage += total_shield_damage;
		_log(ITEM__TRACE, "%lu: Applying entire %.1f damage to shields. New charge: %.1f", GetID(), total_shield_damage, new_charge);
	} else {
		//first determine how much we can actually apply to 
		//the shield, the rest goes further down.
		double consumed_shield_ratio = available_shield / shield_damage.GetTotal();
		d *= 1.0 - consumed_shield_ratio;
		if(available_shield > 0) {
			total_damage += available_shield;
			
			_log(ITEM__TRACE, "%lu: Shield depleated with %.1f damage. %.1f damage remains.", GetID(), available_shield, d.GetTotal());
			
			//set shield to 0, it is fully depleated.
			m_self->Set_shieldCharge(0);
		}
		
		//Armor:
		double available_armor = m_self->armorHP() - m_self->armorDamage();
		Damage armor_damage = 
			d.MultiplyDup(
						m_self->armorKineticDamageResonance(),
						m_self->armorThermalDamageResonance(),
						m_self->armorEmDamageResonance(),
						m_self->armorExplosiveDamageResonance() );
		//other:
		//activeEmResistanceBonus
		//activeExplosiveResistanceBonus
		//activeThermicResistanceBonus
		//activeKineticResistanceBonus
		//passiveEmDamageResistanceBonus
		//passiveExplosiveDamageResistanceBonus
		//passiveKineticDamageResistanceBonus
		//passiveThermicDamageResistanceBonus

		//TODO: figure out how much passes through to structure/modules.
		//armorUniformity
		
		double total_armor_damage = armor_damage.GetTotal();
		if(total_armor_damage <= available_armor) {
			//we can take all this damage with our armor...
			double new_damage = m_self->armorDamage() + total_armor_damage;
			m_self->Set_armorDamage(new_damage);
			
			total_damage += total_armor_damage;
			_log(ITEM__TRACE, "%lu: Applying entire %.1f damage to armor. New armor damage: %.1f", GetID(), total_armor_damage, new_damage);
		} else {
			//first determine how much we can actually apply to 
			//the armor, the rest goes further down.
			double consumed_armor_ratio = available_armor / armor_damage.GetTotal();
			d *= 1.0 - consumed_armor_ratio;

			if(available_armor > 0) {
				total_damage += available_armor;
				
				_log(ITEM__TRACE, "%lu: Armor depleated with %.1f damage. %.1f damage remains.", GetID(), available_armor, d.GetTotal());
				
				//all armor has been penetrated.
				m_self->Set_armorDamage(m_self->armorHP());
			}
			
			
			//Hull/Structure:
			
			//The base hp and damage attributes represent structure.
			double available_hull = m_self->hp() - m_self->damage();
			Damage hull_damage = 
				d.MultiplyDup(
						m_self->hullKineticDamageResonance(),
						m_self->hullThermalDamageResonance(),
						m_self->hullEmDamageResonance(),
						m_self->hullExplosiveDamageResonance() );
			//other:
			//passiveEmDamageResonanceMultiplier
			//passiveThermalDamageResonanceMultiplier
			//passiveKineticDamageResonanceMultiplier
			//passiveExplosiveDamageResonanceMultiplier
			//activeEmDamageResonance
			//activeThermalDamageResonance
			//activeKineticDamageResonance
			//activeExplosiveDamageResonance
			//structureUniformity
			double total_hull_damage = hull_damage.GetTotal();
			total_damage += total_hull_damage;
			if(total_hull_damage < available_hull) {
				//we can take all this damage with our hull...
				double new_damage = m_self->damage() + total_hull_damage;
				m_self->Set_damage(new_damage);
				_log(ITEM__TRACE, "%lu: Applying entire %.1f damage to structure. New structure damage: %.1f", GetID(), total_hull_damage, new_damage);
			} else {
				//dead....
				_log(ITEM__TRACE, "%lu: %.1f damage has depleated our structure. Time to explode.", GetID(), total_hull_damage);
				killed = true;
				m_self->Set_damage(m_self->hp());
			}
			
			//TODO: deal with damaging modules. no idea the mechanics on this.
		}
	}
	
	if(total_damage <= 0.0)
		return;
	
	//Notifications to others:
	//I am not sure what the correct scope of this broadcast
	//should be. For now, it goes to anybody targeting us.
	if(targets.IsTargetedBySomething()) {
		PyRepTuple *up;
		Notify_OnEffectHit noeh;
		noeh.itemID = d.source->GetID();
		noeh.effectID = d.effect;
		noeh.targetID = GetID();
		noeh.damage = total_damage;
		up = noeh.Encode();
		targets.QueueTBDestinyEvent(&up);
		delete up;
		
		Notify_OnDamageMessage_Other ondam;
		ondam.messageID = "AttackHit3";		//TODO: select this based on the severity of the hit...
		ondam.format_type = fmtMapping_itemTypeName;
		ondam.weaponType = d.weapon->typeID();
		ondam.damage = total_damage;
		ondam.target = GetID();
		ondam.splash = "";
		up = ondam.FastEncode();
		targets.QueueTBDestinyEvent(&up);
		delete up;
	}
	
	if(killed == true) {
		Killed(d);
	} else {
		_SendDamageStateChanged();
	}
}

void ItemSystemEntity::_SendDamageStateChanged() const {
	//armorHP
	/*double damage = m_self->hp();
	if(damage > hp) {
		blah
	}
	damage = damage + delta;
	m_self->Set_hp(hp);*/
	
	DoDestinyDamageState state;
	MakeDamageState(state);
	
	DoDestiny_OnDamageStateChange ddsc;
	ddsc.entityID = GetID();
	ddsc.state = state.FastEncode();
	
	PyRepTuple *up;
	up = ddsc.FastEncode();
	targets.QueueTBDestinyUpdate(&up);
}

void SystemEntity::Killed(Damage &fatal_blow) {
	targets.ClearAllTargets(false);	//I assume a client does not need this notification.
}


void DynamicSystemEntity::Killed(Damage &fatal_blow) {
	ItemSystemEntity::Killed(fatal_blow);
	//NOTE: Client::Killed intentionally skips over this function in order to
	//handle the destiny stuff more carefully.
	if(m_destiny != NULL) {
		m_destiny->SendTerminalExplosion();
		m_destiny->SendRemoveBall();
	}
}


void Client::Killed(Damage &fatal_blow) {
	//we intentionally skip over dynamic system entity because we 
	ItemSystemEntity::Killed(fatal_blow);
	
	if(m_ship->typeID() == itemTypeCapsule) {
		//we have been pod killed... off we go.
		
		m_destiny->SendTerminalExplosion();	//I think they actually delay this one a second.
		
		//TODO: destroy all implants
		
		//TODO: send them back to their clone.
		m_system->RemoveClient(this);
	} else {
		//our ship has been destroyed. Off to our capsule.
		//We are currently not keeping our real capsule around in the DB, so we need to make a new one.
		
		std::string capsule_name = m_char.name + "'s Capsule";
		InventoryItem *capsule = m_services->item_factory->SpawnSingleton(
																		 itemTypeCapsule,
																		 m_char.charid,
																		 m_char.stationID,
																		 flagCapsule,
																		 capsule_name.c_str());
		if(capsule == NULL) {
			codelog(CLIENT__ERROR, "Failed to create capsule for character '%s'", m_char.name.c_str());
			//what to do?
			return;
		}
		
		InventoryItem *dead_ship = m_ship->Ref();	//grab a ship ref to ensure that nobody else nukes it first.
		
		//ok, nothing can fail now, we need have our capsule, make the transition.
		
		//put the capsule where the ship was
		capsule->Relocate(m_ship->position());
		
		//this updates m_self and manages destiny updates as needed.
		//This sends the RemoveBall for the old ship.
		BoardShip(capsule);
		
		m_destiny->SendTerminalExplosion();	//I think they actually delay this one a second.
		
		//and off we go into our new ship.
		capsule->Release();	//we are done with our instance, others may have kept one for themself.
		
		//kill off the old ship.
		//TODO: figure out anybody else which may be referencing this ship...
		dead_ship->Delete();	//remove from the DB.
		//dead_ship->Release();	//delete does our release for us.
	}
}

void NPC::Killed(Damage &fatal_blow) {
	DynamicSystemEntity::Killed(fatal_blow);
	
	//TODO: drop loot.
	_DropLoot(fatal_blow.source);
	
	//TODO: award status changes. (entitySecurityStatusKillBonus)

   //notify our spawner that we are gone.
   if(m_spawner != NULL) {
      m_spawner->SpawnDepoped(m_self->itemID());
   }
	
	//award kill bounty.
	_AwardBounty(fatal_blow.source);
	
	m_system->RemoveNPC(this);
}

void NPC::_DropLoot(SystemEntity *owner) {

	//entityLootValueMin
	//entityLootValueMax
	//entityLootCountMin
	//entityLootCountMax
	//minLootCount
	//maxLootCount
	//minLootValue
	//maxLootValue
	
	// Send an OnSpecialFX (9) for effects.Jettison (with can's ID, not npc)
}

void NPC::_AwardBounty(SystemEntity *who) {
	double bounty = m_self->entityKillBounty();
	if(bounty <= 0) {
		return;	//no bounty to award...
	}
	
	//TODO: handle case where drone strikes fatal blow... bounty goes to master.

	//TODO: handle distribution to gangs.
	
	if(who->IsClient() == false) {
		_log(NPC__TRACE, "Refusing to award bounty on %lu to non-client %lu", GetID(), who->GetID());
		return;	//bounty dosent make sense for anything other than clients.
	}

	Client *killer = who->CastToClient();

	killer->AddBalance(bounty);

	std::string reason = "Bounty";	//TODO: improve this.
	
	if(!m_services->GetServiceDB()->GiveCash(
			killer->GetID(),
			RefType_playerDonation,	//TODO: find the proper type
			m_self->itemID(),	//prolly actually a special concord item ID or something.
			killer->GetID(),
			"",	//unknown const char *argID1,
			killer->GetAccountID(),
			accountCash,
			bounty,
			killer->GetBalance(),
			reason.c_str()
	)) {
		codelog(CLIENT__ERROR, "%s: Failed to record bountry of %f from death of %lu (type %lu)", killer->GetName(), bounty, GetID(), m_self->typeID());
		//well.. this isnt a huge deal, so we will get over it.
	}
	
}




















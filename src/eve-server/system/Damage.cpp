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
    Author:        Zhur
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "PyServiceMgr.h"
#include "corporation/CorporationDB.h"
#include "inventory/AttributeEnum.h"
#include "mining/Asteroid.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/SpawnManager.h"
#include "pos/Structure.h"
#include "ship/DestinyManager.h"
#include "ship/Drone.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Celestial.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/Deployable.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/WrecksAndLoot.h"

Damage::Damage(
    SystemEntity *_source,
    InventoryItemRef _weapon,
    double _kinetic,
    double _thermal,
    double _em,
    double _explosive,
    EVEEffectID _effect): source(_source), charge(), effect(_effect)
{
	kinetic = _kinetic;
	thermal = _thermal;
	em = _em;
	explosive = _explosive;
	weapon = _weapon;
}

Damage::Damage(
    SystemEntity *_source,
    bool fatal_blow): source(_source), effect(effectTargetAttack)
{
	assert(fatal_blow && "Damage() constructor meant for fatal_blow called without 2nd param being true!");

	// No specific damage dealt here, just killed
	kinetic = 0.0;
	thermal = 0.0;
	em = 0.0;
	explosive = 0.0;

	// These are set to NULL for this specific case of Damage obj meant for Killed() methods of derived SystemEntity objects
	weapon = InventoryItemRef();
	charge = InventoryItemRef();
}

Damage::Damage(
    SystemEntity *_source,
    InventoryItemRef _weapon,
    EVEEffectID _effect): source(_source), charge(), effect(_effect)
{
	if(_weapon->HasAttribute(AttrKineticDamage))
		kinetic = _weapon->GetAttribute(AttrKineticDamage).get_float();
	else
		kinetic = 0.0;

	if(_weapon->HasAttribute(AttrThermalDamage))
		thermal = _weapon->GetAttribute(AttrThermalDamage).get_float();
	else
		thermal = 0.0;

	if(_weapon->HasAttribute(AttrEmDamage))
		em = _weapon->GetAttribute(AttrEmDamage).get_float();
	else
		em = 0.0;

	if(_weapon->HasAttribute(AttrExplosiveDamage))
		explosive = _weapon->GetAttribute(AttrExplosiveDamage).get_float();
	else
		explosive = 0.0;

	weapon = _weapon;
}

Damage::Damage(
    SystemEntity *_source,
    InventoryItemRef _weapon,
    InventoryItemRef _charge,
    EVEEffectID _effect): source(_source), charge(_charge), effect(_effect)
{
	if(_charge->HasAttribute(AttrKineticDamage))
		kinetic = (_charge->GetAttribute(AttrKineticDamage) * _weapon->GetAttribute(AttrDamageMultiplier)).get_float();
	else
		kinetic = 0.0;

	if(_charge->HasAttribute(AttrThermalDamage))
		thermal = (_charge->GetAttribute(AttrThermalDamage) * _weapon->GetAttribute(AttrDamageMultiplier)).get_float();
	else
		thermal = 0.0;

	if(_charge->HasAttribute(AttrEmDamage))
		em = (_charge->GetAttribute(AttrEmDamage) * _weapon->GetAttribute(AttrDamageMultiplier)).get_float();
	else
		em = 0.0;

	if(_charge->HasAttribute(AttrExplosiveDamage))
		explosive = (_charge->GetAttribute(AttrExplosiveDamage) * _weapon->GetAttribute(AttrDamageMultiplier)).get_float();
	else
		explosive = 0.0;
	
	weapon = _weapon;

}

Damage::~Damage()
{
}

static const char *DamageMessageIDs_Self[6] = {
    "AttackHit1R",    //barely scratches
    "AttackHit2R",    //lightly hits
    "AttackHit3R",    //hits
    "AttackHit4R",    //aims well at you
    "AttackHit5R",    //places an excellent hit
    "AttackHit6R"    //strikes you  perfectly, wrecking
};

static const char *DamageMessageIDs_SelfNamed[6] = {
    "AttackHit1RD",    //barely scratches
    "AttackHit2RD",    //lightly hits
    "AttackHit3RD",    //hits
    "AttackHit4RD",    //aims well at you
    "AttackHit5RD",    //places an excellent hit
    "AttackHit6RD"    //strikes you  perfectly, wrecking
};

static const char *DamageMessageIDs_Other[6] = {
    "AttackHit1",    //barely scratches
    "AttackHit2",    //lightly hits
    "AttackHit3",    //hits
    "AttackHit4",    //aims well at you
    "AttackHit5",    //places an excellent hit
    "AttackHit6"    //strikes you  perfectly, wrecking
};


//default implementation bases everything directly on our item.
void ItemSystemEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target) {
    //m_self->damageMultiplier()

	// Basic damage multiplier
	if( target->Item()->HasAttribute(AttrDamageMultiplier) )
	{
		double damageMultiplier = target->Item()->GetAttribute(AttrDamageMultiplier).get_float();
		d.SumWithMultFactor(damageMultiplier);
	}

    //these are straight additives to the damage.
    //emDamageBonus
    //explosiveDamageBonus
    //kineticDamageBonus
    //thermalDamageBonus

}

//default implementation bases everything directly on our item.
//the notifications which this puts out probably needs some work.
bool ItemSystemEntity::ApplyDamage(Damage &d) {
    _log(ITEM__TRACE, "%s(%u): Applying %.1f total damage from %u", GetName(), GetID(), d.GetTotal(), d.source->GetID());

    double total_damage = 0;
    bool killed = false;
    int random_damage = 0;
    double random_damage_mult = 1.0;


    //apply resistances...
    //damageResistance?

    //Shield:
    /*double available_shield = m_self->shieldCharge();
    Damage shield_damage = d.MultiplyDup(
        m_self->shieldKineticDamageResonance(),
        m_self->shieldThermalDamageResonance(),
        m_self->shieldEmDamageResonance(),
        m_self->shieldExplosiveDamageResonance()
    );*/


    double available_shield = m_self->GetAttribute(AttrShieldCharge).get_float();
    Damage shield_damage = d.MultiplyDup(
        m_self->GetAttribute(AttrShieldKineticDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldThermalDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldEmDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldExplosiveDamageResonance).get_float()
        );


    //other:
    //emDamageResistanceBonus
    //explosiveDamageResistanceBonus
    //kineticDamageResistanceBonus
    //thermalDamageResistanceBonus
    //

    //TODO: deal with seepage from shield into armor.
    //shieldUniformity
    //uniformity (chance of seeping through to armor)

    // Make a random value to use in msg's and attack multiplier
    random_damage = static_cast<int32>(MakeRandomInt(0, 5));
    random_damage_mult = (double)(random_damage / 10.0);    // chance from 0 to 50% more damage

    /*
     * Here we calculates the uniformity thing.
     * I think this must be calculated based on
     * the type of damage -> resistance basis.
    *
    double shield_uniformity = available_shield / m_self->shieldCapacity();
    if( shield_uniformity < ( 1.0  - m_self->shieldUniformity() ) )
    {
        /*
         * As far i can see mostly npc/entities have a
         * chance of transpassing when the shield is below 25%
        /
    }
    //*/

    // Not sure about this, but with this we get some random hits... :)
    shield_damage.SumWithMultFactor( random_damage_mult );
    double total_shield_damage = shield_damage.GetTotal();

    if(total_shield_damage <= available_shield)
    {
        //we can take all this damage with our shield...
        //double new_charge = m_self->shieldCharge() - total_shield_damage;
        //m_self->Set_shieldCharge(new_charge);
        EvilNumber new_charge = m_self->GetAttribute(AttrShieldCharge) - EvilNumber(total_shield_damage);
        m_self->SetAttribute(AttrShieldCharge, new_charge);

        total_damage += total_shield_damage;
        _log(ITEM__TRACE, "%s(%u): Applying entire %.1f damage to shields. New charge: %.1f", GetName(), GetID(), total_shield_damage, new_charge.get_float());
    }
    else
    {
        //first determine how much we can actually apply to
        //the shield, the rest goes further down.
        double consumed_shield_ratio = available_shield / shield_damage.GetTotal();
        d *= 1.0 - consumed_shield_ratio;
        if(available_shield > 0) {
            total_damage += available_shield;

            _log(ITEM__TRACE, "%s(%u): Shield depleated with %.1f damage. %.1f damage remains.", GetName(), GetID(), available_shield, d.GetTotal());

            //set shield to 0, it is fully depleted.
            m_self->SetAttribute(AttrShieldCharge, 0);
        }

        //Armor:
        double available_armor = m_self->GetAttribute(AttrArmorHP).get_float() - m_self->GetAttribute(AttrArmorDamage).get_float();
        Damage armor_damage = d.MultiplyDup(
            m_self->GetAttribute(AttrArmorKineticDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorThermalDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorEmDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorExplosiveDamageResonance).get_float()
        );
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

        // Not sure about this, but with this we get some random hits... :)
        armor_damage.SumWithMultFactor( random_damage_mult );
        double total_armor_damage = armor_damage.GetTotal();
        if(total_armor_damage <= available_armor)
        {
            //we can take all this damage with our armor...
            //double new_damage = m_self->armorDamage() + total_armor_damage;
            //m_self->Set_armorDamage(new_damage);

            EvilNumber new_damage = m_self->GetAttribute(AttrArmorDamage) + EvilNumber(total_armor_damage);
            m_self->SetAttribute(AttrArmorDamage, new_damage);

            total_damage += total_armor_damage;
            _log(ITEM__TRACE, "%s(%u): Applying entire %.1f damage to armor. New armor damage: %.1f", GetName(), GetID(), total_armor_damage, new_damage.get_float());
        }
        else
        {
            //first determine how much we can actually apply to
            //the armor, the rest goes further down.
            double consumed_armor_ratio = available_armor / armor_damage.GetTotal();
            d *= 1.0 - consumed_armor_ratio;

            if(available_armor > 0)
            {
                total_damage += available_armor;

                _log(ITEM__TRACE, "%s(%u): Armor depleated with %.1f damage. %.1f damage remains.", GetName(), GetID(), available_armor, d.GetTotal());

                //all armor has been penetrated.
                m_self->SetAttribute(AttrArmorDamage, m_self->GetAttribute(AttrArmorHP));
            }

            //Hull/Structure:

            //The base hp and damage attributes represent structure.
            double available_hull = m_self->GetAttribute(AttrHp).get_float() - m_self->GetAttribute(AttrDamage).get_float();
            Damage hull_damage = d.MultiplyDup(
                m_self->GetAttribute(AttrHullKineticDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullThermalDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullEmDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullExplosiveDamageResonance).get_float()
            );
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

			/*	This is already passed on the constructor of Damage
            // If we have a passive or an active module to boost the resistance.
            // The modules itself must provide us this value.
            //hull_damage.em *= m_self->GetAttribute(AttrHullEmDamageResonance).get_float();
            //hull_damage.explosive *= m_self->GetAttribute(AttrHullExplosiveDamageResonance).get_float();
            //hull_damage.kinetic *= m_self->GetAttribute(AttrHullKineticDamageResonance).get_float();
            //hull_damage.thermal *= m_self->GetAttribute(AttrHullThermalDamageResonance).get_float();
			*/
            // Not sure about this, but with this we get some random hits... :)
            hull_damage.SumWithMultFactor( random_damage_mult );
            double total_hull_damage = hull_damage.GetTotal();
            total_damage += total_hull_damage;
            if(total_hull_damage < available_hull)
            {
                //we can take all this damage with our hull...
                /*double new_damage = m_self->damage() + total_hull_damage;
                m_self->Set_damage(new_damage);*/
                EvilNumber new_damage = m_self->GetAttribute(AttrDamage) + EvilNumber(total_hull_damage);
                m_self->SetAttribute(AttrDamage, new_damage);
                _log(ITEM__TRACE, "%s(%u): Applying entire %.1f damage to structure. New structure damage: %.1f", GetName(), GetID(), total_hull_damage, new_damage.get_float());
            }
            else
            {
                //dead....
                _log(ITEM__TRACE, "%s(%u): %.1f damage has depleated our structure. Time to explode.", GetName(), GetID(), total_hull_damage);
                killed = true;
                //m_self->Set_damage(m_self->hp());
                m_self->SetAttribute(AttrDamage, m_self->GetAttribute(AttrDamage));
            }

            //TODO: deal with damaging modules. no idea the mechanics on this.
        }
    }

    if( total_damage <= 0.0 )
        return(killed);

    PyTuple *up;

    //Notifications to ourself:
    Notify_OnEffectHit noeh;
    noeh.itemID = d.source->GetID();
    noeh.effectID = d.effect;
    noeh.targetID = GetID();
    noeh.damage = total_damage;

    up = noeh.Encode();
    QueueDestinyEvent(&up);
    PySafeDecRef( up );

    //NOTE: could send out the RD version of this message instead of the R version, which
    //includes "weapon" and "owner" instead of "source".
    Notify_OnDamageMessage_Self ondam;
    ondam.messageID = DamageMessageIDs_Other[random_damage];	//DamageMessageIDs_Self[random_damage];
    ondam.damage = total_damage;
    ondam.source = d.source->GetID();
    ondam.splash = "";
    up = ondam.Encode();
    QueueDestinyEvent(&up);
    PySafeDecRef( up );

    //Notifications to others:
    //I am not sure what the correct scope of this broadcast
    //should be. For now, it goes to anybody targeting us.
    if(targets.IsTargetedBySomething()) {
        up = noeh.Encode();
        targets.QueueTBDestinyEvent(&up);
        PySafeDecRef( up );

        Notify_OnDamageMessage_Other ondamo;
        ondamo.messageID = DamageMessageIDs_Other[random_damage];
        ondamo.format_type = fmtMapping_itemTypeName;
        ondamo.weaponType = d.weapon->typeID();
        ondamo.damage = total_damage;
        ondamo.target = GetID();
        ondamo.splash = "";

        up = ondamo.Encode();
        targets.QueueTBDestinyEvent(&up);
        PySafeDecRef( up );
    }

    if(killed == true)
    {
        Killed(d);
    }
    else
    {
        _SendDamageStateChanged();
    }

    return(killed);
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
	sLog.Warning("Client::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void Client::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// for now this uses ItemSystemEntity
bool Client::ApplyDamage(Damage &d) {
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

// This is a NPC implementation of damage system (incomplete)
bool NPC::ApplyDamage(Damage &d) {
    _log(ITEM__TRACE, "%u: Applying %.1f total damage from %u", GetID(), d.GetTotal(), d.source->GetID());

    double total_damage = 0;
    bool killed = false;
    int random_damage = 0;
    double random_damage_mult = 1.0;

    //apply resistances...
    //damageResistance?

    //Shield:
    double available_shield = m_shieldCharge;
    Damage shield_damage = d.MultiplyDup(
        m_self->GetAttribute(AttrShieldKineticDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldThermalDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldEmDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldExplosiveDamageResonance).get_float()
    );
    //other:
    //emDamageResistanceBonus
    //explosiveDamageResistanceBonus
    //kineticDamageResistanceBonus
    //thermalDamageResistanceBonus
    //



    //TODO: deal with seepage from shield into armor.
     //shieldUniformity
    //uniformity (chance of seeping through to armor)

    /*
     * Here we calculates the uniformity thing.
     * I think this must be calculated based on
     * the type of damage -> resistance basis.
    *
    double shield_uniformity = available_shield / m_self->shieldCapacity();
    if( shield_uniformity < ( 1.0  - m_self->shieldUniformity() ) )
    {
        /*
         * As far i can see mostly npc/entities have a
         * chance of transpassing when the shield is below 25%
        /
    }
    */

    // Make a random value to use in msg's and attack multiplier
    random_damage = static_cast<int32>(MakeRandomInt(0, 5));
    random_damage_mult = (double)(random_damage / 10.0);

    // Not sure about this, but with this we get some random hits... :)
    //total_shield_damage += total_shield_damage * random_damage_mult;
    shield_damage.SumWithMultFactor( random_damage_mult );
    double total_shield_damage = shield_damage.GetTotal();

    if(total_shield_damage <= available_shield)
    {
        //we can take all this damage with our shield...
        double new_charge = m_shieldCharge - total_shield_damage;

        m_shieldCharge = new_charge;
        total_damage += total_shield_damage;
        _log(ITEM__TRACE, "%s(%u): Applying entire %.1f damage to shields. New charge: %.1f", GetName(), GetID(), total_shield_damage, new_charge);
    }
    else
    {
        //first determine how much we can actually apply to
        //the shield, the rest goes further down.
        double consumed_shield_ratio = available_shield / shield_damage.GetTotal();
        d *= 1.0 - consumed_shield_ratio;
        if(available_shield > 0) {
            total_damage += available_shield;

            _log(ITEM__TRACE, "%s(%us): Shield depleated with %.1f damage. %.1f damage remains.", GetName(), GetID(), available_shield, d.GetTotal());

            //set shield to 0, it is fully depleated.
            m_shieldCharge = 0;
        }

        //Armor:
        double available_armor = m_self->GetAttribute(AttrArmorHP).get_float() - m_armorDamage;
        Damage armor_damage = d.MultiplyDup(
            m_self->GetAttribute(AttrArmorKineticDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorThermalDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorEmDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorExplosiveDamageResonance).get_float()
        );
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

        // Not sure about this, but with this we get some random hits... :)
        //total_armor_damage += total_armor_damage * random_damage_mult;
        armor_damage.SumWithMultFactor( random_damage_mult );
        double total_armor_damage = armor_damage.GetTotal();

        if(total_armor_damage <= available_armor)
        {
            //we can take all this damage with our armor...
            double new_damage = m_armorDamage + total_armor_damage;
            m_armorDamage = new_damage;

            total_damage += total_armor_damage;
            _log(ITEM__TRACE, "%s(%u): Applying entire %.1f damage to armor. New armor damage: %.1f", GetName(), GetID(), total_armor_damage, new_damage);
        }
        else
        {
            //first determine how much we can actually apply to
            //the armor, the rest goes further down.
            double consumed_armor_ratio = available_armor / armor_damage.GetTotal();
            d *= 1.0 - consumed_armor_ratio;

            if(available_armor > 0)
            {
                total_damage += available_armor;

                _log(ITEM__TRACE, "%s(%u): Armor depleated with %.1f damage. %.1f damage remains.", GetName(), GetID(), available_armor, d.GetTotal());

                //all armor has been penetrated.
                m_armorDamage = m_self->GetAttribute(AttrArmorHP).get_float();
            }


            //Hull/Structure:

            //The base hp and damage attributes represent structure.
            double available_hull = m_self->GetAttribute(AttrHp).get_float() - m_hullDamage;
            Damage hull_damage = d.MultiplyDup(
                m_self->GetAttribute(AttrHullKineticDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullThermalDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullEmDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullExplosiveDamageResonance).get_float()
            );
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

            // Not sure about this, but with this we get some random hits... :)
            //total_hull_damage += total_hull_damage * random_damage_mult;
            hull_damage.SumWithMultFactor( random_damage_mult );
            double total_hull_damage = hull_damage.GetTotal();
            total_damage += total_hull_damage;

            if(total_hull_damage < available_hull)
            {

                //we can take all this damage with our hull...
                double new_damage = m_hullDamage + total_hull_damage;
                m_hullDamage = new_damage;
                _log(ITEM__TRACE, "%s(%u): Applying entire %.1f damage to structure. New structure damage: %.1f", GetName(), GetID(), total_hull_damage, new_damage);
            }
            else
            {
                //dead....
                _log(ITEM__TRACE, "%s(%u): %.1f damage has depleated our structure. Time to explode.", GetName(), GetID(), total_hull_damage);
                killed = true;
                //m_hullDamage = m_self->hp();
                m_hullDamage = m_self->GetAttribute(AttrHp).get_float();
            }

            //TODO: deal with damaging modules. no idea the mechanics on this.
        }
    }

    //if( total_damage <= 0.0 )
    //    return(killed);

    PyTuple *up;

    //Notifications to ourself:
    Notify_OnEffectHit noeh;
    noeh.itemID = d.source->GetID();
    noeh.effectID = d.effect;
    noeh.targetID = GetID();
    noeh.damage = total_damage;

    up = noeh.Encode();
    QueueDestinyEvent(&up);
    PySafeDecRef( up );

    //NOTE: could send out the RD version of this message instead of the R version, which
    //includes "weapon" and "owner" instead of "source".
    Notify_OnDamageMessage_Self ondam;
    ondam.messageID = DamageMessageIDs_Other[random_damage];	//DamageMessageIDs_Self[random_damage];
    ondam.damage = total_damage;
    ondam.source = d.source->GetID();
    ondam.splash = "";
    up = ondam.Encode();
    QueueDestinyEvent(&up);
    PySafeDecRef( up );

    //Notifications to others:
    //I am not sure what the correct scope of this broadcast
    //should be. For now, it goes to anybody targeting us.
    if(targets.IsTargetedBySomething()) {
        up = noeh.Encode();
        targets.QueueTBDestinyEvent(&up);
        PySafeDecRef( up );

        Notify_OnDamageMessage_Other ondamo;
        ondamo.messageID = DamageMessageIDs_Other[random_damage];
        ondamo.format_type = fmtMapping_itemTypeName;
        ondamo.weaponType = d.weapon->typeID();
        ondamo.damage = total_damage;
        ondamo.target = GetID();
        ondamo.splash = "";

        up = ondamo.Encode();
        targets.QueueTBDestinyEvent(&up);
        PySafeDecRef( up );
    }

    if(killed == true)
    {
        Killed(d);
    }
    else
    {
        _SendDamageStateChanged();
    }

    return(killed);

}

void NPC::_SendDamageStateChanged() const
{
    DoDestinyDamageState state;
    MakeDamageState(state);

    DoDestiny_OnDamageStateChange ddsc;
    ddsc.entityID = GetID();
    ddsc.state = state.Encode();

    PyTuple *up;
    up = ddsc.Encode();
    targets.QueueTBDestinyUpdate(&up);
}

void ItemSystemEntity::_SendDamageStateChanged() const {
    DoDestinyDamageState state;
    MakeDamageState(state);

    DoDestiny_OnDamageStateChange ddsc;
    ddsc.entityID = GetID();
    ddsc.state = state.Encode();

    PyTuple *up;
    up = ddsc.Encode();
    targets.QueueTBDestinyUpdate(&up);
}

void SystemEntity::Killed(Damage &fatal_blow) {
    targets.ClearAllTargets(false);    //I assume a client does not need this notification.
}

void DynamicSystemEntity::Killed(Damage &fatal_blow) {
    ItemSystemEntity::Killed(fatal_blow);
    if(m_destiny != NULL) {
        m_destiny->SendTerminalExplosion();
    }
}

void Client::Killed(Damage &fatal_blow) {
    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
			client = NULL;
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    if(GetShip()->typeID() == itemTypeCapsule)
	{
		//sLog.Error( "Client::Killed()", "WARNING!  This is likely to crash the server, DO NOT ATTEMPT To POD KILL until this function properly returns client to the station of your last clone update!" );
        //we have been pod killed... off we go.

		//TODO: explode the capsule and make a new one in the last medical clone's station:
		GPoint deadPodPosition = GetPosition();
		uint32 oldPodItemID = GetShipID();
		GPoint capsulePosition = GPoint(0,0,0);

		ShipRef deadPodRef = GetShip();    //grab a ship ref to ensure that nobody else nukes it first.
        std::string corpse_name = GetName();
        corpse_name += "'s Corpse";

		// Get location of last medical clone:
        std::string capsule_name = GetName();
        capsule_name += "'s Capsule";
		ItemData capsuleItemData(
			capsule_name.c_str(),
			itemTypeCapsule,
			GetCharacterID(),
			GetCloneLocationID(),
			(EVEItemFlags)flagHangar,
			false, true, 1,
			capsulePosition,
			""
		);

        ShipRef capsuleRef = m_services.item_factory.SpawnShip( capsuleItemData );
		if( !capsuleRef )
		{
			sLog.Error("Client::Killed()", "Failed to create capsule for character '%s'", GetName());
            //what to do?
			throw PyException( MakeCustomError ( "Unable to generate escape pod" ) );
            return;
        }

		Destiny()->SendTerminalExplosion();

	    capsuleRef->Move(GetLocationID(), (EVEItemFlags)flagHangar, true);

	    ShipRef updatedCapsuleRef = services().item_factory.GetShip( capsuleRef->itemID() );

		System()->bubbles.Remove( this, true );

        BoardShip(updatedCapsuleRef);

		//TODO: spawn a corpsed named for this client's character
		uint32 corpseTypeID = 10041;	// typeID from 'invTypes' table for "Frozen Corpse"
		InventoryItemRef corpseItemRef;
		ItemData corpseItemData(
			corpseTypeID,
			GetCharacterID(),
			GetLocationID(),
			flagAutoFit,
			corpse_name.c_str(),
			deadPodPosition
		);

		corpseItemRef = System()->GetServiceMgr()->item_factory.SpawnItem( corpseItemData );
		if( !corpseItemRef )
			throw PyException( MakeCustomError( "Unable to spawn item of type %u.", corpseTypeID ) );

		DBSystemDynamicEntity corpseEntity;

		corpseEntity.allianceID = 0;
		corpseEntity.categoryID = EVEDB::invCategories::Celestial;
		corpseEntity.corporationID = 0;
		corpseEntity.flag = 0;
		corpseEntity.groupID = EVEDB::invGroups::Biomass;
		corpseEntity.itemID = corpseItemRef->itemID();
		corpseEntity.itemName = corpse_name;
		corpseEntity.locationID = GetLocationID();
		corpseEntity.ownerID = 1;
		corpseEntity.typeID = corpseTypeID;
		corpseEntity.x = deadPodPosition.x;
		corpseEntity.y = deadPodPosition.y;
		corpseEntity.z = deadPodPosition.z;

		// Actually do the spawn using SystemManager's BuildEntity:
		if( !(System()->BuildDynamicEntity( this, corpseEntity )) )
		{
			sLog.Error("Client::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", corpseTypeID);
			throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
			return;
		}

		//TODO: destroy all implants

		//TODO: change current clone back to Clone Grade Alpha
		CorporationDB corpDBobj;
		corpDBobj.ChangeCloneType(GetCharacterID(), 164);		// typeID = 164 is for Clone Grade Alpha

        //TODO: send them back to their clone.
        m_system->RemoveClient(this);
    }
	else
	{
        //our ship has been destroyed. Off to our capsule.
        //We are currently not keeping our real capsule around in the DB, so we need to make a new one.
		GPoint deadShipPosition = GetPosition();
		uint32 oldShipItemID = GetShipID();
		GPoint capsulePosition = GetPosition();

		//set capsule position 500m off from old ship:
		capsulePosition.x += GetShip()->GetAttribute(AttrRadius).get_float() + 100.0;
		capsulePosition.y += GetShip()->GetAttribute(AttrRadius).get_float() + 100.0;
		capsulePosition.z += GetShip()->GetAttribute(AttrRadius).get_float() - 100.0;

		ShipRef deadShipRef = GetShip();    //grab a ship ref to ensure that nobody else nukes it first.
        std::string wreck_name = GetName();
        wreck_name += "'s ";
		wreck_name += deadShipRef->itemName();
		wreck_name += "Wreck";

        std::string capsule_name = GetName();
        capsule_name += "'s Capsule";
		ItemData capsuleItemData(
			capsule_name.c_str(),
			itemTypeCapsule,
			GetCharacterID(),
			GetLocationID(),
			(EVEItemFlags)flagCapsule,
			false, true, 1,
			capsulePosition,
			""
		);

        ShipRef capsuleRef = m_services.item_factory.SpawnShip( capsuleItemData );
		if( !capsuleRef )
		{
			sLog.Error("Client::Killed()", "Failed to create capsule for character '%s'", GetName());
            //what to do?
			throw PyException( MakeCustomError ( "Unable to generate escape pod" ) );
            return;
        }

        //ok, nothing can fail now, we need have our capsule, make the transition.

        //put the capsule where the ship was
	    capsuleRef->Move(GetLocationID(), (EVEItemFlags)flagCapsule, true);

	    ShipRef updatedCapsuleRef = services().item_factory.GetShip( capsuleRef->itemID() );

		System()->bubbles.Remove( this, true );

        BoardShip(updatedCapsuleRef);

		System()->bubbles.Add( this, true);
		Destiny()->SetPosition( capsulePosition, true );

		// Set ownership of dead ship to EVE system:
	    deadShipRef->ChangeOwner( 1 );

        m_shipId = capsuleRef->itemID();
        mSession.SetInt("shipid", capsuleRef->itemID() );

        // === Kill off the old ship ===
		// Create new ShipEntity for dead ship and add it to the SystemManager, before we explode it:
		ShipEntity * deadShipObj = new ShipEntity( deadShipRef, System(), *(System()->GetServiceMgr()), deadShipPosition );
		System()->AddEntity( deadShipObj );

		// Add ball to bubble manager for this client's character's system for the dead pilot-less ship:
		System()->bubbles.Add( deadShipObj, true );

		// Do Destiny Updates to exit the dead ship:
		Destiny()->SendEjectShip( updatedCapsuleRef, deadShipRef );

		deadShipObj->Destiny()->SendTerminalExplosion();

		//TODO: figure out anybody else which may be referencing this ship...
		uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(deadShipObj->Item()->typeID());
		deadShipObj->Bubble()->Remove(deadShipObj, true);
        deadShipRef->Delete();    //remove from the DB.

		// TODO: Spawn a wreck matching the ship we lost
		InventoryItemRef wreckItemRef;
		ItemData wreckItemData(
			wreckTypeID,
			GetCharacterID(),
			GetLocationID(),
			flagAutoFit,
			wreck_name.c_str(),
			deadShipPosition
		);

		wreckItemRef = System()->GetServiceMgr()->item_factory.SpawnItem( wreckItemData );
		if( !wreckItemRef )
			throw PyException( MakeCustomError( "Unable to spawn item of type %u.", wreckTypeID ) );

		DBSystemDynamicEntity wreckEntity;

		wreckEntity.allianceID = 0;
		wreckEntity.categoryID = EVEDB::invCategories::Celestial;
		wreckEntity.corporationID = 0;
		wreckEntity.flag = 0;
		wreckEntity.groupID = EVEDB::invGroups::Wreck;
		wreckEntity.itemID = wreckItemRef->itemID();
		wreckEntity.itemName = wreck_name;
		wreckEntity.locationID = GetLocationID();
		wreckEntity.ownerID = 1;
		wreckEntity.typeID = wreckTypeID;
		wreckEntity.x = deadShipPosition.x;
		wreckEntity.y = deadShipPosition.y;
		wreckEntity.z = deadShipPosition.z;

		// Actually do the spawn using SystemManager's BuildEntity:
		if( !(System()->BuildDynamicEntity( this, wreckEntity )) )
		{
			sLog.Error("Client::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
			throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
			return;
		}

		//TODO: award status changes. (entitySecurityStatusKillBonus)
		if( client != NULL )
			client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

		// TODO: Place random selection of modules/charges/cargo/drones into container of wreck
    }
}


void NPC::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
			client = NULL;
        }
    }
    else
    {
        client = killer->CastToClient();
    }

	GPoint deadNPCPosition = this->Destiny()->GetPosition();
	uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(this->Item()->typeID());
	this->AI()->ClearAllTargets();
	this->Bubble()->Remove(this, true);

	// TODO: Spawn a wreck matching the ship we lost
	InventoryItemRef wreckItemRef;
	ItemData wreckItemData(
		wreckTypeID,
		client->GetCharacterID(),
		GetLocationID(),
		flagAutoFit,
		this->Item()->itemName().c_str(),
		deadNPCPosition
	);

	wreckItemRef = System()->GetServiceMgr()->item_factory.SpawnItem( wreckItemData );
	if( !wreckItemRef )
		throw PyException( MakeCustomError( "Unable to spawn item of type %u.", wreckTypeID ) );

	DBSystemDynamicEntity wreckEntity;

	wreckEntity.allianceID = 0;
	wreckEntity.categoryID = EVEDB::invCategories::Celestial;
	wreckEntity.corporationID = 0;
	wreckEntity.flag = 0;
	wreckEntity.groupID = EVEDB::invGroups::Wreck;
	wreckEntity.itemID = wreckItemRef->itemID();
	wreckEntity.itemName = this->Item()->itemName();
	wreckEntity.locationID = GetLocationID();
	wreckEntity.ownerID = 1;
	wreckEntity.typeID = wreckTypeID;
	wreckEntity.x = deadNPCPosition.x;
	wreckEntity.y = deadNPCPosition.y;
	wreckEntity.z = deadNPCPosition.z;

	// Actually do the spawn using SystemManager's BuildEntity:
	if( !(System()->BuildDynamicEntity( NULL, wreckEntity )) )
	{
		sLog.Error("NPC::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
		throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
		return;
	}

    //TODO: drop loot.
    //_DropLoot(fatal_blow.source);
    _DropLoot(killer);

    //award kill bounty.
    //_AwardBounty(fatal_blow.source);
    _AwardBounty(killer);

    //TODO: award status changes. (entitySecurityStatusKillBonus)
	if( client != NULL )
		client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    //notify our spawner that we are gone.
    if(m_spawner != NULL) {
      m_spawner->SpawnDepoped(m_self->itemID());
    }

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
	sLog.Warning("NPC::_DropLoot", "TODO: This function has NO code in it to create a loot drop for an NPC kill!");

    // Send an OnSpecialFX (9) for effects.Jettison (with can's ID, not npc)
}

void NPC::_AwardBounty(SystemEntity *who) {
    //double bounty = m_self->entityKillBounty();
    double bounty = m_self->GetAttribute(AttrEntityKillBounty).get_float();
    if(bounty <= 0) {
        return;    //no bounty to award...
    }

    //TODO: handle case where drone strikes fatal blow... bounty goes to master.

    //TODO: handle distribution to gangs.

    if(who->IsClient() == false) {
        _log(NPC__TRACE, "Refusing to award bounty on %u to non-client %u", GetID(), who->GetID());
        return;    //bounty doesn't make sense for anything other than clients.
    }

    Client *killer = who->CastToClient();

    killer->AddBalance(bounty);

    std::string reason = "Bounty";    //TODO: improve this.

    if(!m_services.serviceDB().GiveCash(
            killer->GetID(),
            RefType_playerDonation,    //TODO: find the proper type
            m_self->itemID(),    //probably actually a special concord item ID or something.
            killer->GetID(),
            "",    //unknown const char *argID1,
            killer->GetAccountID(),
            accountCash,
            bounty,
            killer->GetBalance(),
            reason.c_str()
    )) {
        codelog(CLIENT__ERROR, "%s: Failed to record bountry of %f from death of %u (type %u)", killer->GetName(), bounty, GetID(), m_self->typeID());
        //well.. this isnt a huge deal, so we will get over it.
    }

}


void ShipEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("ShipEntity::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void ShipEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a ShipEntity implementation of damage system (incomplete)
bool ShipEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void ShipEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
			client = NULL;
        }
    }
    else
    {
        client = killer->CastToClient();
    }

	// Spawn a wreck for the Ship that was destroyed:
	uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(this->Item()->typeID());
	std::string wreck_name = this->Item()->itemName();
	GPoint wreckPosition = this->Destiny()->GetPosition();
	InventoryItemRef wreckItemRef;
	ItemData wreckItemData(
		wreckTypeID,
		1,						// Assign this wreck to be owned by the EVE system
		GetLocationID(),
		flagAutoFit,
		wreck_name.c_str(),
		wreckPosition
	);

	wreckItemRef = System()->GetServiceMgr()->item_factory.SpawnItem( wreckItemData );
	if( !wreckItemRef )
		throw PyException( MakeCustomError( "Unable to spawn item of type %u.", wreckTypeID ) );

	DBSystemDynamicEntity wreckEntity;

	wreckEntity.allianceID = 0;
	wreckEntity.categoryID = EVEDB::invCategories::Celestial;
	wreckEntity.corporationID = 0;
	wreckEntity.flag = 0;
	wreckEntity.groupID = EVEDB::invGroups::Wreck;
	wreckEntity.itemID = wreckItemRef->itemID();
	wreckEntity.itemName = wreck_name;
	wreckEntity.locationID = GetLocationID();
	wreckEntity.ownerID = 1;
	wreckEntity.typeID = wreckTypeID;
	wreckEntity.x = wreckPosition.x;
	wreckEntity.y = wreckPosition.y;
	wreckEntity.z = wreckPosition.z;

	// Actually do the spawn using SystemManager's BuildEntity:
	if( !(System()->BuildDynamicEntity( NULL, wreckEntity )) )		// WARNING! Passing NULL for a client object (this is ok since BuildDynamicEntity() does not use the first argument
	{
		sLog.Error("ShipEntity::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
		throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
		return;
	}

    //TODO: drop loot.
    //_DropLoot(fatal_blow.source);
    _DropLoot(killer);

    //TODO: award status changes. (entitySecurityStatusKillBonus)
	if( client != NULL )
		client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}

void ShipEntity::_DropLoot(SystemEntity *owner) {

    //entityLootValueMin
    //entityLootValueMax
    //entityLootCountMin
    //entityLootCountMax
    //minLootCount
    //maxLootCount
    //minLootValue
    //maxLootValue
	sLog.Warning("NPC::_DropLoot", "TODO: This function has NO code in it to create a loot drop for an NPC kill!");

    // Send an OnSpecialFX (9) for effects.Jettison (with can's ID, not npc)
}


void DroneEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("Drone::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void DroneEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a DroneEntity implementation of damage system (incomplete)
bool DroneEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void DroneEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    //TODO: award status changes. (entitySecurityStatusKillBonus)
    client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}


void StructureEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("StructureEntity::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void StructureEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a StructureEntity implementation of damage system (incomplete)
bool StructureEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void StructureEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    //TODO: award status changes. (entitySecurityStatusKillBonus)
    client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}


void ContainerEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("ContainerEntity::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void ContainerEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a ContainerEntity implementation of damage system (incomplete)
bool ContainerEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void ContainerEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    //TODO: award status changes. (entitySecurityStatusKillBonus)
    client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}


void DeployableEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("DeployableEntity::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void DeployableEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a DeployableEntity implementation of damage system (incomplete)
bool DeployableEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void DeployableEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    //TODO: award status changes. (entitySecurityStatusKillBonus)
    client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}


// This is a AsteroidEntity implementation of damage system (incomplete)
bool AsteroidEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity

    return ItemSystemEntity::ApplyDamage(d);
}

void AsteroidEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    m_system->RemoveEntity(this);
}


void CelestialEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("CelestialEntity::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void CelestialEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a CelestialEntity implementation of damage system (incomplete)
bool CelestialEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void CelestialEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    //TODO: award status changes. (entitySecurityStatusKillBonus)
    client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}


void StationEntity::_ReduceDamage(Damage &d) {
    // armorEmDamageResonance
    // armorExplosiveDamageResonance
    // armorKineticDamageResonance
    // armorThermalDamageResonance
    // shieldEmDamageResonance
    // shieldExplosiveDamageResonance
    // shieldKineticDamageResonance
    // shieldThermalDamageResonance
	sLog.Warning("StationEntity::_ReduceDamage", "TODO: This function has NO code in it to reduce damage dealt by another entity!");
}

void StationEntity::ApplyDamageModifiers(Damage &d, SystemEntity *target)
{
}

// This is a StationEntity implementation of damage system (incomplete)
bool StationEntity::ApplyDamage(Damage &d) {
// for now this uses ItemSystemEntity
    _ReduceDamage(d);

    return ItemSystemEntity::ApplyDamage(d);
}

void StationEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    DynamicSystemEntity::Killed(fatal_blow);

    SystemEntity *killer = fatal_blow.source;
    Client* client = m_services.entity_list.FindByShip( killer->Item()->ownerID() );
    if( !killer->IsClient() )
    {
        if( client != NULL )
        {
            killer = static_cast<SystemEntity*>(client);
        }
    }
    else
    {
        client = killer->CastToClient();
    }

    //TODO: award status changes. (entitySecurityStatusKillBonus)
    client->GetChar()->addSecurityRating( m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float() );

    m_system->RemoveEntity(this);
}

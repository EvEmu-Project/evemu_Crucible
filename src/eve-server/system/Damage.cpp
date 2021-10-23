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
    Author:        Zhur
    Rewrite:    Allan
*/


#include "system/Damage.h"

#include "../../eve-common/EVE_Damage.h"
//#include "packets/Damage.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "manufacturing/Blueprint.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

/*
DAMAGE
DAMAGE__ERROR
DAMAGE__WARNING
DAMAGE__MESSAGE
DAMAGE__INFO
DAMAGE__TRACE
DAMAGE__DEBUG
*/

// this is for turrets
Damage::Damage(SystemEntity* pSE, InventoryItemRef wRef, float kin, float ther, float emp, float exp, float mod, uint16 eID)
: srcSE(pSE), effectID(eID), weaponRef(wRef), chargeRef(InventoryItemRef(nullptr))
{
    em          = emp;
    kinetic     = kin;
    thermal     = ther;
    explosive   = exp;
    modifier    = mod;
}

// this is for npcs
Damage::Damage(SystemEntity* pSE, InventoryItemRef wRef, float mod, uint16 eID)
: srcSE(pSE), effectID(eID), weaponRef(wRef), chargeRef(InventoryItemRef(nullptr))
{
    modifier    = mod;

    em          = wRef->GetAttribute(AttrEmDamage).get_float();
    kinetic     = wRef->GetAttribute(AttrKineticDamage).get_float();
    thermal     = wRef->GetAttribute(AttrThermalDamage).get_float();
    explosive   = wRef->GetAttribute(AttrExplosiveDamage).get_float();

    _log(DAMAGE__WARNING, "Damage:C'tor - Called by source %s(%u) with weapon %s(%u).",
         srcSE->GetName(), srcSE->GetID(), wRef->name(), wRef->itemID() );
}

// this is for missiles
Damage::Damage(SystemEntity* pSE, InventoryItemRef wRef, InventoryItemRef cRef, uint16 eID)
: srcSE(pSE), effectID(eID), weaponRef(wRef), chargeRef(cRef)
{
    modifier    = 1;

    em          = cRef->GetAttribute(AttrEmDamage).get_float();
    kinetic     = cRef->GetAttribute(AttrKineticDamage).get_float();
    thermal     = cRef->GetAttribute(AttrThermalDamage).get_float();
    explosive   = cRef->GetAttribute(AttrExplosiveDamage).get_float();

    _log(DAMAGE__WARNING, "Damage:C'tor - Called by source %s(%u) with weapon %s(%u) using charge %s(%u).",
         srcSE->GetName(), srcSE->GetID(), wRef->name(), wRef->itemID(), cRef->name(), cRef->itemID() );
}

// No specific damage dealt here, just killed
Damage::Damage(SystemEntity* pSE, bool fatal_blow/*false*/)
: srcSE(pSE), effectID(EVEEffectID::targetAttack)
{
    assert(fatal_blow and "Damage() fatal_blow called without 2nd param being true!");

    em = kinetic = thermal = explosive = 0.0f;
    weaponRef = InventoryItemRef(nullptr);
    chargeRef = InventoryItemRef(nullptr);
}

bool SystemEntity::ApplyDamage(Damage &d) {
    double profileStartTime(GetTimeUSeconds());

    if (is_log_enabled(DAMAGE__MESSAGE)) {
        if (d.srcSE->IsNPCSE()) {
            _log(DAMAGE__MESSAGE, "%s(%u): Initializing %.2f damage from NPC %s(%u) with K:%.3f, T:%.3f, EM:%.3f, E:%.3f",\
                    GetName(), GetID(), d.GetTotal(), d.srcSE->GetName(), d.srcSE->GetID(), \
                    d.GetKinetic(), d.GetThermal(), d.GetEM(), d.GetExplosive() );
        } else if (d.srcSE->IsDroneSE()){
            _log(DAMAGE__MESSAGE, "%s(%u): Initializing %.2f damage from Drone %s(%u) with K:%.3f, T:%.3f, EM:%.3f, E:%.3f",\
                    GetName(), GetID(), d.GetTotal(), d.srcSE->GetName(), d.srcSE->GetID(), \
                    d.GetKinetic(), d.GetThermal(), d.GetEM(), d.GetExplosive() );
        } else if (d.srcSE->HasPilot()) {
            _log(DAMAGE__MESSAGE, "%s(%u): Initializing %.2f damage from %s's %s(%u) using %s(%u) %s with K:%.3f, T:%.3f, EM:%.3f, E:%.3f",\
                    GetName(), GetID(), d.GetTotal(), d.srcSE->GetPilot()->GetName(), d.srcSE->GetName(), d.srcSE->GetID(), \
                    d.weaponRef->name(), d.weaponRef->itemID(), (d.chargeRef ? d.chargeRef->name() : ""), \
                    d.GetKinetic(), d.GetThermal(), d.GetEM(), d.GetExplosive() );
        } else {
            _log(DAMAGE__MESSAGE, "%s(%u): Initializing %.2f damage from unknown source.", GetName(), GetID(), d.GetTotal());
        }
    }

    int8 damageID = 0;
    switch (d.weaponRef->groupID()) {
        case EVEDB::invGroups::Missile_Launcher_Assault:
        case EVEDB::invGroups::Missile_Launcher_Bomb:       // not sure here
        case EVEDB::invGroups::Missile_Launcher_Citadel:
        case EVEDB::invGroups::Missile_Launcher_Cruise:
        case EVEDB::invGroups::Missile_Launcher_Defender:   // not sure here
        case EVEDB::invGroups::Missile_Launcher_Heavy:
        case EVEDB::invGroups::Missile_Launcher_Heavy_Assault:
        case EVEDB::invGroups::Missile_Launcher_Rocket:
        case EVEDB::invGroups::Missile_Launcher_Siege:
        case EVEDB::invGroups::Missile_Launcher_Standard: {
            // apply damage modifier from config
            d *= sConfig.rates.missileDamage;
            // should this be adjusted based on damage?
            damageID = 6;
        } break;
        case EVEDB::invGroups::Super_Weapon: {
        /*   TODO
         * this damage will need to be adjusted based on distance from target, then called for each target,
         *  and modified/corrected as the weapon implementation is completed.
         *  all modifiers to be calc'd in weapon code and sent here for correct damageID
         */
            damageID = 5;
        } break;
        case EVEDB::invGroups::Missile_Launcher_Snowball: {
            // these dont do any damage
            //  update this to use real toHit data (once we implement them....)
            damageID = MakeRandomInt(0,8);
        } break;
        default: {
            float modifier = d.GetModifier();
            d *= modifier;
                 if (modifier == 3.0f)   { damageID = 8; }  //strikes perfectly, wrecking
            else if (modifier > 1.2501f) { damageID = 7; } //places an excellent hit
            else if (modifier > 0.9999f) { damageID = 6; } //aims well
            else if (modifier > 0.7501f) { damageID = 5; } //hits
            else if (modifier > 0.6251f) { damageID = 4; } //lightly hits
            else if (modifier > 0.4121f) { damageID = 3; } //barely scratches
            else if (modifier > 0.3751f) { damageID = 2; } //glances off
            else if (modifier > 0.2501f) { damageID = 1; } //barely misses
            else                         { damageID = 0; } //misses completely
            _log(DAMAGE__TRACE, "%s(%u): Modifier: %.3f, damageID: %u.", GetName(), GetID(), modifier, damageID);
        } break;
    }

    // apply damage modifier from config
    d *= sConfig.rates.damageRate;

    // this is calculated and created on every call...
    Damage DamageToShield = d.MultiplyDup(
        m_self->GetAttribute(AttrShieldKineticDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldThermalDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldEmDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldExplosiveDamageResonance).get_float() );

    bool killed = false;
    float total_damage(0.0f);
    float shield_damage(DamageToShield.GetTotal());
    float available_shield(m_self->GetAttribute(AttrShieldCharge).get_float());
    if (shield_damage <= available_shield) {
        /** @todo  this works, but still needs work....
        if (HasPilot())
            if (damageID > 2) {
                float uniformity = m_self->GetAttribute(AttrShieldUniformity).get_float();
                uniformity += (0.05 * GetPilot()->GetChar()->GetSkillLevel(EvESkill::TacticalShieldManipulation));
                if ((available_shield /m_self->GetAttribute(AttrShieldCapacity).get_float()) < uniformity) {
                    float bleedthru = (d.GetTotal() * 0.01f);
                    m_self->SetAttribute(AttrArmorDamage, (bleedthru + m_self->GetAttribute(AttrArmorDamage).get_float()));
                    shield_damage -= bleedthru;
                }
            }
        */
        total_damage += shield_damage;
        float new_charge = available_shield - shield_damage;
        m_self->SetAttribute(AttrShieldCharge, new_charge);

        _log(DAMAGE__DEBUG, "%s(%u): Applying %.2f damage to shields. New charge: %.2f.",
             GetName(), GetID(), shield_damage, new_charge);
    } else {
        // get fraction of damage partial shield absorbs, and lower total damage by that fraction
        d *= (1 - (available_shield /shield_damage));
        total_damage += available_shield;

        if (available_shield > 0.0f) {
            _log(DAMAGE__INFO, "%s(%u): Shield depleted with %.2f damage. %.2f damage remains.",
                 GetName(), GetID(), available_shield, d.GetTotal());
            m_self->SetAttribute(AttrShieldCharge, EvilZero);
        }

        //Armor:
        float available_armor = m_self->GetAttribute(AttrArmorHP).get_float() - m_self->GetAttribute(AttrArmorDamage).get_float();
        Damage DamageToArmor = d.MultiplyDup(
            m_self->GetAttribute(AttrArmorKineticDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorThermalDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorEmDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorExplosiveDamageResonance).get_float() );

        float armor_damage = DamageToArmor.GetTotal();
        if (armor_damage <= available_armor) {
            if (HasPilot()) {
                if ((available_armor /m_self->GetAttribute(AttrArmorHP).get_float()) < m_self->GetAttribute(AttrArmorUniformity).get_float()) {
                    float new_damage = d.GetTotal() * 0.01;
                    float hull_damage = m_self->GetAttribute(AttrDamage).get_float() + new_damage;
                    _log(DAMAGE__DEBUG, "%s(%u): Applying %.2f leakthru damage to structure. New structure damage: %.2f",
                         GetName(), GetID(), new_damage, hull_damage);
                    m_self->SetAttribute(AttrDamage, hull_damage);
                    // remove this leakthru damage from armor damage
                    armor_damage -= new_damage;
                }
            }
            total_damage += armor_damage;
            float new_damage = m_self->GetAttribute(AttrArmorDamage).get_float() + armor_damage;
            m_self->SetAttribute(AttrArmorDamage, new_damage);
            _log(DAMAGE__DEBUG, "%s(%u): Applying %.2f damage to armor. New armor damage: %.2f",
                 GetName(), GetID(), armor_damage, new_damage);
        } else {
            d *= (1 - (available_armor /armor_damage));
            total_damage += available_armor;

            if (available_armor > 0) {
                _log(DAMAGE__INFO, "%s(%u): Armor depleted with %.2f damage. %.2f damage remains.",
                     GetName(), GetID(), available_armor, d.GetTotal());
                m_self->SetAttribute(AttrArmorDamage, m_self->GetAttribute(AttrArmorHP));
            }

            //Hull/Structure:
            //The base hp and damage attributes represent structure.
            float available_hull = m_self->GetAttribute(AttrHP).get_float() - m_self->GetAttribute(AttrDamage).get_float();
            Damage DamageToHull = d.MultiplyDup(
                m_self->GetAttribute(AttrKineticDamageResonance).get_float(),
                m_self->GetAttribute(AttrThermalDamageResonance).get_float(),
                m_self->GetAttribute(AttrEmDamageResonance).get_float(),
                m_self->GetAttribute(AttrExplosiveDamageResonance).get_float() );

            float hull_damage = DamageToHull.GetTotal();
            if (hull_damage < available_hull) {
                total_damage += hull_damage;
                float new_damage = m_self->GetAttribute(AttrDamage).get_float() + hull_damage;
                m_self->SetAttribute(AttrDamage, new_damage);
                _log(DAMAGE__DEBUG, "%s(%u): Applying %.2f damage to structure. New structure damage: %.2f",
                     GetName(), GetID(), hull_damage, new_damage);
            } else {
                total_damage += available_hull;
                //dead....
                _log(DAMAGE__INFO, "%s(%u): %.2f damage has depleted our structure. Time to explode.",
                     GetName(), GetID(), hull_damage);
                killed = true;
            }

            // module damage.
            //  after armor is gone, make damage to random module.
            if (HasPilot())
                GetShipSE()->DamageRandModule(sConfig.server.ModuleDamageChance);    // config option for random module damage chance
        }
    }

    if (killed) {
        if (m_killed)
            return true;

        m_killed = true;

        // OnNotify:OnTransmission -  (235799, `You have killed this defenseless NPC, bully.  Also, you have killed this NPC and are receiving this message.`)
        m_destiny->SendTerminalExplosion(m_self->itemID(), m_bubble->GetID(), isGlobal());

        Killed(d);  // this must NOT remove dead SE from system.
        SystemEntity::Killed(d);    // this removes dead SE from system then deletes itemRef and all its contents
    } else {
        /**
         * ALL dmg msgs working  22Apr15 (hacked - found the actual msgIDs)
         * fixed msgIDs and removed xmlp  - 15Sept19
         * @todo  still need to check/add detailed dmg msgs
         */
        if (HasPilot()) {
            //  notify player of damage received
            PyDict* dict = new PyDict();
                dict->SetItemString("source", new PyInt(d.srcSE->GetID()));
                dict->SetItemString("weapon", new PyInt((d.chargeRef.get() != nullptr ? d.chargeRef->typeID() : d.weaponRef->typeID())));
                dict->SetItemString("target", new PyInt(GetID()));
                dict->SetItemString("damage", new PyFloat(total_damage));
            PyTuple* tuple = new PyTuple(3);
                tuple->SetItem(0, new PyString("OnDamageMessage"));
                tuple->SetItem(1, new PyString(Dmg::Msg::Taken[damageID]));
                tuple->SetItem(2, dict);
            GetPilot()->QueueDestinyEvent(&tuple);
        }
        if (d.srcSE->HasPilot()) {
            //notify to player of damage done:
            PyDict* dict = new PyDict();
                dict->SetItemString("weapon", new PyInt((d.chargeRef.get() != nullptr ? d.chargeRef->typeID() : d.weaponRef->typeID())));
                dict->SetItemString("target", new PyInt(GetID()));
                dict->SetItemString("damage", new PyFloat(total_damage));
            PyTuple* tuple = new PyTuple(3);
            bool banked = false;
            tuple->SetItem(0, new PyString("OnDamageMessage"));
            if (d.weaponRef->IsModuleItem()) {
                GenericModule* pMod = d.srcSE->GetShipSE()->GetShipItemRef()->GetModule(d.weaponRef->flag());
                if (pMod != nullptr)
                    if (pMod->IsLinked())
                        banked = true;
            }
            if (banked) {
                tuple->SetItem(1, new PyString(Dmg::Msg::Banked[damageID]));
            } else {
                tuple->SetItem(1, new PyString(Dmg::Msg::Given[damageID]));
            }
            tuple->SetItem(2, dict);
            d.srcSE->GetPilot()->QueueDestinyEvent(&tuple);
        } else if (d.srcSE->IsDroneSE()) {
            // verify drone has owner set
            if (d.srcSE->GetDroneSE()->GetOwner() != nullptr) {
                //  notify player of damage done by drone
                PyDict* dict = new PyDict();
                    dict->SetItemString("source", new PyInt(d.srcSE->GetID()));
                    dict->SetItemString("target", new PyInt(GetID()));
                /*
                PyTuple* tuple = new PyTuple(2);
                    tuple->AddItem(0, PyStatic.NewNone());  // i dont know what this is
                    tuple->AddItem(1, new PyInt(d.srcSE->GetDroneSE()->GetOwner()->GetCharID())):
                */
                    //dict->SetItemString("owner", tuple));
                    dict->SetItemString("damage", new PyFloat(total_damage));
                PyTuple* tuple = new PyTuple(3);
                    tuple->SetItem(0, new PyString("OnDamageMessage"));
                    tuple->SetItem(1, new PyString(Dmg::Msg::Taken[damageID]));
                    tuple->SetItem(2, dict);
                d.srcSE->GetDroneSE()->GetOwner()->QueueDestinyEvent(&tuple);
            } else {
                // make error about active drone with no owner set
                _log(DRONE__WARNING, "Drone %u attacking %s with no owner set.", d.srcSE->GetID(), GetName());
            }
        }

        SendDamageStateChanged();
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::damage, GetTimeUSeconds() - profileStartTime);

    return killed;
}

void ShipSE::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr) or (m_system == nullptr))
        return; // make error here?

    m_shipRef->SetPopped(true);

    /* {'messageKey': 'ShipExploded', 'dataID': 17881627, 'suppressable': True, 'bodyID': 258841, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258840, 'messageID': 1558}
     * u'ShipExplodedBody'}(u'Your ship has been destroyed by {[character]charID.name}.', None, {u'{[character]charID.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'charID'}})
     */
    uint32 killerID = 0, locationID = GetLocationID();
    Client* pClient(nullptr);
    SystemEntity* killer(fatal_blow.srcSE);

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = killer->GetDroneSE()->GetOwner();
        if (pClient == nullptr) {
            /** @todo  make error here */
            sLog.Error("Ship::Killed()", "killer == IsDrone and pPlayer == nullptr");
            EvE::traceStack();
        } else {
            killerID = pClient->GetCharacterID();
        }
    } else {
        killerID = killer->GetID();
    }

    // AttrFwLpKill

    //  log faction kill in dynamic data   -allan
    MapDB::AddKill(locationID);
    MapDB::AddFactionKill(locationID);

    // set up basic wreck data
    GPoint wreckPosition = m_destiny->GetPosition();
    if (wreckPosition.isNaN()) {
        sLog.Error("Ship::Killed()", "Wreck Position is NaN");
        return;
    }
    uint32 wreckTypeID = sDataMgr.GetWreckID(m_self->typeID());
    if (!IsWreckTypeID(wreckTypeID)) {
        sLog.Error("Ship::Killed()", "Could not get wreckType for %s of type %u", m_self->name(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    std::string wreck_name = m_self->itemName() + " Wreck";

    if (!m_self->HasPilot()) {
        // Spawn a wreck for the Ship that was destroyed:
        ItemData wreckItemData(wreckTypeID, killerID, locationID, flagNone, wreck_name.c_str(), wreckPosition, itoa(m_allyID));
        WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
        if (wreckItemRef.get() == nullptr) {
            sLog.Error("Ship::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
            return;
        }

        if (is_log_enabled(PHYSICS__TRACE))
            _log(PHYSICS__TRACE, "Ship::Killed() - Ship %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
                    GetName(), GetID(), x(), y(), z(), wreckItemRef->name(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);

        DBSystemDynamicEntity wreckEntity = DBSystemDynamicEntity();
            wreckEntity.allianceID = killer->GetAllianceID();
            wreckEntity.categoryID = EVEDB::invCategories::Celestial;
            wreckEntity.corporationID = killer->GetCorporationID();
            wreckEntity.factionID = sDataMgr.GetWreckFaction(wreckTypeID);
            wreckEntity.groupID = EVEDB::invGroups::Wreck;
            wreckEntity.itemID = wreckItemRef->itemID();
            wreckEntity.itemName = wreck_name;
            wreckEntity.ownerID = killerID;
            wreckEntity.typeID = wreckTypeID;
            wreckEntity.position = wreckPosition;

        if (!m_system->BuildDynamicEntity(wreckEntity, m_self->itemID())) {
            sLog.Error("Ship::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
            ; /** @todo make error msg here */  //  PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
            wreckItemRef->Delete();
            return;
        }

        m_destiny->SendJettisonPacket();
        // wreck was created successfully.  drop loot and add to wreck.
        DropLoot(wreckItemRef, m_self->groupID(), killerID);

        return;
    }

    Client* pPilot(m_self->GetPilot());
    if (pPilot == nullptr)
        return;    //  make error here

    if (pClient != nullptr)
        if (m_system->GetSystemSecurityRating() > 0) {
            /* http://www.eveinfo.net/wiki/ind~4067.htm
             *  relative_sec_status_penalty = base_penalty * system_truesec * (1 + (victim_sec_status - agressor_sec_status) / 90)
             *  The actual drop in security status seen by the attacker is a function of their current security status and the relative penalty:
             *  security status loss = relative_penalty * (agressor_sec_status + 10)
             */
            /** @todo (allan) check for faction/corp status modifiers here. */
            double modifier = (1 + ((pPilot->GetSecurityRating() - pClient->GetSecurityRating()) / 90));
            double penalty = 6.0f * m_system->GetSystemSecurityRating() * modifier;
            double loss = penalty * ( pClient->GetSecurityRating() + 10);
            loss *= sConfig.rates.secRate;
            pClient->GetChar()->secStatusChange( loss );
        }

    /* populate kill data for killMail and save to db  -allan 01May16  --updated 13July17 */
    CharKillData data = CharKillData();
        data.solarSystemID = m_system->GetID();
        data.victimCharacterID = pPilot->GetCharacterID();
        data.victimCorporationID = m_corpID;
        data.victimAllianceID = m_allyID;
        data.victimFactionID = m_warID;
        data.victimShipTypeID = m_self->typeID();

        data.finalCharacterID = killerID;
        data.finalCorporationID = killer->GetCorporationID();
        data.finalAllianceID = killer->GetAllianceID();
        data.finalFactionID = (killer->GetWarFactionID() > 500021 ? 500021 : killer->GetWarFactionID());
        data.finalShipTypeID = killer->GetTypeID();
        data.finalWeaponTypeID = fatal_blow.weaponRef->typeID();
        data.finalSecurityStatus = 0;   /* fix this */
        data.finalDamageDone = fatal_blow.GetTotal();

        uint32 totalHP = m_self->GetAttribute(AttrHP).get_int();
            totalHP += m_self->GetAttribute(AttrArmorHP).get_int();
            totalHP += m_self->GetAttribute(AttrShieldCapacity).get_int();
        data.victimDamageTaken = totalHP;

    std::stringstream blob;
    std::vector<InventoryItemRef> survivedItems;
    if (pPilot->InPod()) {
        blob << "<items><i t=" << data.victimShipTypeID << " f=0 s=1 d=0 x=1/></items>";
    } else {
        AbortCycle();
        AbandonDrones();

        // remove all charges (per packet data)  ...why???
        //GetShipItemRef()->UnloadAllModules();

        blob << "<items>";
        /* killBlob contains destroyed/dropped items. u'<items><i t=3651 f=0 d=0 x=1/><i t=3634 f=0 d=0 x=1/></items>'  -allan 13July17
            " i*" tag is decoded as follows:
                t = item.typeID
                f = item.flag
                s = item.singleton
                d = item.qtyDropped
                x = item.qtyDestroyed
        */
        std::map<uint32, InventoryItemRef> deadShipInventory;
        deadShipInventory.clear();
        GetShipItemRef()->GetMyInventory()->GetInventoryMap(deadShipInventory);
        if (deadShipInventory.empty()) {
            blob << "<i t=" << data.victimShipTypeID << " f=0 s=1 d=0 x=1/>";
        } else {
            uint32 s = 0, d = 0, x = 0;
            for (auto cur : deadShipInventory) {
                d = 0;
                x = cur.second->quantity();
                s = (cur.second->isSingleton() ? 1 : 0);
                if (cur.second->categoryID() == EVEDB::invCategories::Blueprint) {
                    // singleton for bpo = 1, bpc = 2.
                    BlueprintRef bpRef = BlueprintRef::StaticCast(cur.second);
                    s = (bpRef->copy() ? 2 : s);
                }

                blob << "<i t=" << cur.second->typeID() << " f=" << cur.second->flag() << " s=" << s ;
                // all contained items have 50% chance of drop, except rigs, which do not survive
                // todo:  add damage to item, if applicable, from ship explosion
                //cur.second->SetAttribute(AttrDamage, 5);
                if (IsRigSlot(cur.second->flag())) {
                    /* just avoiding survive check */;
                } else if (IsEven(MakeRandomInt(0, 100))) {
                    // item survived.  check qty for drop
                    if (x > 1) {
                        d = MakeRandomInt(0, x);
                        x -= d;
                    }
                    // move item to vector for insertion into wreck later on
                    survivedItems.push_back(cur.second);
                }
                blob << " d=" << d << " x=" << x << "/>";
            }
        }
        blob << "</items>";
    }

    data.killBlob = blob.str().c_str();
    data.killTime = GetFileTimeNow();
    data.moonID = 0;

    pPilot->GetChar()->LogKill(data);

    if (pPilot->InPod()) {
        // log podKill
        MapDB::AddPodKill(locationID);

        if (pClient != nullptr)
            pClient->GetChar()->PayBounty(pPilot->GetChar());

        std::string corpse_name = pPilot->GetName();
        corpse_name += "'s Frozen Corpse";
        uint32 corpseTypeID = 10041; // typeID from 'invTypes' table for "Frozen Corpse"
        ItemData corpseItemData(corpseTypeID, m_ownerID, locationID, flagNone, corpse_name.c_str(), wreckPosition);
        InventoryItemRef corpseItemRef = sItemFactory.SpawnItem( corpseItemData );
        if (corpseItemRef.get() == nullptr) {
            sLog.Error("Ship::Killed()", "Creating Corpse Item Failed for %s of type %u", corpse_name.c_str(), corpseTypeID);
            DBSystemDynamicEntity corpseEntity = DBSystemDynamicEntity();
            corpseEntity.allianceID = m_allyID;
            corpseEntity.categoryID = EVEDB::invCategories::Celestial;
            corpseEntity.corporationID = m_corpID;
            corpseEntity.factionID = m_warID;
            corpseEntity.groupID = EVEDB::invGroups::Biomass;
            corpseEntity.itemID = corpseItemRef->itemID();
            corpseEntity.itemName = corpse_name;
            corpseEntity.ownerID = 1;   //would this be 'owned' by killer?
            corpseEntity.typeID = corpseTypeID;
            corpseEntity.position = wreckPosition;
            if (!m_system->BuildDynamicEntity(corpseEntity)) {
                sLog.Error("Ship::Killed()", "Spawning Corpse Failed: typeID or typeName not supported: '%u'", corpseTypeID);
            } else if (is_log_enabled(PHYSICS__TRACE)) {
                _log(PHYSICS__TRACE, "Ship::Killed() - Pod %s(%u) Position: %.2f,%.2f,%.2f.  Corpse %s(%u) Position: %.2f,%.2f,%.2f.", \
                    GetName(), GetID(), x(), y(), z(), corpseItemRef->name(), corpseItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);
            }
        }

        // this method will reset char variables to last clone state after being podded.  NOTE  *** NOT TESTED YET ***
        pPilot->ResetAfterPodded();
    } else {
        PayInsurance();

        m_destiny->SendJettisonPacket();

        uint16 groupID = m_self->groupID();
        GPoint podPosition(wreckPosition);
        podPosition.MakeRandomPointOnSphere(GetShipItemRef()->radius() + pPilot->GetPod()->radius() + MakeRandomFloat(100, 200));
        // this resets client ship data
        pPilot->ResetAfterPopped(podPosition);

        ItemData wreckItemData(wreckTypeID, pPilot->GetCharacterID(), locationID, flagNone, wreck_name.c_str(), wreckPosition, itoa(m_allyID));
        WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
        if (wreckItemRef.get() == nullptr) {
            sLog.Error("Ship::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
            return;
        }

        if (is_log_enabled(PHYSICS__TRACE))
            _log(PHYSICS__TRACE, "Ship::Killed() - Ship %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
            GetName(), GetID(), x(), y(), z(), wreckItemRef->name(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);

        DBSystemDynamicEntity wreckEntity = DBSystemDynamicEntity();
            wreckEntity.allianceID = killer->GetAllianceID();
            wreckEntity.categoryID = EVEDB::invCategories::Celestial;
            wreckEntity.corporationID = killer->GetCorporationID();
            wreckEntity.factionID = sDataMgr.GetWreckFaction(wreckTypeID);
            wreckEntity.groupID = EVEDB::invGroups::Wreck;
            wreckEntity.itemID = wreckItemRef->itemID();
            wreckEntity.itemName = wreck_name;
            wreckEntity.ownerID = pPilot->GetCharacterID();
            wreckEntity.typeID = wreckTypeID;
            wreckEntity.position = wreckPosition;

        if (!m_system->BuildDynamicEntity(wreckEntity, m_self->itemID())) {
            sLog.Error("Ship::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
            wreckItemRef->Delete();
            return;
        }

        DropLoot(wreckItemRef, groupID, killerID);

        for (auto cur: survivedItems)
            cur->Move(wreckItemRef->itemID(), flagNone); // populate wreck with items that survived
    }
}

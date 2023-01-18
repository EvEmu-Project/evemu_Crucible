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

#include "eve-server.h"

#include "ConsoleCommands.h"
#include "Client.h"
#include "Container.h"
#include "EVEServerConfig.h"

#include "StatisticMgr.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "planet/CustomsOffice.h"
#include "planet/Planet.h"
#include "pos/Structure.h"
#include "standing/StandingMgr.h"
#include "system/DestinyManager.h"
#include "station/Station.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"



SystemEntity::SystemEntity(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: m_self(self),
m_services(services),
m_system(system),
m_bubble(nullptr),
m_destiny(nullptr),
m_targMgr(nullptr),
m_killed(false)
{
    assert(m_system != nullptr);
    assert(m_self.get() != nullptr);

    m_warID = 0;
    m_allyID = 0;
    m_corpID = 0;
    m_fleetID = 0;
    m_ownerID = 1;

    m_radius = m_self->GetAttribute(AttrRadius).get_double();

    m_harmonic = EVEPOS::Harmonic::Inactive;

    _log(SE__DEBUG, "Created SE for item %s (%u) with radius of %.1f.", self->name(), self->itemID(), m_radius);
}

// copy c'tor
SystemEntity::SystemEntity(const SystemEntity* oth) : m_self(oth->m_self),m_services(oth->m_services),m_system(oth->m_system),
m_bubble(oth->m_bubble),m_destiny(oth->m_destiny),m_targMgr(oth->m_targMgr),m_killed(oth->m_killed),m_warID(oth->m_warID),
m_allyID(oth->m_allyID),m_corpID(oth->m_corpID),m_fleetID(oth->m_fleetID),m_ownerID(oth->m_ownerID),m_radius(oth->m_radius),
m_harmonic(oth->m_harmonic)
{
    sLog.Error("SE::SE()", "copy c'tor.");
    // wip
}


void SystemEntity::Process() {
    if (m_killed) {
        _log(SE__DEBUG, "SE::Process() - %s(%u) is dead but still in system.", m_self->name(), m_self->itemID());
        return;
    }

    if (m_destiny != nullptr)
        m_destiny->Process();
}

PyTuple* SystemEntity::MakeDamageState() {
    if (IsWreckSE()) {
        DoDestinyDamageState3 ddds;
            ddds.shield = 0;
            ddds.armor = 0;
            ddds.structure = 1.0;
        return ddds.Encode();
    }
    DoDestinyDamageState ddds;
    MakeDamageState(ddds);
    return ddds.Encode();
}

void SystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1;
    into.recharge = 110000;
    into.armor = 1;
    into.structure = 1;
    into.timestamp = GetFileTimeNow();
}

PyDict* SystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for SE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_ownerID));
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
    return slim;
}

void SystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head = BallHeader();
        head.entityID = m_self->itemID();
        head.mode = Ball::Mode::RIGID;
        head.radius = m_radius;
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsGlobal;
    into.Append( head );
    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(SE__DESTINY, "SE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void SystemEntity::Killed(Damage& damage)
{
    if (m_targMgr != nullptr) {
        // loop thru list of all modules targeting this entity and let them know it has been killed.
        m_targMgr->Destroyed();
        // remove TargMgr here to avoid redundant calls upon object deletion
        SafeDelete(m_targMgr);
    }
    Delete();
}

void SystemEntity::Delete()
{
    if (m_targMgr != nullptr)
        m_targMgr->ClearFromTargets(); //OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::Deleted);
    if (m_system != nullptr)
        m_system->RemoveEntity(this);
    // containers have additional calls to process and calls this.  calling here will create infinite loop
    if (!IsContainerSE())
        m_self->Delete();
}

double SystemEntity::DistanceTo2(const SystemEntity* other) {
    if (other->m_bubble == nullptr)
        return 1000000.0;
    return GetPosition().distance(other->GetPosition());
}

void SystemEntity::SendDamageStateChanged() {  //working 24Apr15
     DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        dmgState.timestamp = GetFileTimeNow();
        dmgState.armor = (1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double()));
        dmgState.structure = (1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double()));
     OnDamageStateChange dmgChange;
        dmgChange.entityID = m_self->itemID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    if (m_targMgr != nullptr)
        m_targMgr->QueueUpdate(&up);
    PySafeDecRef(up);
    _log(DAMAGE__MESSAGE, "%s(%u): DamageUpdate - S:%f A:%f H:%f.", \
            m_self->name(), m_self->itemID(), dmgState.shield, dmgState.armor, dmgState.structure);
}

void SystemEntity::DropLoot(WreckContainerRef wreckRef, uint32 groupID, uint32 owner) {
    /*   allan 27Nov14    */
    std::vector<LootList> lootList;
    sDataMgr.GetLoot(groupID, lootList);
    if (lootList.empty()) {
        _log(LOOT__INFO, "lootList empty for %s(%u)", m_self->name(), m_self->itemID());
        return;
    }

    uint32 quantity(0);
    std::vector<LootList>::iterator itr = lootList.begin();
    while (itr != lootList.end()) {
        if (itr->minDrop == itr->maxDrop) {
            quantity = itr->minDrop;
        } else {
            quantity = (uint32)(MakeRandomInt(itr->minDrop, itr->maxDrop));
        }
        if (quantity == 0)
            quantity = 1;

        ItemData iLoot(itr->itemID, owner, wreckRef->itemID(), flagNone, quantity);
        wreckRef->AddItem(sItemFactory.SpawnItem(iLoot));
        _log(LOOT__INFO, "added %u of %u to list for %s(%u)", quantity, itr->itemID, m_self->name(), m_self->itemID());
        ++itr;
    }
}

/** @todo (allan)  this doesnt need to be here */
void SystemEntity::AwardSecurityStatus(InventoryItemRef iRef, Character* pChar) {
    //New Status = ((10 - Old Status) * Sec Incr) + Old Status
    double oldSec = pChar->GetSecurityRating();
    EvilNumber maxGain = 0;
    if (iRef->HasAttribute(AttrEntitySecurityMaxGain, maxGain))
        if (oldSec > maxGain.get_double())
            return;
    float killBonus = iRef->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float();
    double secAward = (((10 - oldSec) * killBonus) + oldSec) / 100;
    secAward *=  (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::FastTalk, true))));      // 5% increase
    if (killBonus and secAward) {
        secAward *= sConfig.rates.secRate;
        sLog.Magenta("SE::AwardSecurityStatus()"," %s(%u): killBonus: %f.  oldSec: %f.  secAward: %f.",
                     GetName(), iRef->itemID(), killBonus, oldSec, secAward);
        pChar->secStatusChange( secAward );
        std::string msg = "Status Change for killing";
        if (iRef->HasPilot()) {
            msg += iRef->GetPilot()->GetName();
            msg += " in ";
            msg += m_system->GetNameStr();
            sStandingMgr.UpdateStandings(iRef->itemID(), pChar->itemID(), Standings::CombatShipKill, secAward, msg);
        } else {
            msg += " pirates in ";
            msg += m_system->GetNameStr();
            sStandingMgr.UpdateStandings(corpCONCORD, pChar->itemID(), Standings::LawEnforcement, secAward, msg);
            // decrease standings with faction of this npc kill
            sStandingMgr.UpdateStandings(iRef->ownerID(), pChar->itemID(), Standings::CombatShipKill, -secAward, msg);
        }
    }

    /** @todo msg need work for details to appear correctly.  currently working, but could be better. (incomplete, but working)
     * see data in eve/common/script/util/eveFormat.py:300 for details
     *
     *
     * SE::AwardSecurityStatus():  Shadow Serpentis Port Admiral(750000006): killBonus: 0.072000.  oldSec: 0.044933.  secAward: 0.007617.
     * SE::AwardSecurityStatus():               Estamel Tharchon(750000001): killBonus: 0.244141.  oldSec: 0.000000.  secAward: 0.024414.
     *
     */
}

void SystemEntity::Abandon()
{
    m_warID = 0;
    m_allyID = 0;
    m_corpID = 0;
    m_fleetID = 0;
    m_ownerID = 1;
    m_self->ChangeOwner(1); // update this to use system owner?    not sure.  logs show this as "1" for all non-player items
    /** @todo  should this have a slimupdate or bubblecast or something?  */
}


/* Static / Non-Mobile / Non-Destructable / Celestial Objects - Suns, Planets, Moons, Belts, Gates, Stations */
StaticSystemEntity::StaticSystemEntity(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: SystemEntity(self, services, system)
{
}

// copy c'tor
StaticSystemEntity::StaticSystemEntity(const StaticSystemEntity* oth)
: SystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("SSE::SSE()", "copy c'tor.");
    // wip
}

bool StaticSystemEntity::LoadExtras() {
    return true;
}

PyDict* StaticSystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for SSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       PyStatic.NewNone());
        slim->SetItemString("ownerID",      PyStatic.NewOne());
    return slim;
}

void StaticSystemEntity::EncodeDestiny( Buffer& into ) {
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = m_self->itemID();
        head.mode = Ball::Mode::RIGID;
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.radius = m_radius;
        head.flags = Ball::Flag::IsGlobal;
    into.Append( head );
    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(SE__DESTINY, "SSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X, radius:%.1f", GetName(), head.entityID, head.mode, head.flags, head.radius);
}

BeltSE::BeltSE(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

// copy c'tor
BeltSE::BeltSE(const BeltSE* oth)
: StaticSystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("SSE::BeltSE()", "copy c'tor.");
    // wip
}

bool BeltSE::LoadExtras() {
    if (!StaticSystemEntity::LoadExtras())
        return false;

    if (m_bubble == nullptr)
        sBubbleMgr.Add(this);

    m_bubble->SetBelt(m_self);
    _log(DESTINY__BUBBLE_DEBUG, "BeltSE::LoadExtras() - IsBelt set to true for bubble %u.", m_bubble->GetID() );
    return true;
}

StargateSE::StargateSE(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: StaticSystemEntity(self, services, system),
m_sbuSE(nullptr)
{
}

// copy c'tor
StargateSE::StargateSE(const StargateSE* oth)
: StaticSystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("SSE::gateSE()", "copy c'tor.");
    // wip
}

bool StargateSE::LoadExtras() {
    if (!StaticSystemEntity::LoadExtras())
        return false;

    if (m_bubble == nullptr)
        sBubbleMgr.Add(this);

    m_bubble->SetGate(true);
    _log(DESTINY__BUBBLE_DEBUG, "StargateSE::LoadExtras() - IsGate set to true for bubble %u.", m_bubble->GetID() );
    m_jumps = SystemDB::ListJumps(m_self->itemID());
    if (m_jumps != nullptr)
        return true;

    return false;
}

PyDict* StargateSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StargateSE %s(%u)", GetName(), m_self->itemID());
    /** @todo  finish gate rotation data
    PyTuple* rotation = new PyTuple(3);
        rotation->SetItem(0, new PyFloat(0));
        rotation->SetItem(1, new PyFloat(0));
        rotation->SetItem(2, new PyFloat(0));*/
    PyDict *slim = new PyDict();
        //slim->SetItemString("dunRotation", rotation);
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        /** @todo (allan) make function to lookup controlling faction id for this */
        //  NOTE:  maybe not...logs show this is "1" for all items.
        slim->SetItemString("ownerID",      PyStatic.NewOne());
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       PyStatic.NewNone());
    if (m_jumps != nullptr)
        slim->SetItemString("jumps", m_jumps->Clone());
    return slim;
}


/* Non-Static / Non-Mobile / Non-Destructable / Celestial Objects - Containers, DeadSpace, ForceFields, ScanProbes */
ItemSystemEntity::ItemSystemEntity(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: SystemEntity(self, services, system),
m_keyType(0)
{
}

// copy c'tor
ItemSystemEntity::ItemSystemEntity(const ItemSystemEntity* oth)
: SystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("ISE::ISE()", "copy c'tor.");
    // wip
}

PyDict* ItemSystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for ISE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_ownerID));
        if (m_self->groupID() == EVEDB::invGroups::Warp_Gate) {
            // this is incomplete........
            slim->SetItemString("dunSkillLevel", PyStatic.NewNone());   //?
            slim->SetItemString("dunSkillTypeID", PyStatic.NewNone());   //?
            slim->SetItemString("dunObjectID", new PyInt(160449));  //?   902139
            slim->SetItemString("dunToGateID", new PyInt(160484));  //?   902140
            slim->SetItemString("dunCloaked", new PyBool(0));   //?
            slim->SetItemString("dunScenarioID", new PyInt(23));    //?  3347
            slim->SetItemString("dunSpawnID", new PyInt(1572));  //?
            slim->SetItemString("dunAmount", new PyFloat(0.0));  //?
            PyList* classList = new PyList();
                classList->AddItem( new PyInt(324));
                classList->AddItem( new PyInt(420));
                classList->AddItem( new PyInt(541));
                classList->AddItem( new PyInt(834));
                classList->AddItem( new PyInt(25));
                classList->AddItem( new PyInt(830));
            slim->SetItemString("dunShipClasses", classList);   //?
            PyList* dirList = new PyList();
                dirList->AddItem(new PyInt(5));     //234
                dirList->AddItem(new PyInt(-1));
                dirList->AddItem(PyStatic.NewZero());
            slim->SetItemString("dunDirection", dirList);
            slim->SetItemString("dunKeyLock", PyStatic.NewNone());   //?
            slim->SetItemString("dunWipeNPC", new PyBool(0));   //?
            slim->SetItemString("dunKeyQuantity", PyStatic.NewOne());   //?
            slim->SetItemString("dunKeyTypeID", new PyInt(m_keyType));   //Training Complex Passkey   group Acceleration_Gate_Keys
            slim->SetItemString("dunOpenUntil", new PyInt(Win32TimeNow()+EvE::Time::Hour));   //?
            slim->SetItemString("dunRoomName", new PyString("Lobby"));   //?
            slim->SetItemString("dunMusicUrl", new PyString("res:/Sound/Music/Ambient031combat.ogg"));
        }
    /** @todo  finish rotation data
    Large_Collidable_Structure
    Large_Collidable_Ship
    Large_Collidable_Object
    PyTuple* rotation = new PyTuple(3);
        rotation->SetItem(0, new PyFloat(0));
        rotation->SetItem(1, new PyFloat(0));
        rotation->SetItem(2, new PyFloat(0));
    slim->SetItemString("dunRotation", rotation);
    */
    return slim;
}

void ItemSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = m_self->itemID();
        head.mode = Ball::Mode::RIGID;
        head.radius = m_radius;
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = 0;
    into.Append( head );
    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "ISE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void ItemSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    if (m_self->groupID() == EVEDB::invGroups::Force_Field) {
        SystemEntity::MakeDamageState(into);
    } else {
        into.shield = (m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
        into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        into.timestamp = GetFileTimeNow();
        into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
        into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
    }
}

FieldSE::FieldSE(InventoryItemRef self, EVEServiceManager &services, SystemManager *system, const FactionData& data)
: ItemSystemEntity(self, services, system)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
}

// copy c'tor
FieldSE::FieldSE(const FieldSE* oth)
: ItemSystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("ISE::FieldSE()", "copy c'tor.");
    // wip
}

void FieldSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = m_self->itemID();
        head.mode = (m_harmonic > EVEPOS::Harmonic::Offline ? Ball::Mode::FIELD : Ball::Mode::STOP);
        head.radius = m_radius;
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = 0 /*(m_harmonic > EVEPOS::Harmonic::Offline ? Ball::Flag::IsMassive : 0)*/; // leave this as 0 to disable client-side bump checks for now
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = 10000000000;    // as seen in packets
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAlliance(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    if (head.mode == Ball::Mode::FIELD) {
        FIELD_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    } else if (head.mode == Ball::Mode::STOP) {
        STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    }

    _log(SE__DESTINY, "FSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *FieldSE::MakeSlimItem()
{
    return SystemEntity::MakeSlimItem();
}


/* Non-Static / Non-Mobile / Destructible / Celestial Objects - POS Structures, Outposts, Deployables, empty Ships, Asteroids */
ObjectSystemEntity::ObjectSystemEntity(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: SystemEntity(self, services, system),
m_invul(false)
{
    m_targMgr = new TargetManager(this);
    m_destiny = new DestinyManager(this);

    assert(m_targMgr != nullptr);
    assert(m_destiny != nullptr);
}

// copy c'tor
ObjectSystemEntity::ObjectSystemEntity(const ObjectSystemEntity* oth)
: SystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("OSE::OSE()", "copy c'tor.");
    // wip
}

ObjectSystemEntity::~ObjectSystemEntity()
{
    if (m_targMgr != nullptr)
        if (!sConsole.IsShutdown()) {
            m_targMgr->ClearModules();
            m_targMgr->ClearAllTargets(false);
            //m_targMgr->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::Destroyed);
        }

    SafeDelete(m_targMgr);
    SafeDelete(m_destiny);
}

void ObjectSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = m_self->itemID();
        head.mode = Ball::Mode::RIGID;
        head.radius = m_radius;
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsMassive;
    into.Append( head );
    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "OSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict* ObjectSystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for OSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(GetTypeID()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    return slim;
}

void ObjectSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
}

void ObjectSystemEntity::UpdateDamage()
{
    /** @todo (Allan) needs more work */
    SystemEntity::UpdateDamage();
     DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        dmgState.timestamp = GetFileTimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double();
     OnDamageStateChange dmgChange;
        dmgChange.entityID = m_self->itemID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
    PySafeDecRef(up);
}

void ObjectSystemEntity::Killed(Damage& damage)
{
    // do we need to make wreck items here?
    // do these structures have loot?  probably so eventually

    /** @todo  test and complete this to null current customs office for this planet ... */
    if (IsCOSE()) {
        if (GetCOSE()->GetPlanetID() > 0) {
            SystemEntity* pSE = m_system->GetSE(GetCOSE()->GetPlanetID());
            pSE->GetPlanetSE()->SetCustomsOffice(nullptr);
        }
    }
}

DeployableSE::DeployableSE(InventoryItemRef self, EVEServiceManager &services, SystemManager *system, const FactionData& data)
: ObjectSystemEntity(self, services, system)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
}

// copy c'tor
DeployableSE::DeployableSE(const DeployableSE* oth)
: ObjectSystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("OSE::DeployableSE()", "copy c'tor.");
    // wip
}


/* Non-Static / Mobile / Destructible / Celestial Objects - PC's, NPC's, Drones, Ships, Missiles, Wrecks  */
DynamicSystemEntity::DynamicSystemEntity(InventoryItemRef self, EVEServiceManager &services, SystemManager* system)
: SystemEntity(self, services, system),
m_invul(false),
m_frozen(false)
{
    m_targMgr = new TargetManager(this);
    m_destiny = new DestinyManager(this);

    assert(m_targMgr != nullptr);
    assert(m_destiny != nullptr);
}

// copy c'tor
DynamicSystemEntity::DynamicSystemEntity(const DynamicSystemEntity* oth)
: SystemEntity(oth->m_self, oth->m_services, oth->m_system)
{
    sLog.Error("DSE::DSE()", "copy c'tor.");
    // wip
}

DynamicSystemEntity::~DynamicSystemEntity()
{
    if (m_targMgr != nullptr)
        if (!sConsole.IsShutdown()) {
            m_targMgr->ClearModules();
            m_targMgr->ClearAllTargets(false);
            //m_targMgr->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::Destroyed);
        }

    SafeDelete(m_targMgr);
    SafeDelete(m_destiny);
}

PyDict *DynamicSystemEntity::MakeSlimItem() {
    if (IsNPCSE())
        return SystemEntity::MakeSlimItem();

    _log(SE__SLIMITEM, "MakeSlimItem for DSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        //slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        //slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    return (slim);
}

void DynamicSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = m_self->itemID();
        head.mode = Ball::Mode::STOP;
        head.radius = m_radius;
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsFree;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_destiny->GetMass();
        mass.cloak = (m_destiny->IsCloaked() ? 1 : 0);
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAlliance(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    DataSector data = DataSector();
        data.inertia = m_destiny->GetInertia();
        data.maxSpeed = m_destiny->GetMaxVelocity();
        data.velX = m_destiny->GetVelocity().x;
        data.velY = m_destiny->GetVelocity().y;
        data.velZ = m_destiny->GetVelocity().z;
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    STOP_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "DSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DynamicSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
}

void DynamicSystemEntity::UpdateDamage()
{
    /** @todo (Allan) needs more work */
    SystemEntity::UpdateDamage();
     DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        dmgState.timestamp = GetFileTimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double();
     OnDamageStateChange dmgChange;
        dmgChange.entityID = m_self->itemID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
    PySafeDecRef(up);
}

void DynamicSystemEntity::AwardBounty(Client* pClient)
{
    // this will use a map{charID/BountyData} in system manager for using a bounty timer.
    double bounty = m_self->GetAttribute(AttrEntityKillBounty).get_double();
    bounty *= sConfig.rates.npcBountyMultiply;
    if (bounty < 1)
        return;

    // add data to StatisticMgr
    sStatMgr.Add(Stat::npcBounties, bounty);

    std::string reason = "Bounty for killing a pirate in ";
    reason += pClient->GetSystemName();

    BountyData data = BountyData();
    data.fromID = m_self->itemID();
    data.toID = pClient->GetCharacterID();
    data.refTypeID = Journal::EntryType::BountyPrize;
    data.fromKey = Account::KeyType::Cash;
    data.toKey = Account::KeyType::Cash;
    data.reason = reason;

    // handle distribution to fleets
    if (pClient->InFleet()) {
        // get fleet members onGrid and distrubute bounty
        std::vector< uint32 > members;
        sFltSvc.GetFleetMembersOnGrid(pClient, members);
        // split bounty between members
        bounty /= members.size();
        // send bounty to members
        if (sConfig.server.BountyPayoutDelayed and sConfig.server.FleetShareDelayed) {
            for (auto cur :members)
                m_system->AddBounty(cur, data);
        } else {
            reason += " (FleetShare) ";
            reason += " by ";
            reason += pClient->GetName();
            data.reason = reason;
            for (auto cur :members)
                AccountService::TranserFunds(corpCONCORD, cur, bounty, reason.c_str(), Journal::EntryType::BountyPrize, -GetTypeID());
        }
    } else {
        data.amount = bounty;
        if (sConfig.server.BountyPayoutDelayed) {
            m_system->AddBounty(pClient->GetCharacterID(), data);
        } else {
            AccountService::TranserFunds(corpCONCORD, pClient->GetCharacterID(), bounty, reason.c_str(), Journal::EntryType::BountyPrize, -GetTypeID());
        }
    }
}

DungeonEditSE::DungeonEditSE(InventoryItemRef self, EVEServiceManager& services, SystemManager* system, Dungeon::RoomObject data)
: ObjectSystemEntity(self, services, system),
    m_data(data)
{
}

PyDict* DungeonEditSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for DungeonEditSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID", new PyLong(m_self->itemID()));
        slim->SetItemString("typeID", new PyInt(m_self->typeID()));
        slim->SetItemString("groupID", new PyInt(m_self->groupID()));
        slim->SetItemString("dunObjectID", new PyInt(m_self->itemID()));
        slim->SetItemString("dunRadius", new PyFloat(m_data.radius));
        slim->SetItemString("dunRoomID", new PyInt(m_data.roomID));
        slim->SetItemString("dunX", new PyFloat(m_data.x));
        slim->SetItemString("dunY", new PyFloat(m_data.y));
        slim->SetItemString("dunZ", new PyFloat(m_data.z));

    return slim;
}
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
    Author:     Aknor Jaden
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "planet/PlanetDB.h"
#include "system/DestinyManager.h"
#include "system/Container.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/SpawnMgr.h"

/*
 * CargoContainer
 */
CargoContainer::CargoContainer(uint32 _containerID, const ItemType &_containerType, const ItemData &_data)
: InventoryItem(_containerID, _containerType, _data),
mySE(nullptr),
m_isAnchored(false)
{
    pInventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created CargoContainer object for item %s (%u).", name(), m_itemID);
}

CargoContainer::~CargoContainer()
{
    if (pInventory != nullptr)
        pInventory->Unload();
    SafeDelete(pInventory);
}

CargoContainerRef CargoContainer::Load( uint32 containerID)
{
    return InventoryItem::Load<CargoContainer>(containerID );
}

bool CargoContainer::_Load() {
    if (!pInventory->LoadContents())
        return false;

    return InventoryItem::_Load();
}

CargoContainerRef CargoContainer::Spawn( ItemData &data) {
    uint32 containerID = CargoContainer::CreateItemID(data );
    if (containerID == 0 )
        return CargoContainerRef(nullptr);
    CargoContainerRef containerRef = CargoContainer::Load(containerID );

    // Create default dynamic attributes in the AttributeMap:
    containerRef->SetAttribute(AttrRadius,        containerRef->type().radius(), false);			// Radius
    containerRef->SetAttribute(AttrShieldCharge,  containerRef->GetAttribute(AttrShieldCapacity), false);  // Shield Charge
    containerRef->SetAttribute(AttrArmorDamage,   EvilZero, false);                                               // Armor Damage
    containerRef->SetAttribute(AttrMass,          containerRef->type().mass(), false);          // Mass
    containerRef->SetAttribute(AttrVolume,        containerRef->GetPackagedVolume(), false);        // Volume
    containerRef->SetAttribute(AttrCapacity,      containerRef->type().capacity(), false);      // Capacity

    return containerRef;
}

CargoContainerRef CargoContainer::SpawnTemp(ItemData& data)
{
    uint32 containerID = CargoContainer::CreateTempItemID(data);
    if (containerID == 0)
        return CargoContainerRef(nullptr);

    if (data.quantity == 0)
        return CargoContainerRef(nullptr);

    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr)
        return CargoContainerRef(nullptr);

    CargoContainerRef cRef = CargoContainerRef(new CargoContainer(containerID, *iType, data));
    /* we dont need attribs or inventory here....these are position markers
    if (cRef.get() != nullptr)
        cRef->_Load();
    */

    return cRef;
}

uint32 CargoContainer::CreateItemID( ItemData &data)
{
    return InventoryItem::CreateItemID(data);
}

void CargoContainer::Delete()
{
    if (typeID() == EVEDB::invTypes::PlanetaryLaunchContainer)
        PlanetDB::DeleteLaunch(m_itemID);

    // if SE exists, remove from system before deleting item
    if (mySE != nullptr)
        mySE->Delete();

    pInventory->LoadContents();
    pInventory->DeleteContents();
    InventoryItem::Delete();
}

double CargoContainer::GetCapacity(EVEItemFlags flag) const
{
    return pInventory->GetCapacity(flag);
}

void CargoContainer::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const {
    pInventory->ValidateAddItem(flag, item);
}

PyObject *CargoContainer::CargoContainerGetInfo() {
    if (!pInventory->LoadContents( ) ) {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for CargoContainerGetInfo", name(), m_itemID );
        return nullptr;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the CargoContainer.
    if (!Populate(entry))
        return nullptr;    //print already done.

    result.items[ m_itemID ] = entry.Encode();

    return result.Encode();
}

void CargoContainer::RemoveItem(InventoryItemRef iRef)
{
    /** @todo  put check in here for container owner (if space container) and implement sec penalty */
    /* http://www.eveinfo.net/wiki/ind~4067.htm
     *  relative_sec_status_penalty = base_penalty * system_truesec * (1 + (victim_sec_status - agressor_sec_status) / 90)
     *  The actual drop in security status seen by the attacker is a function of their current security status and the relative penalty:
     *  security status loss = relative_penalty * (agressor_sec_status + 10)
     *
    double modifier = (1 + ((GetChar()->GetSecurityRating() - client->GetSecurityRating()) /90));
    double penalty = 6.0f * m_system->GetSystemSecurityRating() * modifier;
    double loss = penalty * (client->GetSecurityRating() + 10);
    client->GetChar()->secStatusChange( loss );
    */

    if (iRef.get() == nullptr)
        return;

    InventoryItem::RemoveItem(iRef);

    if (m_isAnchored)
        return;

    if (pInventory->IsEmpty()) {
        switch (typeID()) {
        case EVEDB::invTypes::CargoContainer:
            sLog.Warning("CargoContainer::RemoveItem()", "Cargo Container %u is empty and being deleted.", m_itemID);
            Delete();
            break;
        case EVEDB::invTypes::PlanetaryLaunchContainer:
            sLog.Warning("CargoContainer::RemoveItem()", "Launch Container %u is empty and being deleted.", m_itemID);
            PlanetDB::UpdateLaunchStatus(m_itemID, PI::Cargo::Claimed);
            Delete();
            break;
        case EVEDB::invTypes::SmallSecureContainer:
        case EVEDB::invTypes::SmallStandardContainer:
        case EVEDB::invTypes::MediumSecureContainer:
        case EVEDB::invTypes::MediumStandardContainer:
        case EVEDB::invTypes::LargeStandardContainer:
        case EVEDB::invTypes::LargeSecureContainer:
            sLog.Warning("CargoContainer::RemoveItem()", "Launch Container %u is empty and being deleted.", m_itemID);
            break;
        default:
            sLog.Warning("CargoContainer::RemoveItem()", "Non-Cargo Container %u (type: %u) is empty and being deleted.", m_itemID, typeID());
            Delete();
            break;
        }
    }
}

void CargoContainer::MakeDamageState(DoDestinyDamageState &into) const
{
    //FIXME  container attributes here are NOT saved in the db....
    into.shield = 1.0;//(m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
    into.recharge = 10000;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0;//1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
    into.structure = 1.0;
}

/*
 *    def HaveLootRight(self, objectID):
 *        """
 *        This method mirrors logic on server in lootRightsMgr.py HaveLootRight
 *
 *        Returns True if the character has loot rights to a container, otherwise returns False
 *        """
 *        if self.slimItems.has_key(objectID):
 *            slim = self.slimItems[objectID]
 *            if session.charid == slim.ownerID:
 *                return True
 *            lootRights = getattr(slim, 'lootRights', None)
 *            if lootRights is not None:
 *                ownerID, corpID, fleetID, abandoned = lootRights
 *                if abandoned:
 *                    return True
 *                if session.charid == ownerID:
 *                    return True
 *                if not util.IsNPCCorporation(session.corpid) and session.corpid in (ownerID, corpID):
 *                    return True
 *                if session.fleetid is not None and session.fleetid == fleetID:
 *                    return True
 *                if self.broker.crimewatchSvc.CanAttackFreely(slim):
 *                    return True
 *            elif not util.IsNPCCorporation(session.corpid) and session.corpid == slim.ownerID:
 *                return True
 *        return False
 */

ContainerSE::ContainerSE(CargoContainerRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: ItemSystemEntity(self, services, system),
 m_contRef(self),
 m_deleteTimer(0),
 m_global(false),
 m_shieldCharge(0),
 m_armorDamage(0),
 m_hullDamage(0)
{
    m_targMgr = new TargetManager(this);
    m_destiny = new DestinyManager(this);

    assert(m_targMgr != nullptr);
    assert(m_destiny != nullptr);

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    if (!sDataMgr.IsStation(m_self->locationID())) { // should NEVER be true (SE object in station???)
        if (m_self->typeID() == EVEDB::invTypes::PlanetaryLaunchContainer) {
            m_deleteTimer.Start(5 *24 *60 *60 *1000);  //5d timer for PI launch.  should probably get this saved value from planet launches
        } else {
            m_deleteTimer.Start(sConfig.rates.WorldDecay *60 *1000);
        }
    }

    m_self->SetAttribute(AttrCapacity, m_self->type().capacity(), false);
}

ContainerSE::~ContainerSE()
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

void ContainerSE::Process() {
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    if (m_deleteTimer.Check(false)) {
        m_deleteTimer.Disable();
        sLog.Magenta( "ContainerSE::Process()", "Garbage Collection is removing Cargo Container %u.", m_contRef->itemID() );
        m_contRef->Delete();
        delete this;
    }
}

void ContainerSE::Activate(int32 effectID)
{
    // check effectID, check current state, check current timer, set new state, update timer

    /** @todo somehow notify client with one of these effects:
     *  effectAnchorDrop = 649
     *  effectAnchorLift = 650
     *  effectAnchorDropForStructures = 1022
     *  effectAnchorLiftForStructures = 1023
     *
     ** @todo  many more effects to send for.....look into later.
     * effectOnlineForStructures = 901
     *
     ** @note  also note there are timers involved here...
     */
}

void ContainerSE::Deactivate(int32 effectID)
{
    // check effectID, check current state, check current timer, set new state, update timer
}

void ContainerSE::AnchorContainer()
{
    m_deleteTimer.Disable();
    m_contRef->SetAnchor(true);
}

void ContainerSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.mode = Ball::Mode::TROLL;
        head.flags = Ball::Flag::IsFree | Ball::Flag::IsInteractive;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_self->type().mass();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAlliance(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    DataSector data = DataSector();
        data.inertia = 1;
        data.maxSpeed = 1;
        data.velX = 0;
        data.velY = 0;
        data.velZ = 0;
        data.speedfraction = 1;
    into.Append( data );
    TROLL_Struct troll;
        troll.formationID = 0xFF;
        troll.effectStamp = sEntityList.GetStamp();
    into.Append( troll );

    _log(SE__DESTINY, "ContainerSE::EncodeDestiny(): %s - id:%li, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void ContainerSE::MakeDamageState(DoDestinyDamageState &into)
{
    //FIXME  container attributes are NOT saved in the db....
    into.shield = 1;
    into.recharge = 2000000;
    into.timestamp = GetFileTimeNow();
    into.armor = 1;
    into.structure = 1;
}

PyDict *ContainerSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for ContainerSE %s(%u)", m_self->name(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("nameID",           PyStatic.NewNone());
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        if (m_contRef->IsAnchored())        // not sure if this is right...testing
            slim->SetItemString("structureState",       new PyInt(EVEPOS::StructureState::Anchored));

    if (is_log_enabled(DESTINY__DEBUG)) {
        _log( DESTINY__DEBUG, "ContainerSE::MakeSlimItem() - %s(%u)", GetName(), GetID());
        slim->Dump(DESTINY__DEBUG, "     ");
    }

    return slim;
}

void ContainerSE::SendDamageStateChanged() {  //working 24Apr15
    DoDestinyDamageState dmgState;
    MakeDamageState(dmgState);
    OnDamageStateChange dmgChange;
    dmgChange.entityID = m_self->itemID();
    dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    if (m_targMgr != nullptr)
        m_targMgr->QueueUpdate(&up);
    PySafeDecRef(up);
}


/*
 * WreckContainer
 */
WreckContainer::WreckContainer(uint32 _containerID, const ItemType &_containerType, const ItemData &_data)
: InventoryItem(_containerID, _containerType, _data),
mySE(nullptr),
m_delete(false),
m_salvaged(false)
{
    pInventory = new Inventory(InventoryItemRef(this));

    _log(ITEM__TRACE, "Created WreckContainer object for item %s (%u).", name(), itemID());
}

WreckContainer::~WreckContainer()
{
    if (pInventory != nullptr)
        pInventory->Unload();
    SafeDelete(pInventory);
}

WreckContainerRef WreckContainer::Load( uint32 containerID)
{
    return InventoryItem::Load<WreckContainer>(containerID );
}

bool WreckContainer::_Load() {
    if (!pInventory->LoadContents( ) )
        return false;

    return InventoryItem::_Load();
}

WreckContainerRef WreckContainer::Spawn( ItemData &data) {
    uint32 containerID = WreckContainer::CreateItemID(data );
    if (containerID == 0 )
        return WreckContainerRef(nullptr);
    WreckContainerRef wreckRef = WreckContainer::Load(containerID );

    // Create default dynamic attributes in the AttributeMap:
    wreckRef->SetAttribute(AttrShieldCharge,  wreckRef->GetAttribute(AttrShieldCapacity), false);  // Shield Charge
    wreckRef->SetAttribute(AttrArmorDamage,   EvilZero, false);                                               // Armor Damage
    wreckRef->SetAttribute(AttrMass,          wreckRef->type().mass(), false);          // Mass
    wreckRef->SetAttribute(AttrRadius,        wreckRef->type().radius(), false);        // Radius
    wreckRef->SetAttribute(AttrVolume,        wreckRef->type().volume(), false);        // Volume
    wreckRef->SetAttribute(AttrCapacity,      wreckRef->type().capacity(), false);      // Capacity

    return wreckRef;
}

uint32 WreckContainer::CreateItemID( ItemData &data)
{
    return InventoryItem::CreateItemID(data);
}

void WreckContainer::Delete()
{
    // verify this is actually called correctly
    m_delete = true;
    pInventory->LoadContents();
    pInventory->DeleteContents();
    InventoryItem::Delete();
}

double WreckContainer::GetCapacity(EVEItemFlags flag) const
{
    return pInventory->GetCapacity(flag);
}

PyObject *WreckContainer::WreckContainerGetInfo()
{
    if (!pInventory->LoadContents()) {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for WreckContainerGetInfo", name(), itemID() );
        return nullptr;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the WreckContainer.
    if (!Populate( entry ) )
        return nullptr;    //print already done.

    result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void WreckContainer::ValidateAddItem( EVEItemFlags flag, InventoryItemRef item ) const
{
    // throw();
        //  no code here.  should NOT be able to add items to a wreck container.
    //_log(INV__ERROR, "Adding item to wreck.");
    sLog.Error("ValidateAddItem", "Adding item to wreck");
    EvE::traceStack();
}

void WreckContainer::RemoveItem(InventoryItemRef iRef)
{
    if (iRef.get() == nullptr)
        return;

    InventoryItem::RemoveItem(iRef);

    if (m_salvaged)
        return;

    if (pInventory->IsEmpty()) {
        MakeSlimItemChange();
        _log(INV__INFO, "WreckContainer::IsEmpty() for %s(%u)", name(), itemID());
    }
}

void WreckContainer::MakeSlimItemChange()
{
    if (m_delete)
        return;
    if ((mySE == nullptr) or (mySE->SysBubble() == nullptr))
        return;
    PyDict* slimPod = mySE->MakeSlimItem();
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(itemID()));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slimPod));
    PyTuple* updates = new PyTuple(2);
        updates->SetItem(0, new PyString("OnSlimItemChange"));
        updates->SetItem(1, shipData);
    //consumes updates
    mySE->SysBubble()->BubblecastDestinyUpdate(&updates, "destiny" );
}

// wrecks are invul. but shouldnt be.  see todo notes
WreckSE::WreckSE(WreckContainerRef self, PyServiceMgr &services, SystemManager* system, const FactionData &data)
: DynamicSystemEntity(self, services, system),
m_deleteTimer(sConfig.rates.WorldDecay *60 *1000),
m_abandoned(false),
m_contRef(self),
m_launchedByID(0)
{
    assert(m_targMgr != nullptr);
    assert(m_destiny != nullptr);

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    m_self->SetAttribute(AttrCapacity, m_self->type().capacity(), false);
}

WreckSE::~WreckSE()
{
    // these are cleared in base class
    //SafeDelete(m_targMgr);
    //SafeDelete(m_destiny);
}

void WreckSE::Process() {
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    if (m_deleteTimer.Check(false)) {
        m_deleteTimer.Disable();
        sLog.Magenta( "WreckSE::Process()", "Garbage Collection is removing Wreck %u.", m_contRef->itemID() );
        Delete();
        delete this;
    }
}

void WreckSE::Abandon()
{
    SystemEntity::Abandon();
    m_abandoned = true;
}

void WreckSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.mode = Ball::Mode::TROLL;
        head.flags = Ball::Flag::IsFree | Ball::Flag::IsInteractive;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_self->type().mass();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAlliance(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    DataSector data = DataSector();
        data.inertia = 1;
        data.maxSpeed = 1;
        data.velX = 0;
        data.velY = 0;
        data.velZ = 0;
        data.speedfraction = 1;
    into.Append( data );
    TROLL_Struct troll;
        troll.formationID = 0xFF;
        troll.effectStamp = sEntityList.GetStamp();
    into.Append( troll );
    _log(SE__DESTINY, "WreckSE::EncodeDestiny(): %s - id:%li, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *WreckSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for WreckSE %s(%u)", m_self->name(), m_self->itemID());
    /* not used, unsure if needed
    PyTuple* nameID = new PyTuple(2);
        nameID->SetItem(0,  new PyString("UI/Inflight/WreckNameShipName"));
    PyDict* shipName = new PyDict();
        shipName->SetItem("shipName", new PyInt(0));    // does this need data here?
        nameID->SetItem(1, shipName);
    */
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        if (m_abandoned or (m_fleetID)) { // this is ONLY for abandoned wrecks or wrecks from fleet ops
            PyTuple* loot = new PyTuple(4);
                loot->SetItem(0,                new PyInt(m_ownerID));
                loot->SetItem(1,                IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
                loot->SetItem(2,                IsFleetID(m_fleetID) ? new PyInt(m_fleetID) : PyStatic.NewNone());
                loot->SetItem(3,                new PyBool(false)); // what is this??
            slim->SetItemString("lootRights",   loot );
        }
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("isEmpty",          new PyBool(m_contRef->IsEmpty()));
        slim->SetItemString("launcherID",       new PyLong(m_launchedByID));
        slim->SetItemString("securityStatus",   new PyInt(0));  //FIXME TODO
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        PyDict* dict = new PyDict;
            dict->SetItemString("WreckTypeID",  new PyInt(m_self->typeID()));
        PyTuple* tuple2 = new PyTuple(2);
            tuple2->SetItem(0, new PyString("UI/Inflight/WreckNameTypeID"));
            tuple2->SetItem(1, dict);
        slim->SetItemString("nameID",           tuple2);

    if (is_log_enabled(DESTINY__DEBUG)) {
        _log( DESTINY__DEBUG, "WreckSE::MakeSlimItem() - %s(%u)", GetName(), GetID());
        slim->Dump(DESTINY__DEBUG, "     ");
    }

    return slim;
}

void WreckSE::SendDamageStateChanged() {  //working 24Apr15
    DoDestinyDamageState dmgState;
    MakeDamageState(dmgState);
    OnDamageStateChange dmgChange;
    dmgChange.entityID = m_self->itemID();
    dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    if (m_targMgr != nullptr)
        m_targMgr->QueueUpdate(&up);
    PySafeDecRef(up);
}

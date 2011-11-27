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
    Author:     Zhur
*/

#include "EVEServerPCH.h"

#ifndef INT_MAX
#   define INT_MAX 0x7FFFFFFF
#endif

//const uint32 SKILL_BASE_POINTS = 250;
const EvilNumber EVIL_SKILL_BASE_POINTS(250);
const int32 ITEM_DB_SAVE_TIMER_EXPIRY(10);

/*
 * ItemData
 */
ItemData::ItemData(
    const char *_name,
    uint32 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    bool _contraband,
    bool _singleton,
    uint32 _quantity,
    const GPoint &_position,
    const char *_customInfo)
: name(_name),
  typeID(_typeID),
  ownerID(_ownerID),
  locationID(_locationID),
  flag(_flag),
  contraband(_contraband),
  singleton(_singleton),
  quantity(_quantity),
  position(_position),
  customInfo(_customInfo)
{
}

ItemData::ItemData(
    uint32 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    uint32 _quantity,
    const char *_customInfo,
    bool _contraband)
: name(""),
  typeID(_typeID),
  ownerID(_ownerID),
  locationID(_locationID),
  flag(_flag),
  contraband(_contraband),
  singleton(false),
  quantity(_quantity),
  position(0, 0, 0),
  customInfo(_customInfo)
{
}

ItemData::ItemData(
    uint32 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    const char *_name,
    const GPoint &_position,
    const char *_customInfo,
    bool _contraband)
: name(_name),
  typeID(_typeID),
  ownerID(_ownerID),
  locationID(_locationID),
  flag(_flag),
  contraband(_contraband),
  singleton(true),
  quantity(1),
  position(_position),
  customInfo(_customInfo)
{
}

/*
 * InventoryItem
 */
InventoryItem::InventoryItem(
    ItemFactory &_factory,
    uint32 _itemID,
    const ItemType &_type,
    const ItemData &_data)
: RefObject( 0 ),
  //attributes(_factory, *this, true, true),
  mAttributeMap(*this),
  m_saveTimer(0,true),
  m_factory(_factory),
  m_itemID(_itemID),
  m_itemName(_data.name),
  m_type(_type),
  m_ownerID(_data.ownerID),
  m_locationID(_data.locationID),
  m_flag(_data.flag),
  m_contraband(_data.contraband),
  m_singleton(_data.singleton),
  m_quantity(_data.quantity),
  m_position(_data.position),
  m_customInfo(_data.customInfo)
  
{
    // assert for data consistency
    assert(_data.typeID == _type.id());

    //m_saveTimerExpiryTime = ITEM_DB_SAVE_TIMER_EXPIRY * 60 * 1000;      // 10 minutes in milliseconds
    //m_saveTimer.SetTimer(m_saveTimerExpiryTime);                        // set timer in milliseconds
    m_saveTimer.Disable();                                              // disable timer by default

    _log(ITEM__TRACE, "Created object %p for item %s (%u).", this, itemName().c_str(), itemID());
}

InventoryItem::~InventoryItem()
{
    // Save this item's entity_attributes info to the Database before it is destroyed
    //mAttributeMap.SaveAttributes();

    // Save this item's entity table info to the Database before it is destroyed
    //SaveItem();
}

InventoryItemRef InventoryItem::Load(ItemFactory &factory, uint32 itemID)
{
    return InventoryItem::Load<InventoryItem>( factory, itemID );
}

template<class _Ty>
RefPtr<_Ty> InventoryItem::_LoadItem(ItemFactory &factory, uint32 itemID,
    // InventoryItem stuff:
    const ItemType &type, const ItemData &data)
{
    // See what to do next:
    switch( type.categoryID() ) {
        //! TODO not handled.
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Station:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Accessories:
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::Reaction:
        case EVEDB::invCategories::Asteroid:
             break;
        ///////////////////////////////////////
        // Blueprint:
        ///////////////////////////////////////
        case EVEDB::invCategories::Blueprint: {
            return Blueprint::_LoadItem<Blueprint>( factory, itemID, type, data );
        }

        ///////////////////////////////////////
        // Entity:
        ///////////////////////////////////////
        case EVEDB::invCategories::Entity: {
            if( (type.groupID() == EVEDB::invGroups::Spawn_Container) )
                return CargoContainerRef( new CargoContainer( factory, itemID, type, data ) );
            else
                return CelestialObjectRef( new CelestialObject( factory, itemID, type, data ) );
        }

        ///////////////////////////////////////
        // Celestial:
        ///////////////////////////////////////
        case EVEDB::invCategories::Celestial: {
            if( (type.groupID() == EVEDB::invGroups::Secure_Cargo_Container)
                || (type.groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
                || (type.groupID() == EVEDB::invGroups::Freight_Container)
                || (type.groupID() == EVEDB::invGroups::Cargo_Container)
                || (type.groupID() == EVEDB::invGroups::Wreck) )
                return CargoContainerRef( new CargoContainer( factory, itemID, type, data ) );
            else
                return CelestialObjectRef( new CelestialObject( factory, itemID, type, data ) );
        }

        ///////////////////////////////////////
        // Ship:
        ///////////////////////////////////////
        case EVEDB::invCategories::Ship: {
            return Ship::_LoadItem<Ship>( factory, itemID, type, data );
        }

        ///////////////////////////////////////
        // Skill:
        ///////////////////////////////////////
        case EVEDB::invCategories::Skill: {
            return Skill::_LoadItem<Skill>( factory, itemID, type, data );
        }

        ///////////////////////////////////////
        // Owner:
        ///////////////////////////////////////
        case EVEDB::invCategories::Owner: {
            return Owner::_LoadItem<Owner>( factory, itemID, type, data );
        }
    }

    // ItemCategory didn't do it, try ItemGroup:
    switch( type.groupID() ) {
        ///////////////////////////////////////
        // Station:
        ///////////////////////////////////////
        case EVEDB::invGroups::Station: {
            return Station::_LoadItem<Station>( factory, itemID, type, data );
        }
    }

    // Generic item, create one:
    return InventoryItemRef( new InventoryItem( factory, itemID, type, data ) );
}

bool InventoryItem::_Load()
{
    // load attributes
    mAttributeMap.Load();

    // update inventory
    Inventory *inventory = m_factory.GetInventory( locationID(), false );
    if( inventory != NULL )
        inventory->AddItem( InventoryItemRef( this ) );

    return true;
}

InventoryItemRef InventoryItem::Spawn(ItemFactory &factory, ItemData &data)
{
    // obtain type of new item
    const ItemType *t = factory.GetType( data.typeID );
    if( t == NULL )
        return InventoryItemRef();

    // See what to do next:
    switch( t->categoryID() ) {
        //! TODO not handled.
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Station:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Accessories:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Entity:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Reaction:
             break;
        ///////////////////////////////////////
        // Blueprint:
        ///////////////////////////////////////
        case EVEDB::invCategories::Blueprint: {
            BlueprintData bdata; // use default blueprint attributes

            BlueprintRef blueRef = Blueprint::Spawn( factory, data, bdata );
            blueRef.get()->SaveAttributes();

            return blueRef;
        }

        ///////////////////////////////////////
        // Celestial:
        // (used for Cargo Containers, Rings, and Biomasses, Wrecks, Large Collidable Objects, Clouds,
        //  Cosmic Signatures, Mobile Sentry Guns, Global Warp Disruptors, Agents in Space, Cosmic Anomaly, Beacons, Wormholes,
        //  and other celestial static objects such as NPC stations, stars, moons, planets, and stargates)
        ///////////////////////////////////////
        case EVEDB::invCategories::Celestial: {
            if ( (t->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
                || (t->groupID() == EVEDB::invGroups::Cargo_Container)
                || (t->groupID() == EVEDB::invGroups::Freight_Container)
                || (t->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
                || (t->groupID() == EVEDB::invGroups::Spawn_Container)
                || (t->groupID() == EVEDB::invGroups::Wreck) )
            {
                // Spawn new Cargo Container
                uint32 itemID = CargoContainer::_Spawn( factory, data );
                if( itemID == 0 )
                    return CargoContainerRef();

                CargoContainerRef cargoRef = CargoContainer::Load( factory, itemID );

                // THESE SHOULD BE MOVED INTO A CargoContainer::Spawn() function that does not exist yet
                // Create default dynamic attributes in the AttributeMap:
                cargoRef.get()->SetAttribute(AttrIsOnline,      1);                                                 // Is Online
                cargoRef.get()->SetAttribute(AttrDamage,        0.0);                                               // Structure Damage
                //cargoRef.get()->SetAttribute(AttrShieldCharge,  cargoRef.get()->GetAttribute(AttrShieldCapacity));  // Shield Charge
                //cargoRef.get()->SetAttribute(AttrArmorDamage,   0.0);                                               // Armor Damage
                cargoRef.get()->SetAttribute(AttrMass,          cargoRef.get()->type().attributes.mass());          // Mass
                cargoRef.get()->SetAttribute(AttrRadius,        cargoRef.get()->type().attributes.radius());        // Radius
                cargoRef.get()->SetAttribute(AttrVolume,        cargoRef.get()->type().attributes.volume());        // Volume
                cargoRef.get()->SetAttribute(AttrCapacity,      cargoRef.get()->type().attributes.capacity());      // Capacity
                cargoRef.get()->SaveAttributes();

                return cargoRef;
                //uint32 itemID = InventoryItem::_Spawn( factory, data );
                //if( itemID == 0 )
                //    return InventoryItemRef();
                //return InventoryItem::Load( factory, itemID );
            }
            else
            {
                // Spawn new Celestial Object
                uint32 itemID = CelestialObject::_Spawn( factory, data );
                if( itemID == 0 )
                    return CelestialObjectRef();
                CelestialObjectRef celestialRef = CelestialObject::Load( factory, itemID );
                celestialRef.get()->SaveAttributes();

                return celestialRef;
            }
        }

        ///////////////////////////////////////
        // Ship:
        ///////////////////////////////////////
        case EVEDB::invCategories::Ship: {
            ShipRef shipRef = Ship::Spawn( factory, data );
            shipRef.get()->SaveAttributes();

            return shipRef;
        }

        ///////////////////////////////////////
        // Skill:
        ///////////////////////////////////////
        case EVEDB::invCategories::Skill: {
            return Skill::Spawn( factory, data );
        }

        ///////////////////////////////////////
        // Owner:
        ///////////////////////////////////////
        case EVEDB::invCategories::Owner:
        {
            return Owner::Spawn( factory, data );
        }

        ///////////////////////////////////////
        // Module:
        ///////////////////////////////////////
        case EVEDB::invCategories::Module:
        {
            // Spawn generic item:
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if( itemID == 0 )
                return InventoryItemRef();

            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );

            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef.get()->SetAttribute(AttrIsOnline,   1);                                             // Is Online
            itemRef.get()->SetAttribute(AttrDamage,     0.0);                                             // Structure Damage
            //itemRef.get()->SetAttribute(AttrShieldCharge, itemRef.get()->GetAttribute(AttrShieldCapacity));       // Shield Charge
            //itemRef.get()->SetAttribute(AttrArmorDamage, 0.0);                                        // Armor Damage
            itemRef.get()->SetAttribute(AttrMass,       itemRef.get()->type().attributes.mass());           // Mass
            itemRef.get()->SetAttribute(AttrRadius,     itemRef.get()->type().attributes.radius());       // Radius
            itemRef.get()->SetAttribute(AttrVolume,     itemRef.get()->type().attributes.volume());       // Volume
            itemRef.get()->SetAttribute(AttrCapacity,   itemRef.get()->type().attributes.capacity());   // Capacity
            itemRef.get()->SaveAttributes();

            return itemRef;
        }

        ///////////////////////////////////////
        // Drone:
        ///////////////////////////////////////
        case EVEDB::invCategories::Drone:
        {
            // Spawn generic item:
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if( itemID == 0 )
                return InventoryItemRef();

            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );

            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef.get()->SetAttribute(AttrIsOnline,       1);                                             // Is Online
            itemRef.get()->SetAttribute(AttrDamage,         0.0);                                             // Structure Damage
            itemRef.get()->SetAttribute(AttrShieldCharge,   itemRef.get()->GetAttribute(AttrShieldCapacity));       // Shield Charge
            itemRef.get()->SetAttribute(AttrArmorDamage,    0.0);                                        // Armor Damage
            itemRef.get()->SetAttribute(AttrMass,           itemRef.get()->type().attributes.mass());           // Mass
            itemRef.get()->SetAttribute(AttrRadius,         itemRef.get()->type().attributes.radius());       // Radius
            itemRef.get()->SetAttribute(AttrVolume,         itemRef.get()->type().attributes.volume());       // Volume
            itemRef.get()->SetAttribute(AttrCapacity,       itemRef.get()->type().attributes.capacity());   // Capacity
            itemRef.get()->SaveAttributes();

            return itemRef;
        }

        ///////////////////////////////////////
        // Deployable:
        ///////////////////////////////////////
        case EVEDB::invCategories::Deployable:
        {
            // Spawn generic item:
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if( itemID == 0 )
                return InventoryItemRef();

            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );

            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef.get()->SetAttribute(AttrIsOnline,       1);                                             // Is Online
            itemRef.get()->SetAttribute(AttrDamage,         0.0);                                             // Structure Damage
            //itemRef.get()->SetAttribute(AttrShieldCharge,   itemRef.get()->GetAttribute(AttrShieldCapacity));       // Shield Charge
            //itemRef.get()->SetAttribute(AttrArmorDamage,    0.0);                                        // Armor Damage
            itemRef.get()->SetAttribute(AttrMass,           itemRef.get()->type().attributes.mass());           // Mass
            itemRef.get()->SetAttribute(AttrRadius,         itemRef.get()->type().attributes.radius());       // Radius
            itemRef.get()->SetAttribute(AttrVolume,         itemRef.get()->type().attributes.volume());       // Volume
            itemRef.get()->SetAttribute(AttrCapacity,       itemRef.get()->type().attributes.capacity());   // Capacity
            itemRef.get()->SaveAttributes();

            return itemRef;
        }

        ///////////////////////////////////////
        // Asteroid:
        ///////////////////////////////////////
        case EVEDB::invCategories::Asteroid:
        {
            // Spawn generic item:
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if( itemID == 0 )
                return InventoryItemRef();

            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );

            // THESE SHOULD BE MOVED INTO A Asteroid::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef.get()->SetAttribute(AttrRadius, 500.0);       // Radius
            itemRef.get()->SetAttribute(AttrMass,   1000000.0);    // Mass
            itemRef.get()->SetAttribute(AttrVolume, 8000.0);       // Volume
            itemRef.get()->SetAttribute(AttrQuantity, 1000.0);      // Quantity
            itemRef.get()->SaveAttributes();

            return itemRef;
        }

        ///////////////////////////////////////
        // Structure:
        ///////////////////////////////////////
        case EVEDB::invCategories::Structure:
        {
            // Spawn generic item:
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if( itemID == 0 )
                return InventoryItemRef();

            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );

            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef.get()->SetAttribute(AttrIsOnline,       1);                                             // Is Online
            itemRef.get()->SetAttribute(AttrDamage,         0.0);                                             // Structure Damage
            itemRef.get()->SetAttribute(AttrShieldCharge,   itemRef.get()->GetAttribute(AttrShieldCapacity));       // Shield Charge
            itemRef.get()->SetAttribute(AttrArmorDamage,    0.0);                                        // Armor Damage
            itemRef.get()->SetAttribute(AttrMass,           itemRef.get()->type().attributes.mass());           // Mass
            itemRef.get()->SetAttribute(AttrRadius,         itemRef.get()->type().attributes.radius());       // Radius
            itemRef.get()->SetAttribute(AttrVolume,         itemRef.get()->type().attributes.volume());       // Volume
            itemRef.get()->SetAttribute(AttrCapacity,       itemRef.get()->type().attributes.capacity());   // Capacity
            itemRef.get()->SaveAttributes();

            return itemRef;
        }
    }

    switch( t->groupID() ) {
        ///////////////////////////////////////
        // Station:
        ///////////////////////////////////////
        case EVEDB::invGroups::Station: {
            //_log( ITEM__ERROR, "Refusing to create station '%s'.", data.name.c_str() );
            //return InventoryItemRef();
            //return Station::Spawn( factory, data );
            uint32 itemID = Station::_Spawn( factory, data );
            if( itemID == 0 )
                return StationRef();

            StationRef stationRef = Station::Load( factory, itemID );

            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            stationRef.get()->SetAttribute(AttrIsOnline,    1);                                              // Is Online
            stationRef.get()->SetAttribute(AttrDamage,      0.0);                                              // Structure Damage
            stationRef.get()->SetAttribute(AttrShieldCharge,stationRef.get()->GetAttribute(AttrShieldCapacity));     // Shield Charge
            stationRef.get()->SetAttribute(AttrArmorDamage, 0.0);                                         // Armor Damage
            stationRef.get()->SetAttribute(AttrMass,        stationRef.get()->type().attributes.mass());         // Mass
            stationRef.get()->SetAttribute(AttrRadius,      stationRef.get()->type().attributes.radius());     // Radius
            stationRef.get()->SetAttribute(AttrVolume,      stationRef.get()->type().attributes.volume());     // Volume
            stationRef.get()->SetAttribute(AttrCapacity,    stationRef.get()->type().attributes.capacity()); // Capacity
            stationRef.get()->SaveAttributes();

            return stationRef;
        }
    }

    // Spawn generic item:
    uint32 itemID = InventoryItem::_Spawn( factory, data );
    if( itemID == 0 )
        return InventoryItemRef();
    InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
    itemRef.get()->SaveAttributes();
    return itemRef;
}

uint32 InventoryItem::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // obtain type of new item
    // this also checks that the type is valid
    const ItemType *t = factory.GetType(data.typeID);
    if(t == NULL)
        return 0;

    // fix the name (if empty)
    if(data.name.empty())
        data.name = t->name();

    // insert new entry into DB
    return factory.db().NewItem(data);
}

void InventoryItem::Delete() {
    //first, get out of client's sight.
    //this also removes us from our inventory.
    Move( 6 );
    ChangeOwner( 2 );

    //take ourself out of the DB
    //attributes.Delete();
    m_factory.db().DeleteItem( itemID() );

    mAttributeMap.Delete();

    //delete ourselves from factory cache
    m_factory._DeleteItem( itemID() );
}
PyPackedRow* InventoryItem::GetItemRow() const
{
	DBRowDescriptor* header = new DBRowDescriptor;
	header->AddColumn( "itemID",     DBTYPE_I4 );
	header->AddColumn( "typeID",     DBTYPE_I2 );
	header->AddColumn( "ownerID",    DBTYPE_I4 );
	header->AddColumn( "locationID", DBTYPE_I4 );
	header->AddColumn( "flagID",     DBTYPE_I2 );
	header->AddColumn( "singleton",  DBTYPE_BOOL );
	header->AddColumn( "quantity",   DBTYPE_I4 );
	header->AddColumn( "groupID",    DBTYPE_I2 );
	header->AddColumn( "categoryID", DBTYPE_UI1 );
	header->AddColumn( "customInfo", DBTYPE_STR );
	header->AddColumn( "stacksize" , DBTYPE_I4 );

	PyPackedRow* row = new PyPackedRow( header );
	GetItemRow( row );

	return row;
}

void InventoryItem::GetItemRow( PyPackedRow* into ) const
{
    into->SetField( "itemID",     new PyInt( itemID() ) );
    into->SetField( "typeID",     new PyInt( typeID() ) );
    into->SetField( "locationID", new PyInt( locationID() ) );
    into->SetField( "ownerID",    new PyInt( ownerID() ) );
    into->SetField( "flagID",     new PyInt( flag() ) );
    into->SetField( "singleton",  new PyBool( singleton() ) );
    into->SetField( "quantity",   new PyInt( quantity() ) );
    into->SetField( "groupID",    new PyInt( groupID() ) );
    into->SetField( "categoryID", new PyInt( categoryID() ) );
    into->SetField( "customInfo", new PyString( customInfo() ) );
	into->SetField( "stacksize",  new PyInt (quantity()) );
}

bool InventoryItem::Populate( Rsp_CommonGetInfo_Entry& result )
{
    //itemID:
    result.itemID = itemID();

    //invItem:
    PySafeDecRef( result.invItem );
    result.invItem = GetItemRow();

    //hacky, but it doesn't really hurt anything.
    if( GetAttribute(AttrIsOnline).get_int() != 0 )
    {
        //there is an effect that goes along with this. We should
        //probably be properly tracking the effect due to some
        // timer things, but for now, were hacking it.
        EntityEffectState es;
        es.env_itemID = itemID();
        es.env_charID = ownerID();  //may not be quite right...
        es.env_shipID = locationID();
        es.env_target = locationID();   //this is what they do.
        es.env_other = new PyNone;
        es.env_effectID = effectOnline;
        es.startTime = Win32TimeNow() - Win32Time_Hour; //act like it happened an hour ago
        es.duration = INT_MAX;
        es.repeat = 0;
        es.randomSeed = new PyNone;

        result.activeEffects[es.env_effectID] = es.Encode();
    }

    //activeEffects:
    //result..activeEffects[id] = List[11];

    //attributes:
    AttributeMap::AttrMapItr itr = mAttributeMap.begin();
    AttributeMap::AttrMapItr itr_end = mAttributeMap.end();
    for (; itr != itr_end; itr++) {
        result.attributes[(*itr).first] = (*itr).second.GetPyObject();
    }

    //no idea what time this is supposed to be
    result.time = Win32TimeNow();

    return true;
}

PyObject * InventoryItem::ItemGetInfo()
{
    Rsp_ItemGetInfo result;

    if(!Populate(result.entry))
        return NULL;    //print already done.

    return(result.Encode());
}

void InventoryItem::Rename(const char *to) {
    
	m_itemName = to;
    SaveItem();
}

void InventoryItem::MoveInto(Inventory &new_home, EVEItemFlags _flag, bool notify) {
    Move( new_home.inventoryID(), _flag, notify );
}

void InventoryItem::Move(uint32 new_location, EVEItemFlags new_flag, bool notify) {
    uint32 old_location = locationID();
    EVEItemFlags old_flag = flag();

    if( new_location == old_location && new_flag == old_flag )
        return; //nothing to do...

    //first, take myself out of my old inventory, if its loaded.
    Inventory *old_inventory = m_factory.GetInventory( old_location, false );
    if(old_inventory != NULL)
        old_inventory->RemoveItem( itemID() );  //releases its ref

    m_locationID = new_location;
    m_flag = new_flag;

    //then make sure that my new inventory is updated, if its loaded.
    Inventory *new_inventory = m_factory.GetInventory( new_location, false );
    if( new_inventory != NULL )
        new_inventory->AddItem( InventoryItemRef( this ) ); //makes a new ref

    SaveItem();

    //notify about the changes.
    if( notify )
    {
        std::map<int32, PyRep *> changes;

        if( new_location != old_location )
            changes[ixLocationID] = new PyInt(old_location);
        if( new_flag != old_flag )
            changes[ixFlag] = new PyInt(old_flag);

        SendItemChange( ownerID(), changes );   //changes is consumed
    }
}

bool InventoryItem::AlterQuantity(int32 qty_change, bool notify) {
    if(qty_change == 0)
        return true;

    int32 new_qty = m_quantity + qty_change;

    if(new_qty < 0) {
        codelog(ITEM__ERROR, "%s (%u): Tried to remove %d quantity from stack of %u", m_itemName.c_str(), m_itemID, -qty_change, m_quantity);
        return false;
    }

    return(SetQuantity(new_qty, notify));
}

bool InventoryItem::SetQuantity(uint32 qty_new, bool notify) {
    //if an object has its singleton set then it shouldn't be able to add/remove qty
    if(m_singleton) {
        //Print error
        codelog(ITEM__ERROR, "%s (%u): Failed to set quantity %u , the items singleton bit is set", m_itemName.c_str(), m_itemID, qty_new);
        //return false
        return false;
    }

    uint32 old_qty = m_quantity;

    m_quantity = qty_new;

    SaveItem();

    //notify about the changes.
    if(notify) {
        std::map<int32, PyRep *> changes;

        //send the notify to the new owner.
        changes[ixQuantity] = new PyInt(old_qty);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    return true;
}

InventoryItemRef InventoryItem::Split(int32 qty_to_take, bool notify) {
    if(qty_to_take <= 0) {
        _log(ITEM__ERROR, "%s (%u): Asked to split into a chunk of %d", itemName().c_str(), itemID(), qty_to_take);
        return InventoryItemRef();
    }
    if(!AlterQuantity(-qty_to_take, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to remove quantity %d during split.", itemName().c_str(), itemID(), qty_to_take);
        return InventoryItemRef();
    }

    ItemData idata(
        typeID(),
        ownerID(),
        (notify ? 1 : locationID()), //temp location to cause the spawn via update
        flag(),
        qty_to_take
    );

    InventoryItemRef res = m_factory.SpawnItem(idata);
    if(notify)
        res->Move( locationID(), flag() );

    return( res );
}

bool InventoryItem::Merge(InventoryItemRef to_merge, uint32 qty, bool notify) {
    if(typeID() != to_merge->typeID()) {
        _log(ITEM__ERROR, "%s (%u): Asked to merge with %s (%u).", itemName().c_str(), itemID(), to_merge->itemName().c_str(), to_merge->itemID());
        return false;
    }
    if(locationID() != to_merge->locationID() || flag() != to_merge->flag()) {
        _log(ITEM__ERROR, "%s (%u) in location %u, flag %u: Asked to merge with item %u in location %u, flag %u.", itemName().c_str(), itemID(), locationID(), flag(), to_merge->itemID(), to_merge->locationID(), to_merge->flag());
        return false;
    }
    if(qty == 0)
        qty = to_merge->quantity();
    if(qty <= 0) {
        _log(ITEM__ERROR, "%s (%u): Asked to merge with %d units of item %u.", itemName().c_str(), itemID(), qty, to_merge->itemID());
        return false;
    }
    if(!AlterQuantity(qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to add quantity %d.", itemName().c_str(), itemID(), qty);
        return false;
    }

    if(qty == to_merge->quantity()) {
        to_merge->Delete();
    } else if(!to_merge->AlterQuantity(static_cast<int32>(-qty), notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to remove quantity %d.", to_merge->itemName().c_str(), to_merge->itemID(), qty);
        return false;
    }

    return true;
}

bool InventoryItem::ChangeSingleton(bool new_singleton, bool notify) {
    bool old_singleton = m_singleton;

    if(new_singleton == old_singleton)
        return true;    //nothing to do...

    m_singleton = new_singleton;

    SaveItem();

    //notify about the changes.
    if(notify) {
        std::map<int32, PyRep *> changes;
        changes[ixSingleton] = new PyInt(old_singleton);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    return true;
}

void InventoryItem::ChangeOwner(uint32 new_owner, bool notify) {
    uint32 old_owner = m_ownerID;

    if(new_owner == old_owner)
        return; //nothing to do...

    m_ownerID = new_owner;

    SaveItem();

    //notify about the changes.
    if(notify) {
        std::map<int32, PyRep *> changes;

        //send the notify to the new owner.
        changes[ixOwnerID] = new PyInt(old_owner);
        SendItemChange(new_owner, changes); //changes is consumed

        //also send the notify to the old owner.
        changes[ixOwnerID] = new PyInt(old_owner);
        SendItemChange(old_owner, changes); //changes is consumed
    }
}

void InventoryItem::SaveItem()
{
    //_log( ITEM__TRACE, "Saving item %u.", itemID() );
    
    //mAttributeMap.Save();
    SaveAttributes();

    m_factory.db().SaveItem(
        itemID(),
        ItemData(
            itemName().c_str(),
            typeID(),
            ownerID(),
            locationID(),
            flag(),
            contraband(),
            singleton(),
            quantity(),
            position(),
            customInfo().c_str()
        )
    );
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const {
    //TODO: figure out the appropriate list of interested people...
    Client *c = m_factory.entity_list.FindCharacter(toID);
    if(c == NULL)
        return; //not found or not online...

    NotifyOnItemChange change;
    change.itemRow = GetItemRow();

    change.changes = changes;
    changes.clear();    //consume them.

    PyTuple *tmp = change.Encode();  //this is consumed below
    c->SendNotification("OnItemChange", "charid", &tmp, false); //unsequenced.
}


/*typedef enum {
    dgmEffPassive = 0,
    dgmEffActivation = 1,
    dgmEffTarget = 2,
    dgmEffArea = 3,
    dgmEffOnline = 4,
    dgmEffOverload = 5,
} EffectCategories;*/


void InventoryItem::SetOnline(bool newval) {

    SetAttribute(AttrIsOnline, int(newval));

    Client *c = sEntityList.FindCharacter(m_ownerID);
    if(c == NULL)
    {
        sLog.Error("InventoryItem", "unable to set ourselfs online//offline because we can't find the client");
        return;
    }

    Notify_OnGodmaShipEffect ogf;
    ogf.itemID = m_itemID;
    ogf.effectID = effectOnline;
    ogf.when = Win32TimeNow();
    ogf.start = newval?1:0;
    ogf.active = newval?1:0;
    ogf.env_itemID = ogf.itemID;
    ogf.env_charID = m_ownerID; //??
    ogf.env_shipID = m_locationID;
    ogf.env_target = m_locationID;
    ogf.env_effectID = ogf.effectID;
    ogf.startTime = ogf.when;
    ogf.duration = INT_MAX; //I think this should be infinity (0x07 may be infinity?)
    ogf.repeat = 0;
    ogf.randomSeed = new PyNone();
    ogf.error = new PyNone();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem( ogf.Encode() );

    PyTuple* tmp = multi.Encode();   //this is consumed below
    c->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void InventoryItem::SetCustomInfo(const char *ci) {
    if(ci == NULL)
        m_customInfo = "";
    else
        m_customInfo = ci;
    SaveItem();
}

void InventoryItem::Relocate(const GPoint &pos) {
    if(m_position == pos)
        return;

    m_position = pos;

    SaveItem();
}

bool InventoryItem::SetAttribute( uint32 attributeID, int64 num, bool notify /* true */ )
{
    EvilNumber devil_number(num);
    return mAttributeMap.SetAttribute(attributeID, devil_number, notify);
}

bool InventoryItem::SetAttribute( uint32 attributeID, double num, bool notify /* true */ )
{
    EvilNumber devil_number(num);
    return mAttributeMap.SetAttribute(attributeID, devil_number, notify);
}

bool InventoryItem::SetAttribute( uint32 attributeID, EvilNumber num, bool notify /* true */ )
{
    return mAttributeMap.SetAttribute(attributeID, num, notify);
}

bool InventoryItem::SetAttribute( uint32 attributeID, int num, bool notify /* true */ )
{
    EvilNumber devil_number(num);
    return mAttributeMap.SetAttribute(attributeID, devil_number, notify);
}

bool InventoryItem::SetAttribute( uint32 attributeID, uint64 num, bool notify /* true */ )
{
    EvilNumber devil_number(*((int64*)&num));
    return mAttributeMap.SetAttribute(attributeID, devil_number, notify);
}

bool InventoryItem::SetAttribute( uint32 attributeID, uint32 num, bool notify /* true */ )
{
    EvilNumber devil_number((int64)num);
    return mAttributeMap.SetAttribute(attributeID, devil_number, notify);
}

EvilNumber InventoryItem::GetAttribute( uint32 attributeID )
{
    return mAttributeMap.GetAttribute(attributeID);
}

EvilNumber InventoryItem::GetAttribute( const uint32 attributeID ) const
{
     return mAttributeMap.GetAttribute(attributeID);
}

bool InventoryItem::HasAttribute(uint32 attributeID)
{
    return mAttributeMap.HasAttribute(attributeID);
}

bool InventoryItem::SaveAttributes()
{
    return mAttributeMap.SaveAttributes();
}

bool InventoryItem::ResetAttribute(uint32 attrID, bool notify)
{
	return mAttributeMap.ResetAttribute(attrID, notify);
}

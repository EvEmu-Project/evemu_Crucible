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
	Author:		Bloody.Rabbit
*/

#include "EVEServerPCH.h"

/*
 * StationTypeData
 */
StationTypeData::StationTypeData(
	uint32 _dockingBayGraphicID,
	uint32 _hangarGraphicID,
	const GPoint &_dockEntry,
	const GVector &_dockOrientation,
	uint32 _operationID,
	uint32 _officeSlots,
	double _reprocessingEfficiency,
	bool _conquerable)
: dockingBayGraphicID(_dockingBayGraphicID),
  hangarGraphicID(_hangarGraphicID),
  dockEntry(_dockEntry),
  dockOrientation(_dockOrientation),
  operationID(_operationID),
  officeSlots(_officeSlots),
  reprocessingEfficiency(_reprocessingEfficiency),
  conquerable(_conquerable)
{
}

/*
 * StationType
 */
StationType::StationType(
	uint32 _id,
	// ItemType stuff:
	const ItemGroup &_group,
	const TypeData &_data,
	// StationType stuff:
	const StationTypeData &_stData)
: ItemType(_id, _group, _data),
  m_dockingBayGraphicID(_stData.dockingBayGraphicID),
  m_hangarGraphicID(_stData.hangarGraphicID),
  m_dockEntry(_stData.dockEntry),
  m_dockOrientation(_stData.dockOrientation),
  m_operationID(_stData.operationID),
  m_officeSlots(_stData.officeSlots),
  m_reprocessingEfficiency(_stData.reprocessingEfficiency),
  m_conquerable(_stData.conquerable)
{
	// consistency check
	assert(_data.groupID == EVEDB::invGroups::Station);
}

StationType *StationType::Load(ItemFactory &factory, uint32 stationTypeID)
{
	return ItemType::Load<StationType>( factory, stationTypeID );
}

template<class _Ty>
_Ty *StationType::_LoadStationType(ItemFactory &factory, uint32 stationTypeID,
	// ItemType stuff:
	const ItemGroup &group, const TypeData &data,
	// StationType stuff:
	const StationTypeData &stData)
{
	// ready to create
	return new StationType( stationTypeID, group, data, stData );
}

/*
 * StationData
 */
StationData::StationData(
	uint32 _security,
	double _dockingCostPerVolume,
	double _maxShipVolumeDockable,
	uint32 _officeRentalCost,
	uint32 _operationID,
	double _reprocessingEfficiency,
	double _reprocessingStationsTake,
	EVEItemFlags _reprocessingHangarFlag)
: security(_security),
  dockingCostPerVolume(_dockingCostPerVolume),
  maxShipVolumeDockable(_maxShipVolumeDockable),
  officeRentalCost(_officeRentalCost),
  operationID(_operationID),
  reprocessingEfficiency(_reprocessingEfficiency),
  reprocessingStationsTake(_reprocessingStationsTake),
  reprocessingHangarFlag(_reprocessingHangarFlag)
{
}

/*
 * Station
 */
Station::Station(
	ItemFactory &_factory,
	uint32 _stationID,
	// InventoryItem stuff:
	const StationType &_type,
	const ItemData &_data,
	// CelestialObject stuff:
	const CelestialObjectData &_cData,
	// Station stuff:
	const StationData &_stData)
: CelestialObject(_factory, _stationID, _type, _data, _cData),
  m_stationType(_type),
  m_security(_stData.security),
  m_dockingCostPerVolume(_stData.dockingCostPerVolume),
  m_maxShipVolumeDockable(_stData.maxShipVolumeDockable),
  m_officeRentalCost(_stData.officeRentalCost),
  m_operationID(_stData.operationID),
  m_reprocessingEfficiency(_stData.reprocessingEfficiency),
  m_reprocessingStationsTake(_stData.reprocessingStationsTake),
  m_reprocessingHangarFlag(_stData.reprocessingHangarFlag)
{
}

StationRef Station::Load(ItemFactory &factory, uint32 stationID)
{
	return InventoryItem::Load<Station>( factory, stationID );
}

template<class _Ty>
RefPtr<_Ty> Station::_LoadStation(ItemFactory &factory, uint32 stationID,
	// InventoryItem stuff:
	const StationType &type, const ItemData &data,
	// CelestialObject stuff:
	const CelestialObjectData &cData,
	// Station stuff:
	const StationData &stData)
{
	// ready to create
	return StationRef( new Station( factory, stationID, type, data, cData, stData ) );
}

bool Station::_Load()
{
	// load contents
	if( !LoadContents( m_factory ) )
		return false;

	return CelestialObject::_Load();
}

uint32 Station::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // make sure it's a Station
    const ItemType *item = factory.GetType(data.typeID);
    if( !(item->categoryID() == EVEDB::invCategories::Station) )
        return 0;

    // store item data
    uint32 stationID = InventoryItem::_Spawn(factory, data);
    if( stationID == 0 )
        return 0;

    // nothing additional

    return stationID;
}

using namespace Destiny;

StationEntity::StationEntity(
    StationRef station,
	SystemManager *system,
	PyServiceMgr &services,
	const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), station),
  m_system(system),
  m_services(services)
{
    _stationRef = station;
	m_destiny->SetPosition(position, false);
}

void StationEntity::Process() {
	SystemEntity::Process();
}

void StationEntity::ForcedSetPosition(const GPoint &pt) {
	m_destiny->SetPosition(pt, false);
}

void StationEntity::EncodeDestiny( Buffer& into ) const
{

    const GPoint& position = GetPosition();
    const std::string itemName( GetName() );
/*
	/*if(m_orbitingID != 0) {
		#pragma pack(1)
		struct AddBall_Orbit {
			BallHeader head;
			MassSector mass;
			ShipSector ship;
			DSTBALL_ORBIT_Struct main;
			NameStruct name;
		};
		#pragma pack()
		
		into.resize(start 
			+ sizeof(AddBall_Orbit) 
			+ slen*sizeof(uint16) );
		uint8 *ptr = &into[start];
		AddBall_Orbit *item = (AddBall_Orbit *) ptr;
		ptr += sizeof(AddBall_Orbit);
		
		item->head.entityID = GetID();
		item->head.mode = Destiny::DSTBALL_ORBIT;
		item->head.radius = m_self->radius();
		item->head.x = x();
		item->head.y = y();
		item->head.z = z();
		item->head.sub_type = IsMassive | IsFree;
		
		item->mass.mass = m_self->mass();
		item->mass.unknown51 = 0;
		item->mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		item->mass.corpID = GetCorporationID();
		item->mass.unknown64 = 0xFFFFFFFF;
		
		item->ship.max_speed = m_self->maxVelocity();
		item->ship.velocity_x = m_self->maxVelocity();	//hacky hacky
		item->ship.velocity_y = 0.0;
		item->ship.velocity_z = 0.0;
		item->ship.agility = 1.0;	//hacky
		item->ship.speed_fraction = 0.133f;	//just strolling around. TODO: put in speed fraction!
		
		item->main.unknown116 = 0xFF;
		item->main.followID = m_orbitingID;
		item->main.followRange = 6000.0f;
		
		item->name.name_len = slen;	// in number of unicode chars
		//strcpy_fake_unicode(item->name.name, GetName());
	} else {
        BallHeader head;
		head.entityID = GetID();
		head.mode = Destiny::DSTBALL_STOP;
		head.radius = GetRadius();
		head.x = position.x;
		head.y = position.y;
		head.z = position.z;
		head.sub_type = IsMassive | IsFree;
        into.Append( head );

        MassSector mass;
		mass.mass = GetMass();
		mass.cloak = 0;
		mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
		mass.corpID = GetCorporationID();
		mass.allianceID = GetAllianceID();
        into.Append( mass );

        ShipSector ship;
		ship.max_speed = GetMaxVelocity();
		ship.velocity_x = 0.0;
		ship.velocity_y = 0.0;
		ship.velocity_z = 0.0;
        ship.unknown_x = 0.0;
        ship.unknown_y = 0.0;
        ship.unknown_z = 0.0;
		ship.agility = GetAgility();
		ship.speed_fraction = 0.0;
        into.Append( ship );

        DSTBALL_STOP_Struct main;
		main.formationID = 0xFF;
        into.Append( main );

        const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
        into.Append( nameLen );

        const Buffer::iterator<uint16> name = into.end<uint16>();
        into.ResizeAt( name, nameLen );
        utf8::utf8to16( itemName.begin(), itemName.end(), name );
	}
*/
    BallHeader head;
	head.entityID = GetID();
	head.mode = Destiny::DSTBALL_RIGID;
	head.radius = GetRadius();
	head.x = position.x;
	head.y = position.y;
	head.z = position.z;
	head.sub_type = HasMiniBalls | IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
	main.formationID = 0xFF;
    into.Append( main );

    const uint16 miniballsCount = 1;
    into.Append( miniballsCount );

    MiniBall miniball;
    miniball.x = -7701.181;
    miniball.y = 8060.06;
    miniball.z = 27878.900;
    miniball.radius = 1639.241;
    into.Append( miniball );

    const uint8 nameLen = utf8::distance( itemName.begin(), itemName.end() );
    into.Append( nameLen );

    const Buffer::iterator<uint16> name = into.end<uint16>();
    into.ResizeAt( name, nameLen );
    utf8::utf8to16( itemName.begin(), itemName.end(), name );
}

void StationEntity::MakeDamageState(DoDestinyDamageState &into) const {
	into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
	into.tau = 100000;	//no freaking clue.
	into.timestamp = Win32TimeNow();
//	armor damage isn't working...
	into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
	into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHp).get_float());
}


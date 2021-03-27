
 /**
  * @name StationOffice.cpp
  *   station office item class derived from container
  *
  * @Author:         Allan
  * @date:          14Dec17
  *
  */

#include "station/StationDataMgr.h"

#include "database/EVEDBUtils.h"


#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "station/StationOffice.h"

/*
 * StationOffice
 */
StationOffice::StationOffice(uint32 _officeID, const ItemType& _containerType, const ItemData& _data, const OfficeData& _odata)
: InventoryItem(_officeID, _containerType, _data),
m_data(_odata)
{
    m_loaded = false;
    pInventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created StationOffice object for item %s (%u).", name(), itemID());
}

StationOffice::~StationOffice()
{
    if (pInventory != nullptr)
        pInventory->Unload();
    SafeDelete(pInventory);
}

StationOfficeRef StationOffice::Load( uint32 officeID)
{
    return InventoryItem::Load<StationOffice>(officeID );
}

/**  we are NOT loading office contents for every office on station load.
 * this will be done on first corp member docking to this station
 * @todo maybe later make config option to load corp hangars on station load.
 */
/*
bool StationOffice::_Load() {
    if (!pInventory->LoadContents())
        return false;
    return InventoryItem::_Load();
}*/

StationOfficeRef StationOffice::Spawn( ItemData& idata, OfficeData& odata) {
    uint32 officeID = StationDB::CreateOffice(idata, odata);
    if (officeID == 0 )
        return StationOfficeRef(nullptr);

    return StationOffice::Load(officeID );
}

void StationOffice::ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef) const {
    if (!m_loaded)
        return; // make error here?

    pInventory->HasAvailableSpace(flag, iRef);
}

PyObject *StationOffice::StationOfficeGetInfo() {
    if (!pInventory->LoadContents( ) ) {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for StationOfficeGetInfo", name(), m_itemID );
        return nullptr;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the StationOffice.
    if (!Populate( entry ) )
        return nullptr;    //print already done.

    result.items[ m_itemID ] = entry.Encode();

    return result.Encode();
}

void StationOffice::AddItem(InventoryItemRef iRef)
{
    if (!m_loaded)
        return; // make error here?

    if (iRef.get() == nullptr)
        return;

    InventoryItem::AddItem(iRef);
}

void StationOffice::RemoveItem(InventoryItemRef iRef)
{
    if (!m_loaded)
        return; // make error here?

    if (iRef.get() == nullptr)
        return;

    InventoryItem::RemoveItem(iRef);
}

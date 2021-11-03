
 /**
  * @name StationOffice.h
  *   station office item class derived from inventoryItem
  *
  * @Author:         Allan
  * @date:          14Dec17
  *
  */

#ifndef EVE_STATION_OFFICE_H
#define EVE_STATION_OFFICE_H


#include "EVEServerConfig.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryItem.h"
#include "station/StationDB.h"

class StationOffice
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    StationOffice(uint32 _officeID, const ItemType& _itemType, const ItemData& _data, const OfficeData& _odata);
    virtual ~StationOffice();

    static StationOfficeRef Load( uint32 officeID);
    static StationOfficeRef Spawn( ItemData& idata, OfficeData& odata);

    void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;

    virtual void AddItem(InventoryItemRef item);
    virtual void RemoveItem(InventoryItemRef item);

    PyObject *StationOfficeGetInfo();

    bool IsEmpty()                                      { return (m_loaded ? pInventory->IsEmpty() : false); }
    bool IsLoaded()                                     { return m_loaded; }

    void SetLoaded(bool set=false)                      { m_loaded = set; }

protected:
    using InventoryItem::_Load;
    //virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 officeID, const ItemType& type, const ItemData& idata) {
        if (type.id() != 27)  {
            _log(ITEM__ERROR, "Trying to load itemID %u as Office.", type.id());
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }

        OfficeData odata = OfficeData();
        if (!StationDB::GetOfficeData(officeID, odata))
            return RefPtr<_Ty>(nullptr);

        return StationOfficeRef(new StationOffice(officeID, type, idata, odata));
    }

    virtual PyRep* GetItem() const                      { return GetItemRow(); }

private:
    OfficeData m_data;

    bool m_loaded;

};

#endif  //EVE_STATION_OFFICE_H


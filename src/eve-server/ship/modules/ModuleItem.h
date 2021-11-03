
 /**
  * @name ModuleItem.h
  *   module item class
  * @Author:         Allan
  * @date:   20 June 2019
  */


#ifndef _EVE_SHIP_MODULES_MODULE_H_
#define _EVE_SHIP_MODULES_MODULE_H_


#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryItem.h"


class ModuleItem
: public InventoryItem
{
    friend class InventoryItem;
public:
    virtual ~ModuleItem() { /* do nothing here */ }

    /* class type pointer querys. */
    virtual ModuleItem*     GetModuleItem()             { return this; }
    /* class type tests. */
    virtual bool            IsModuleItem()              { return true; }

    static ModuleItemRef Load(uint32 modID);
    static ModuleItemRef Spawn(ItemData &data);

    // this resets modifiers, sets attrOnline, then sends GodmaShipEffect
    void SetOnline(bool online=false, bool isRig=false);

protected:
    ModuleItem(uint32 _modID, const ItemType &_type, const ItemData &_data );

    // template loading system
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 modID, const ItemType &type, const ItemData &data) {
        if ((type.categoryID() == EVEDB::invCategories::Module)
        or (type.categoryID() == EVEDB::invCategories::Subsystem))
            return ModuleItemRef( new ModuleItem(modID, type, data ) );

        _log(ITEM__ERROR, "Trying to load %s as Module.", sDataMgr.GetCategoryName(type.categoryID()));
        if (sConfig.debug.StackTrace)
            EvE::traceStack();
        return RefPtr<_Ty>(nullptr);
    }

};

#endif /* !_EVE_SHIP_MODULES_MODULE_H_ */

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
    Author:        Allan
*/

#include "eve-server.h"

#include "StaticDataMgr.h"
#include "Client.h"
#include "inventory/ItemFactory.h"
#include "inventory/Voucher.h"
#include "system/BookmarkDB.h"
#include "services/ServiceManager.h"

VoucherService::VoucherService(EVEServiceManager& mgr) :
    Service("voucher"),
    m_manager (mgr)
{
    this->Add("GetObject", &VoucherService::GetObject);
}
void VoucherService::BoundReleased (VoucherBound* bound) {
    auto it = this->m_instances.find (bound->GetVoucherID());

    if (it == this->m_instances.end ())
        return;

    this->m_instances.erase (it);
}

PyResult VoucherService::GetObject(PyCallArgs& call, PyInt* voucherID) {
  /**
    voucher = self.GetVoucherSvc().GetObject(voucherID)
    if voucher is None:
        return
    self.data[voucherID] = voucher
    */
    VoucherBound* bound;
    auto it = this->m_instances.find (voucherID->value());

    if (it == this->m_instances.end ()) {
        //call.Dump(BOOKMARK__CALL_DUMP);
        // return none for now, to allow client to use default name of 'bookmark'
        //return PyStatic.NewNone();
        InventoryItemRef iRef = sItemFactory.GetItemRef(voucherID->value());
        if (iRef.get() == nullptr) {
            codelog(ITEM__ERROR, "%s: Failed to retrieve bookmark voucher for bmID %u", call.client->GetName(), voucherID->value());
            return new PyDict;
        }

        // this is how i return objects for method chaining
        //we just bind up a new voucher object for item requested and give it back to them.
        bound = new VoucherBound(m_manager, *this, iRef);
        this->m_instances.insert_or_assign (voucherID->value(), bound);
    } else {
        bound = it->second;
    }

    bound->NewReference (call.client);

    PyTuple* rsp = new PyTuple(2);

    rsp->SetItem(0, new PySubStruct(new PySubStream(bound->GetOID())));
    rsp->SetItem(1, PyStatic.NewNone());

    return rsp;
}

VoucherBound::VoucherBound(EVEServiceManager& mgr, VoucherService& parent, InventoryItemRef itemRef) :
    EVEBoundObject (mgr, parent),
    m_itemRef (itemRef)
{
    this->Add("GetDescription", &VoucherBound::GetDescription);
}

PyResult VoucherBound::GetDescription(PyCallArgs &call) {
    //   name = voucher.GetDescription()

    // get bookmark name (memo) as stored in db.  item.customInfo is bookmarkID this item is copied from
    return new PyString(BookmarkDB::GetBookmarkName(atoi(m_itemRef->customInfo().c_str())));
}

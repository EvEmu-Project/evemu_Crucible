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

#include "PyServiceCD.h"
#include "inventory/Voucher.h"
#include "system/BookmarkDB.h"

PyCallable_Make_InnerDispatcher(VoucherService)

VoucherService::VoucherService(PyServiceMgr *mgr)
: PyService(mgr, "voucher"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(VoucherService, GetObject);
}

VoucherService::~VoucherService() {
    delete m_dispatch;
}

PyResult VoucherService::Handle_GetObject( PyCallArgs& call ) {
  /**
    voucher = self.GetVoucherSvc().GetObject(voucherID)
    if voucher is None:
        return
    self.data[voucherID] = voucher
    */

    //call.Dump(BOOKMARK__CALL_DUMP);
    // return none for now, to allow client to use default name of 'bookmark'
    //return PyStatic.NewNone();

    PyDict* dict = new PyDict();
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return dict;
    }
    InventoryItemRef iRef = sItemFactory.GetItem(arg.arg);
    if (iRef.get() == nullptr) {
        codelog(ITEM__ERROR, "%s: Failed to retrieve bookmark voucher for bmID %u", call.client->GetName(), arg.arg);
        return dict;
    }

    // this is how i return objects for method chaining
    //we just bind up a new voucher object for item requested and give it back to them.
    VoucherBound *vb = new VoucherBound(m_manager, iRef);
    return m_manager->BindObject(call.client, vb );
}


PyCallable_Make_InnerDispatcher(VoucherBound)

VoucherBound::VoucherBound(PyServiceMgr* mgr, InventoryItemRef itemRef)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "VoucherBound";

    m_itemRef = itemRef;

    PyCallable_REG_CALL(VoucherBound, GetDescription);
}

VoucherBound::~VoucherBound()
{
    delete m_dispatch;
}

PyResult VoucherBound::Handle_GetDescription(PyCallArgs &call) {
    //   name = voucher.GetDescription()

    // get bookmark name (memo) as stored in db.  item.customInfo is bookmarkID this item is copied from
    return new PyString(BookmarkDB::GetBookmarkName(atoi(m_itemRef->customInfo().c_str())));
}

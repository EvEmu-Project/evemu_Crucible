/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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


#ifndef EVEMU_INVENTORY_VOUCHER_H_
#define EVEMU_INVENTORY_VOUCHER_H_

#include "PyService.h"
#include "PyBoundObject.h"

class VoucherService : public PyService {
public:
    VoucherService(PyServiceMgr *mgr);
    virtual ~VoucherService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;


    PyCallable_DECL_CALL(GetObject);

};


class VoucherBound
: public PyBoundObject
{
public:
    VoucherBound(PyServiceMgr* mgr, InventoryItemRef itemRef);

    virtual ~VoucherBound();

    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetDescription);

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;


private:
    InventoryItemRef m_itemRef;

};

#endif  // EVEMU_INVENTORY_VOUCHER_H_

//voucher types:  share, playerkill, bookmark

/*typeID        typeName        description
49      Player Kill     Kill confirmation
50      Company Shares  Shares of a corporation.
51      Bookmark
29247   Loyalty Points  Loyalty points are rewarded to capsuleers for successfully completing missions for a corporation, which can be redeemed for special items from that corporation.



To see your current amount of loyalty points, open your journal and click on the Loyalty Points tab.



To spend them, open the LP Store in any station owned by a corporation with which you have loyalty points.



For more information, please refer to the EVElopedia article about <a href="evebrowser: http://wiki.eveonline.com/en/wiki/Loyalty_point">loyalty points</a>.
*/
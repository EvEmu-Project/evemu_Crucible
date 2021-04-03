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
    Author:     Zhur, Captnoord
*/

#ifndef _INVENTORY_BOUND_H
#define _INVENTORY_BOUND_H

#include "PyBoundObject.h"
#include "PyService.h"

class InventoryBound
: public PyBoundObject
{
public:
    InventoryBound(PyServiceMgr *mgr, InventoryItemRef item, EVEItemFlags flag, uint32 ownerID,  bool passive);
    virtual ~InventoryBound();

    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(List);
    PyCallable_DECL_CALL(Add);
    PyCallable_DECL_CALL(MultiAdd);
    PyCallable_DECL_CALL(GetItem);
    PyCallable_DECL_CALL(RemoveChargeToCargo);
    PyCallable_DECL_CALL(RemoveChargeToHangar);
    PyCallable_DECL_CALL(MultiMerge);
    PyCallable_DECL_CALL(StackAll);
    PyCallable_DECL_CALL(StripFitting);
    PyCallable_DECL_CALL(DestroyFitting);
    PyCallable_DECL_CALL(SetPassword);
    PyCallable_DECL_CALL(CreateBookmarkVouchers);
    PyCallable_DECL_CALL(RunRefiningProcess);
    PyCallable_DECL_CALL(ImportExportWithPlanet);
    PyCallable_DECL_CALL(TakeOutTrash);
    PyCallable_DECL_CALL(ListDroneBay);


protected:
    bool m_passive;     // still not sure what this is for
    EVEItemFlags m_flag;

    uint32 m_itemID;
    uint32 m_ownerID;

    class Dispatcher;
    Dispatcher *const m_dispatch;
    Inventory* pInventory;

    InventoryItemRef m_self;

    std::vector< int32 > CatSortItems(std::vector< InventoryItemRef >& itemVec);

    PyRep* MoveItems(Client* pClient, std::vector< int32 >& items, EVEItemFlags toFlag, int32 quantity, bool manyFlags, float capacity);
};

#endif//_INVENTORY_BOUND_H

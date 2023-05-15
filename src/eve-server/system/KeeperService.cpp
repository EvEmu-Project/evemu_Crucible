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
    Author:        Zhur
    Updates:    Allan
*/

/* Dungeon Logging
 * DUNG__ERROR
 * DUNG__WARNING
 * DUNG__INFO
 * DUNG__MESSAGE
 * DUNG__TRACE
 * DUNG__CALL
 * DUNG__CALL_DUMP
 * DUNG__RSP_DUMP
 * DUNG__DB_ERROR
 * DUNG__DB_WARNING
 * DUNG__DB_INFO
 * DUNG__DB_MESSAGE
 */

#include "eve-server.h"

#include "system/KeeperService.h"
#include "system/SystemManager.h"
#include "dungeon/DungeonDB.h"
#include "services/ServiceManager.h"
#include "system/cosmicMgrs/DungeonMgr.h"

KeeperService::KeeperService(EVEServiceManager& mgr) :
    Service("keeper", eAccessLevel_SolarSystem2),
    m_manager (mgr),
    m_instance (nullptr)
{
    this->Add("GetLevelEditor", &KeeperService::GetLevelEditor);
    this->Add("ActivateAccelerationGate", &KeeperService::ActivateAccelerationGate);
    this->Add("CanWarpToPathPlex", &KeeperService::CanWarpToPathPlex);

    //sm.RemoteSvc('keeper').ClientBSDRevisionChange(action, schemaName, tableName, rowKeys, columnValues, reverting)
}

void KeeperService::BoundReleased (KeeperBound* bound) {
    this->m_instance = nullptr;
}

PyResult KeeperService::GetLevelEditor(PyCallArgs &call)
{
    // self.ed = sm.RemoteSvc('keeper').GetLevelEditor()  (this is to bind new editor object)
    _log(DUNG__CALL,  "KeeperService::Handle_GetLevelEditor  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if (this->m_instance == nullptr) {
        this->m_instance = new KeeperBound (m_manager, *this, &m_db);
    }

    this->m_instance->NewReference (call.client);

    return new PySubStruct(new PySubStream(this->m_instance->GetOID()));
}

PyResult KeeperService::CanWarpToPathPlex(PyCallArgs &call, PyInt* instanceID) {
    /*
                    resp = sm.RemoteSvc('keeper').CanWarpToPathPlex(node.rec.instanceID)
                if resp:
                    if resp is True:
                        m.append((uiutil.MenuLabel('UI/Inflight/WarpToBookmark'), self.WarpToHiddenDungeon, (node.id, node)))
                    else:
                        mickey = sm.StartService('michelle')
                        me = mickey.GetBall(eve.session.shipid)
                        dist = (foo.Vector3(resp) - foo.Vector3(me.x, me.y, me.z)).Length()

                        */
    _log(DUNG__CALL,  "KeeperService::Handle_CanWarpToPathPlex  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

	return nullptr;
}

PyResult KeeperService::ActivateAccelerationGate(PyCallArgs &call, PyInt* itemID) {
    _log(DUNG__CALL,  "KeeperService::Handle_ActivateAccelerationGate  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    Client *pClient(call.client);

    // Send gate activation effect
    pClient->GetShipSE()->DestinyMgr()->SendSpecialEffect10(itemID->value(), 0, "effects.WarpGateEffect", 0, 1, 0);

    // Currently, next rooms are at a fixed distance from previous room (on x axis), so we can just warp the ship in that direction
    GPoint currentPosition(pClient->GetShipSE()->GetPosition());
    GPoint deltaPosition;
    deltaPosition.x = NEXT_DUNGEON_ROOM_DIST;
    deltaPosition.y = 0;
    deltaPosition.z = 0;

    GPoint warpToPoint(currentPosition+deltaPosition);
    GVector vectorToDestination(currentPosition, warpToPoint);
    double distanceToDestination = vectorToDestination.length();
    pClient->GetShipSE()->DestinyMgr()->WarpTo(warpToPoint, distanceToDestination);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(Win32TimeNow());
}

KeeperBound::KeeperBound(EVEServiceManager& mgr, KeeperService& parent, SystemDB* db) :
    EVEBoundObject(mgr, parent),
    m_db(db)
{
    this->Add("EditDungeon", &KeeperBound::EditDungeon);
    this->Add("PlayDungeon", &KeeperBound::PlayDungeon);
    this->Add("Reset", &KeeperBound::Reset);
    this->Add("GotoRoom", &KeeperBound::GotoRoom); //(int room)
    this->Add("GetCurrentlyEditedRoomID", &KeeperBound::GetCurrentlyEditedRoomID);
    this->Add("GetRoomObjects", &KeeperBound::GetRoomObjects);
    this->Add("GetRoomGroups", &KeeperBound::GetRoomGroups);
    this->Add("ObjectSelection", &KeeperBound::ObjectSelection);
    this->Add("BatchStart", &KeeperBound::BatchStart);
    this->Add("BatchEnd", &KeeperBound::BatchEnd);
}

PyResult KeeperBound::EditDungeon(PyCallArgs &call, PyInt* dungeonID)
{
    //ed.EditDungeon(dungeonID, roomID=roomID)
    _log(DUNG__CALL,  "KeeperBound::Handle_EditDungeon  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    /*
    Tasks to accomplish
    1. Set current position as 0,0,0 in room
    2. Load all room objects from dunRoomObjects into a vector
    3. Spawn all room objects using their relative positions from 0,0,0
    */

    Client *pClient(call.client);

    GPoint roomPos = pClient->GetShipSE()->GetPosition();

    pClient->GetSession()->SetFloat("editor_room_x", roomPos.x);
    pClient->GetSession()->SetFloat("editor_room_y", roomPos.y);
    pClient->GetSession()->SetFloat("editor_room_z", roomPos.z);

    pClient->GetSession()->SetInt("editor_bind_id", this->GetBoundID());

    std::vector<Dungeon::RoomObject> objects;
    DungeonDB::GetRoomObjects(call.byname["roomID"]->AsInt()->value(), objects);

    // Spawn the items in the object list
    for (auto cur : objects) {
        GPoint objPos;
        objPos.x = roomPos.x + cur.x;
        objPos.y = roomPos.y + cur.y;
        objPos.z = roomPos.z + cur.z;

        ItemData dData(cur.typeID, 1/*EVE SYSTEM*/, pClient->GetLocationID(), flagNone, "", objPos);
        InventoryItemRef iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), dData);
        if (iRef.get() == nullptr) // Failed to spawn the item
            continue;
        DungeonEditSE* oSE;
        oSE = new DungeonEditSE(iRef, pClient->services(), pClient->SystemMgr(), cur);
        m_roomObjects.push_back(oSE);
        pClient->SystemMgr()->AddEntity(oSE, false);
        // Set the radius from the data structure
        oSE->DestinyMgr()->SetRadius(cur.radius, true);
    }

    // Send notification to client to update UI
    PyList* posList = new PyList();
        posList->AddItem(new PyFloat(roomPos.x));
        posList->AddItem(new PyFloat(roomPos.y));
        posList->AddItem(new PyFloat(roomPos.z));

    PyTuple* payload = new PyTuple(3);
    payload->SetItem(0, dungeonID); //dungeonID
    payload->SetItem(1, new PyInt(call.byname["roomID"]->AsInt()->value())); //roomID
    payload->SetItem(2, posList); //roomPos

    pClient->SendNotification("OnDungeonEdit", "charid", payload, false);

    // update local variables with what we're editing right now
    this->m_currentRoom = call.byname["roomID"]->AsInt()->value();
    this->m_currentDungeon = dungeonID->value();

    return nullptr;
}

PyResult KeeperBound::GetRoomObjects(PyCallArgs &call)
{
    _log(DUNG__CALL,  "KeeperBound:::Handle_GetRoomObjects  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    DBRowDescriptor *header = new DBRowDescriptor();
    header->AddColumn("objectID", DBTYPE_I4);
    header->AddColumn("groupID", DBTYPE_I4);

    CRowSet *rowset = new CRowSet(&header);

    for (auto cur : m_roomObjects) {
        PyPackedRow *newRow = rowset->NewRow();
        newRow->SetField("objectID", new PyInt(cur->GetID()));
        newRow->SetField("groupID", new PyInt(cur->GetData().groupID));
    }

    return rowset;
}

PyResult KeeperBound::GetRoomGroups( PyCallArgs& call, PyInt* roomID )
{
    _log(DUNG__CALL,  "KeeperBound::Handle_GetRoomGroups  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return DungeonDB::GetRoomGroups(roomID->value());
}

PyResult KeeperBound::PlayDungeon(PyCallArgs &call, PyInt* dungeonID)
{
    //ed.PlayDungeon(dungeonID, roomID=roomID, godmode=godmode)
    _log(DUNG__CALL,  "KeeperBound::Handle_PlayDungeon  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    // Play the currently selected dungeon at the player's current position
    sDunDataMgr.UpdateDungeon(dungeonID->value());
    
    Client *pClient(call.client);

    CosmicSignature sig;
    sig.systemID = pClient->GetSystemID();
    sig.ownerID = pClient->GetCharacterID();
    sig.sigName = "DebugDungeon";
    sig.position = pClient->GetShipSE()->GetPosition();

    pClient->SystemMgr()->GetDungMgr()->MakeDungeon(sig, dungeonID->value());

    _log(DUNG__INFO, "KeeperBound::PlayDungeon() created dungeon %u for playtest.", dungeonID->value());

    return nullptr;
}

PyResult KeeperBound::Reset(PyCallArgs &call)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_Reset  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if (this->m_roomObjects.size())
        // reset means unload everything
        for (auto cur : m_roomObjects)
            cur->Delete();

    // empty the list
    this->m_roomObjects.clear();

    // make sure state is sent, this should call the correct flow in the client to update the item
    call.client->SetStateSent(false);
    call.client->GetShipSE()->DestinyMgr()->SendSetState();

    return nullptr;
}

PyResult KeeperBound::GotoRoom(PyCallArgs &call, PyInt* roomID)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_GotoRoom  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::GetCurrentlyEditedRoomID(PyCallArgs &call)
{
//return sm.RemoteSvc('keeper').GetLevelEditor().GetCurrentlyEditedRoomID()
    _log(DUNG__CALL,  "KeeperBound::Handle_GetCurrentlyEditedRoomID  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult KeeperBound::ObjectSelection(PyCallArgs &call, PyList* objects)
{
    _log(DUNG__CALL,  "KeeperBound::Handle_ObjectSelection  size: %lli", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    
    for (auto cur : objects->items) {
        this->m_selectedObjects.push_back(cur->AsInt()->value());
    }

    // Return a copy of the list to the client
    return new PyList(*objects);
}

PyResult KeeperBound::BatchStart(PyCallArgs &call)
{
    // nothing needed for now
    // might be used by CCP to lock selected items or something
    return nullptr;
}

PyResult KeeperBound::BatchEnd(PyCallArgs &call)
{
    // make sure state is sent, this should call the correct flow in the client to update the item
    call.client->SetStateSent(false);
    call.client->GetShipSE()->DestinyMgr()->SendSetState();

    // send new set state to the client
    return nullptr;
}

void KeeperBound::RemoveRoomObject(uint32 itemID)
{
    uint32 objectID = 0;
    for (std::vector <DungeonEditSE*> ::iterator cur = m_roomObjects.begin(); cur != m_roomObjects.end(); cur++) {
        if ((*cur)->GetID() == itemID) {
            objectID = (*cur)->GetData().objectID;
            (*cur)->Delete();
            SafeDelete(*cur);
            m_roomObjects.erase(cur);
            break;
        }
    }

    DungeonDB::DeleteObject(objectID);
}

DungeonEditSE* KeeperBound::GetRoomObject(uint32 itemID)
{
    for (std::vector <DungeonEditSE*> ::iterator cur = m_roomObjects.begin(); cur != m_roomObjects.end(); cur++) {
        if ((*cur)->GetID() == itemID)
            return (*cur);
    }

    return nullptr;
}
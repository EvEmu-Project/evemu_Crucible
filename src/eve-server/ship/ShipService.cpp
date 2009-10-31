/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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

PyCallable_Make_InnerDispatcher(ShipService)

class ShipBound : public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(ShipBound)

    ShipBound(PyServiceMgr *mgr, ShipDB& db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(ShipBound, Board)
        PyCallable_REG_CALL(ShipBound, Undock)
        PyCallable_REG_CALL(ShipBound, AssembleShip)
        PyCallable_REG_CALL(ShipBound, Drop)
        PyCallable_REG_CALL(ShipBound, ScoopDrone)
        PyCallable_REG_CALL(ShipBound, Jettison)
    }

    virtual ~ShipBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(Board)
    PyCallable_DECL_CALL(Undock)
    PyCallable_DECL_CALL(AssembleShip)
    PyCallable_DECL_CALL(Drop)
    PyCallable_DECL_CALL(ScoopDrone)
    PyCallable_DECL_CALL(Jettison)
    PyCallable_DECL_CALL(Eject)

protected:
    ShipDB& m_db;
    Dispatcher *const m_dispatch;
};


ShipService::ShipService(PyServiceMgr *mgr)
: PyService(mgr, "ship"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

//  PyCallable_REG_CALL(ShipService, )
}

ShipService::~ShipService() {
    delete m_dispatch;
}

PyBoundObject *ShipService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    _log(CLIENT__MESSAGE, "ShipService bind request for:");
    bind_args->Dump(CLIENT__MESSAGE, "    ");

    return(new ShipBound(m_manager, m_db));
}

PyResult ShipBound::Handle_Board(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

    if(call.client->IsInSpace()) {
        //TODO: handle the logistics of this!
        codelog(CLIENT__ERROR, "%s: Boarding a ship in space is not yet implemented!!!!", call.client->GetName());
    }

    ShipRef ship = m_manager->item_factory.GetShip( args.arg );
    if( !ship ) {
        _log(CLIENT__ERROR, "%s: Failed to get new ship %u.", call.client->GetName(), args.arg);
    }
    else
        call.client->BoardShip( ship );

    return NULL;
}


PyResult ShipBound::Handle_Undock(PyCallArgs &call) {
    Call_SingleBoolArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    //int ignoreContraband = args.arg;

    GPoint dockPosition;
    if(!m_db.GetStationInfo(call.client->GetLocationID(), NULL, NULL, NULL, NULL, &dockPosition, NULL)) {
        _log(SERVICE__ERROR, "%s: Failed to query location of station %u for undock.", call.client->GetName(), call.client->GetLocationID());
        //TODO: throw exception
        return NULL;
    }


    _log(CLIENT__ERROR, "undock not fully implemented!");

    //send an OnItemChange notification
    // tuple:
    //  util.Row for item,
    //      customInfo=Undocking:(insert station ID)
    //  dict:
    //      3->(insert station ID)
    //      4->4

    //Update the custom info field.
    char ci[256];
    snprintf(ci, sizeof(ci), "Undocking:%u", call.client->GetLocationID());
    call.client->GetShip()->SetCustomInfo(ci);

    //do session change...
    call.client->MoveToLocation(call.client->GetSystemID(), dockPosition);

    //revert custom info, for testing.
    call.client->GetShip()->SetCustomInfo(NULL);

    call.client->OnCharNoLongerInStation();
    //should get a stationSvc.GetSolarSystem(solarsystemID)

    return NULL;
}

PyResult ShipBound::Handle_AssembleShip(PyCallArgs &call) {
    //TODO: Return correct values
    Call_AssembleShip args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    //Start sloppy implementation
    InventoryItemRef item = m_manager->item_factory.GetItem( args.items.front() );
    if( !item )
    {
        _log( ITEM__ERROR, "Failed to load ship %u to assemble.", args.items.front() );
        return NULL;
    }

    item->ChangeSingleton(true, true);
    //TODO: something...

    return NULL;
}

PyResult ShipBound::Handle_Drop(PyCallArgs &call) {
    if(!IsSolarSystem(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return(new PyList());
    }

    Call_SingleIntList args;
    Call_Drop2 dropargs;
    Call_SingleIntList successfully_dropped;
    std::vector<int32>::const_iterator cur, end;
    uint32 contID = 0;

    PyTuple * t = call.tuple;
    bool d2 = t->items.size() == 2;

    if (d2) {
        if(!dropargs.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

        cur = dropargs.toDrop.begin();
        end = dropargs.toDrop.end();
    } else {
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Failed to decode arguments");
            //TODO: throw exception
            return NULL;
    }

    cur = args.ints.begin();
    end = args.ints.end();
    }

    for(; cur != end; cur++) {
        InventoryItemRef item = m_manager->item_factory.GetItem( *cur );
        if( !item ) {
            _log(SERVICE__ERROR, "%s: Unable to find item %u to drop.", call.client->GetName(), *cur);
            continue;
        }

        //verify that this item is in fact in the player's ship.
        if(item->locationID() != call.client->GetShipID()) {
            _log(SERVICE__ERROR, "%s: Item %u is not in our ship (%u), it is in %u. Not dropping.", call.client->GetName(), *cur, call.client->GetShipID(), item->locationID());
            continue;
        }
        //TODO: check ownership?

        if(item->flag() == flagDroneBay && item->categoryID() == EVEDB::invCategories::Drone) {
            if( call.client->LaunchDrone( item ) )
                successfully_dropped.ints.push_back(item->itemID());
        } else {
            //TODO: drop the crap into a can in space.
            codelog(SERVICE__ERROR, "%s: Dropping non-drones is not yet implemented.", call.client->GetName());

            if (d2) {
                // We just dropped something for our corporation's new station into space
                // Let's put it into space and mark it as ours
            } else {
                // We just dropped something into space
                // Either create a new container or put it into an existing one

                if (contID == 0) {
                    // Create new container
                    // Set contID as this new ID
                }
                // Add item into container
            }

        }
    }

    return(successfully_dropped.Encode());
}

PyResult ShipBound::Handle_ScoopDrone(PyCallArgs &call) {
    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    if(!IsSolarSystem(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return NULL;
    }

    std::vector<int32>::const_iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();
    for(; cur != end; cur++) {
        SystemManager *sm = call.client->System();
        SystemEntity *npc = sm->get(*cur);
        if(npc == NULL) {
            _log(SERVICE__ERROR, "%s: Unable to find drone %u to scoop.", call.client->GetName(), *cur);
            continue;
        }

        //TODO: check to see that this is really a drone.
        //TODO: check ownership/control.
        //TODO: check range.
        //TODO: check drone bay capacity.
        //TODO: check skills?

        InventoryItemRef item = npc->Item();
        item->Move(call.client->GetShipID(), flagDroneBay);

        //I am not happy with this memory management model...
        delete npc; //should remove itself from everything.
    }

    return NULL;
}

PyResult ShipBound::Handle_Jettison(PyCallArgs &call) {
    //TODO: Add correct return values

    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    if( !call.client->IsInSpace() ) {
        _log(SERVICE__ERROR, "%s: Trying to jettison items when not in space!", call.client->GetName());
        return NULL;
    }


    //spawn cargo container
    ItemData idata(
        23, //23 = Cargo Container
        call.client->GetCharacterID(),  //owner
        call.client->GetLocationID(),
        flagAutoFit
    );

    InventoryItemRef cargoItem = m_manager->item_factory.SpawnItem(idata);
    if( !cargoItem ) {
        call.client->SendErrorMsg("Unable to spawn a container for jettison.");
        return NULL;
    }

    //Get location of our ship
    //GPoint loc(call.client->GetPosition());
    //NPC *cargoNpc = new NPC(call.client->System(), m_manager, cargoItem, call.client->GetCorporationID(), loc);

    //args contains id's of items to jettison
    std::vector<int32>::const_iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();

    for(; cur != end; cur++) {
        //Move item into cargo Container
        call.client->MoveItem(*cur, cargoItem->itemID(), flagAutoFit);
    }

    //TODO: properly add the item to the system manager and send the AddBall


    //add cargo container to system
    //cargoNpc->Destiny()->SendAddBall();
    //TODO: Send notification SFX effects.jettison
    return NULL;
}


PyResult ShipBound::Handle_Eject(PyCallArgs &call) {
    //no arguments.

    //spawn capsule (inside ship, flagCapsule, singleton)
    //change location of capsule from old ship to systemID (flag 56->0)

    //send session change for shipID change

    return NULL;
    //things which came after the return (but probably do not have to...

    //DoDestinyUpdate
        //addball for only the capsule.

    //DoDestinyUpdate
        //AddBalls:
            //old ship (STOP, w/ship and mass)
            //capsule (STOP, w/ship and mass)
            //and our newbie ship (STOP, w/ship and mass)
        //a bunch of OnModuleAttributeChange and OnGodmaShipEffect for old ship and capsule

    //DoDestinyUpdate
        //effects.Jettison
        //SetBallInteractive (old ship, false)
        //OnSlimItemChange (old ship, SlimItem)
        //SetBallInteractive (capsule, true)
        //OnSlimItemChange (capsule, SlimItem)
        //SetMaxSpeed (capsule)
}



























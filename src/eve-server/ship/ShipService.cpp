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

        m_strBoundObjectName = "ShipBound";

        PyCallable_REG_CALL(ShipBound, Board)
        PyCallable_REG_CALL(ShipBound, Undock)
        PyCallable_REG_CALL(ShipBound, AssembleShip)
        PyCallable_REG_CALL(ShipBound, Drop)
        PyCallable_REG_CALL(ShipBound, Scoop)
        PyCallable_REG_CALL(ShipBound, ScoopDrone)
        PyCallable_REG_CALL(ShipBound, Jettison)
		PyCallable_REG_CALL(ShipBound, Eject)
		PyCallable_REG_CALL(ShipBound, LeaveShip)
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
    PyCallable_DECL_CALL(Scoop)
    PyCallable_DECL_CALL(ScoopDrone)
    PyCallable_DECL_CALL(Jettison)
    PyCallable_DECL_CALL(Eject)
	PyCallable_DECL_CALL(LeaveShip)

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

    // Save position for old ship
    GPoint shipPosition = call.client->GetPosition();

	// Get ShipRef of our current ship:
	ShipRef oldShipRef = call.client->GetShip();

    if(!args.Decode(&call.tuple)) {
        sLog.Error("ShipBound::Handle_Board()", "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }
	
    // Get ShipRef of the ship we want to board:
	ShipRef boardShipRef = m_manager->item_factory.GetShip( args.arg );                 // This is too inefficient, use line below
    //ShipRef boardShipRef = call.client->System()->GetShipFromInventory( args.arg );
    ShipEntity * pShipEntity = (ShipEntity *)(call.client->System()->get( args.arg ));

    if(call.client->IsInSpace())
    {
        // If client is in space and they are boarding a ship and they are in a pod, the pod must be consumed,
        // and session transerfered, exactly as it is if the person boards a ship from a pod in a station.
        // However, if they are not in a pod, then that ship needs to be left there, so a new entity is created for the
        // ship being left behind and ownership transferred to the EVE system

		if( !boardShipRef )
        {
			sLog.Error("ShipBound::Handle_Board()", "%s: Failed to get new ship %u.", call.client->GetName(), args.arg);
		}
		else
		{
            // Check for boarding perimeter maximum distance, after subtracting ship's radius
            // from the distance from player's existing ship to the dead center of the ship to board:
            GVector boardingVector( shipPosition, pShipEntity->GetPosition() );
            double rangeToBoardShip = boardingVector.length() - boardShipRef->GetAttribute(AttrRadius).get_float();
            if( rangeToBoardShip < 0.0)
                rangeToBoardShip = 0.0;

            if( rangeToBoardShip <= 1500 )  // range to board ship must be less than 1500m
            {
		        if( boardShipRef->ValidateBoardShip( boardShipRef, call.client->GetChar()) )
			    {
                    // Remove the pod from the bubble manager:
                    // TODO: THIS DOES NOT REMOVE POD FROM SPACE.  POD IS STILL VISIBLE IN THIS CHARACTER'S CLIENT
                    // *** There could be an OnItemChangeNotify that is missing...
                    call.client->System()->bubbles.Remove( call.client, true );

                    // Remove the pilot-less ship to be boarded from the bubble manager then delete the Ship Entity object:
                    call.client->System()->bubbles.Remove( pShipEntity, true );
                    if( pShipEntity != NULL )
                        pShipEntity->~ShipEntity();

                    // Change ownership of new ship to this character,
                    // move from current ship into new ship:
                    boardShipRef->ChangeOwner( call.client->GetCharacterID(), true );
				    call.client->BoardShip( boardShipRef );

                    //oldShipRef->DisableSaveTimer();     // Stop auto-save timer for old ship
                    //boardShipRef->EnableSaveTimer();    // Start auto-save timer for new ship

				    if( oldShipRef->typeID() == itemTypeCapsule )
                    {
                        // Move pod out of inventory, then delete its Inventory Item object:
					    call.client->MoveItem( oldShipRef->itemID(), 0, (EVEItemFlags)flagNone );
                        if( oldShipRef )
					        oldShipRef->Delete();
				    }
				    else
				    {
                        // Create new ShipEntity for old ship, now that we've left it, and add it to the System Manager

                        // Set ownership of old ship to EVE system:
                        oldShipRef->ChangeOwner( 1 );

                        // Move the ItemRef to SystemManagers' Inventory:
                        oldShipRef->Move( call.client->System()->GetID(), flagShipOffline );

                        // Set position of the actual ShipRef of the old ship to the same position as the ShipEntity:
                        oldShipRef->Relocate( shipPosition );

                        // Create new ShipEntity for old ship and add it to the SystemManager:
                        ShipEntity * oldShipObj = new ShipEntity( oldShipRef, call.client->System(), *(call.client->System()->GetServiceMgr()), shipPosition );
                        call.client->System()->AddEntity( oldShipObj );

                        // Add ball to bubble manager for this client's character's system for the old pilot-less ship:
                        call.client->System()->bubbles.Add( oldShipObj, true );
                    }

                    // Now, in new ship, add self back to the bubble manager:
                    call.client->MoveToLocation( call.client->GetSystemID(), boardShipRef->position() );
                    call.client->System()->bubbles.Add( call.client, true );
                    //call.client->Destiny()->SetPosition( boardShipRef->position(), true );

                    // Send Destiny Updates:
                    call.client->Destiny()->SendBoardShip( boardShipRef );

                    return NULL;
			    }
			    else
				    throw PyException( MakeCustomError( "You do not have the required skills to fly a %s", boardShipRef->itemName().c_str() ) );		
            }
		}

		return NULL;
	}

    // Player is NOT in space, so board ship as if in station:
    if( !boardShipRef ) {
        sLog.Error("ShipBound::Handle_Board()", "%s: Failed to get new ship %u.", call.client->GetName(), args.arg);
    }
    else
	{
		if( boardShipRef->ValidateBoardShip( boardShipRef, call.client->GetChar()) )
		{
			call.client->BoardShip( boardShipRef );
			
			//consume pod, as it is placed inside the ship
			if( oldShipRef->typeID() == itemTypeCapsule )
            {
				call.client->MoveItem( oldShipRef->itemID(), 0, (EVEItemFlags)flagNone );
				oldShipRef->Delete();
			}

            // Update bubble manager for this client's character's ship:
            //call.client->MoveToLocation( call.client->GetLocationID(), GPoint(0.0,0.0,0.0) );
            call.client->GetShip()->Move( call.client->GetLocationID(), (EVEItemFlags)flagHangar, true );
            ////call.client->services().item_factory.GetShip( call.client->GetShipID() )->Relocate( GPoint(0.0,0.0,0.0) );
            ////call.client->Destiny()->SetPosition( GPoint(0.0,0.0,0.0), true );
            call.client->System()->bubbles.UpdateBubble( call.client, true );

			return NULL;
		}
		else
			throw PyException( MakeCustomError( "You do not have the required skills to fly a %s", boardShipRef->itemName().c_str() ) );		
	}

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
    GVector dockOrientation;
    if(!m_db.GetStationInfo(call.client->GetLocationID(), NULL, NULL, NULL, NULL, &dockPosition, &dockOrientation)) {
        _log(SERVICE__ERROR, "%s: Failed to query location of station %u for undock.", call.client->GetName(), call.client->GetLocationID());
        //TODO: throw exception
        return NULL;
    }

    sLog.Error( "ShipBound::Handle_Undock()", "!!! Undock not fully implemented!");

    //send an OnItemChange notification
    // tuple:
    //  util.Row for item,
    //      customInfo=Undocking:(insert station ID)
    //  dict:
    //      3->(insert station ID)
    //      4->4
	/*
	PyTuple *tmp = new PyTuple;
		util.Row
	
	call.client->SendNotification("OnItemChange", "charid", &tmp, false);
	*/

    //Update the custom info field.
    char ci[256];
    snprintf(ci, sizeof(ci), "Undocking:%u", call.client->GetLocationID());
    call.client->GetShip()->SetCustomInfo(ci);

    //do session change...
    call.client->MoveToLocation(call.client->GetSystemID(), dockPosition);

	//calculate undock movement
    //GPoint dest = GPoint( call.client->x()+10000, call.client->y(), call.client->z() );
	GPoint dest = 
        GPoint
        (
            call.client->x() + dockOrientation.x*1.0e6,
            call.client->y() + dockOrientation.y*1.0e6,
            call.client->z() + dockOrientation.z*1.0e6
        );

	//move away from dock
    //call.client->Destiny()->AlignTo( dest, true );
    //call.client->Destiny()->SetSpeedFraction( 1.0, true );
	
	//prevent client from stopping ship automatically stopping - this is sloppy


    //revert custom info, for testing.
    call.client->GetShip()->SetCustomInfo(NULL);

    call.client->OnCharNoLongerInStation();
    //should get a stationSvc.GetSolarSystem(solarsystemID)

    // THIS IS A HACK AS WE DONT KNOW WHY THE CLIENT CALLS STOP AT UNDOCK
    // SO SAVE THE UNDOCK ALIGN-TO POINT AND TELL CLIEN WE JUST UNDOCKED
    call.client->SetUndockAlignToPoint( dest );
    call.client->SetJustUndocking( true );
    // --- END HACK ---

    return NULL;
}

PyResult ShipBound::Handle_AssembleShip(PyCallArgs &call) {
    //TODO: Return correct values
    Call_AssembleShip args;
    Call_AssembleShipTech3 argsT3;
    uint32 itemID = 0;
    std::vector<uint32> subSystemList;
    bool completeTech3Assembly = false;

    if( !(call.tuple->IsTuple()) )
        return NULL;

    if( !(call.tuple->GetItem(0)->IsList()) )
    {
        if( !(call.tuple->GetItem(0)->IsInt()) )
        {
            sLog.Error( "ShipBound::Handle_AssembleShip()", "Failed to decode arguments: call.tuple->GetItem(0)->IsInt() == false");
            //TODO: throw exception
            return NULL;
        }
        else
        {
            // Tuple contains single Integer, this is for Tech 3 Ship Assembly:
            if( !argsT3.Decode(&call.tuple) )
            {
                sLog.Error( "ShipBound::Handle_AssembleShip()", "Failed to decode arguments: argsT3.Decode(&call.tuple) failed");
                //TODO: throw exception
                return NULL;
            }
            itemID = argsT3.item;
            if( call.byname.find("subSystems") != call.byname.end() )
            {
                PyList * list;
                if( call.byname.find("subSystems")->second->IsList() )
                {
                    list = call.byname.find("subSystems")->second->AsList();
                    for(uint32 index=0; index<list->size(); index++)
                        subSystemList.push_back( list->GetItem( index )->AsInt()->value() );
                }
                else
                {
                    sLog.Error( "ShipBound::Handle_AssembleShip()", "Failed to decode arguments: !call.byname.find(\"subSystems\")->second->IsList() failed");
                    //TODO: throw exception
                    return NULL;
                }
            }
            else
            {
                sLog.Error( "ShipBound::Handle_AssembleShip()", "Failed to decode arguments: call.byname.find(\"subSystems\") != call.byname.end() failed" );
                return NULL;
            }
            completeTech3Assembly = true;
        }
    }
    else
    {
        if(!args.Decode(&call.tuple))
        {
            codelog(SERVICE__ERROR, "Failed to decode arguments");
            //TODO: throw exception
            return NULL;
        }
        itemID = args.items.front();
    }

    ShipRef ship = m_manager->item_factory.GetShip( itemID );

    if( !ship )
    {
        _log( ITEM__ERROR, "Failed to load ship %u to assemble.", itemID );
        return NULL;
    }
	
	//check if the ship is a stack
	if( ship->quantity() > 1 )
	{
		//Split the stack and assemble one ship
		InventoryItemRef new_item = ship->Split(1,true);
	}

    ship->ChangeSingleton(true, true);

    if( completeTech3Assembly )
    {
        // Move the five specified subsystems to the newly assembled Tech 3 ship
        InventoryItemRef subSystemItem;
        for(uint32 index=0; index<subSystemList.size(); index++)
        {
            subSystemItem = m_manager->item_factory.GetItem( subSystemList.at( index ) );
            subSystemItem->MoveInto( *ship, (EVEItemFlags)(subSystemItem->GetAttribute(AttrSubSystemSlot).get_int()), true );
        }
    }

    return NULL;
}

PyResult ShipBound::Handle_Drop(PyCallArgs &call) {
    if(!IsSolarSystem(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return(new PyList());
    }

    Call_Drop3 drop3args;
    Call_SingleIntList successfully_dropped;

    std::vector< std::pair< uint32, uint32 > > toDropList;
    uint32 contID = 0;

    //Get location of our ship
    GPoint location(call.client->GetPosition());
    double radius;
    double theta;
    double phi;

    InventoryItemRef cargoItem;
    StructureRef structureItem;
    CargoContainerRef cargoContainerItem;
    uint32 groupID = 0;
    uint32 categoryID = 0;

    PyTuple * tuple = call.tuple;
    PyList * PyToDropList;
    uint32 ownerID;
    bool unknown;
    bool d3 = tuple->items.size() == 3;

    if(!drop3args.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    if( d3 )
    {
        if( !(drop3args.toDrop->IsList()) )
        {
            sLog.Error("ShipBound::Handle_Drop()", "toDrop argument type is wrong, expected PyList");
            //TODO: throw exception
            return NULL;
        }

        PyToDropList = drop3args.toDrop;
        ownerID = (uint32)(drop3args.ownerID);
        unknown = drop3args.unknown;
    }
    else
    {
        sLog.Error("ShipBound::Handle_Drop()", "call.tuple wrong size, expected 3 items, actual size = %u", call.tuple->size());
        //TODO: throw exception
        return NULL;
    }

    uint32 itemID;
    uint32 itemQuantity;
    for( uint32 i=0; i<PyToDropList->size(); i++ )
    {
        itemID = (uint32)(PyToDropList->items.at(i)->AsTuple()->items.at(0)->AsInt()->value());
        itemQuantity = (uint32)(PyToDropList->items.at(i)->AsTuple()->items.at(1)->AsInt()->value());

        cargoItem = m_manager->item_factory.GetItem( itemID );
        if( !cargoItem )
        {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Unable to find item %u to drop.", call.client->GetName(), itemID);
            continue;
        }

        //verify that this item is in fact in the player's ship.
        if( cargoItem->locationID() != call.client->GetShipID() )
        {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Item %u is not in our ship (%u), it is in %u. Not dropping.", call.client->GetName(), itemID, call.client->GetShipID(), cargoItem->locationID());
            continue;
        }

        // Change ownership to ownerID
        if ( ownerID != call.client->GetCharacterID() )
            cargoItem->ChangeOwner( ownerID, true );

        // Get groupID and categoryID for item 'itemID' to determine if it is a kind of cargo container, structure, or deployable item
        groupID = m_manager->item_factory.GetItem( itemID )->groupID();
        categoryID = m_manager->item_factory.GetItem( itemID )->categoryID();

        if( (groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
            || (groupID == EVEDB::invGroups::Secure_Cargo_Container)
            || (groupID == EVEDB::invGroups::Freight_Container) )
        {
            // This item IS a cargo container, so move it from the ship's cargo into space:
            cargoContainerItem = m_manager->item_factory.GetCargoContainer( itemID );
            Client * who = call.client;
	        GPoint location( who->GetPosition() );
            radius = 1500.0;
            theta = MakeRandomFloat( 0.0, (2*M_PI) );
            phi = MakeRandomFloat( 0.0, (2*M_PI) );
            location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
            location.y += radius * sin(theta) * sin(phi);
            location.z += radius * cos(theta);

	        if( !cargoContainerItem )
                throw PyException( MakeCustomError( "Unable to spawn item of type %u.", cargoContainerItem->typeID() ) );

            // Properly add the container to the system manager
	        SystemManager* sys = who->System();
	        ContainerEntity* containerObj = new ContainerEntity( cargoContainerItem, sys, *m_manager, location );
	        sys->AddEntity( containerObj );

            // Move item from cargo bay to space:
            cargoContainerItem->Relocate( location );
            cargoContainerItem->Move( call.client->GetLocationID(), flagAutoFit, true );

            // Send notification SFX effects.jettison for the jettisoned Container object:
            call.client->Destiny()->SendJettisonCargo( cargoContainerItem );

            successfully_dropped.ints.push_back( cargoContainerItem->itemID() );
        }
        else if( categoryID == EVEDB::invCategories::Structure )
        {
            // This item is a POS structure of some kind, so move it from the ship's cargo into space
            // whilst keeping ownership of it to the character not using the corporation the character belongs to:
            structureItem = m_manager->item_factory.GetStructure( itemID );
            Client * who = call.client;
	        GPoint location( who->GetPosition() );
            radius = 1500.0;
            theta = MakeRandomFloat( 0.0, (2*M_PI) );
            phi = MakeRandomFloat( 0.0, (2*M_PI) );
            location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
            location.y += radius * sin(theta) * sin(phi);
            location.z += radius * cos(theta);

	        if( !structureItem )
                throw PyException( MakeCustomError( "Unable to spawn Structure item of type %u.", structureItem->typeID() ) );

            // Properly add the structure item to the system manager
	        SystemManager* sys = who->System();
	        StructureEntity* structureObj = new StructureEntity( structureItem, sys, *m_manager, location );
	        sys->AddEntity( structureObj );

            // Move item from cargo bay to space:
            structureItem->Relocate( location );
            structureItem->Move( call.client->GetLocationID(), flagAutoFit, true );

            // Send notification SFX effects.jettison for the jettisoned Structure object:
            call.client->Destiny()->SendJettisonCargo( structureItem );

            successfully_dropped.ints.push_back( structureItem->itemID() );
        }
        else if( categoryID == EVEDB::invCategories::Deployable )
        {
            // This item is a Deployable item of some kind, so move it from the ship's cargo into space
            // whilst keeping ownership of it to the character not using the corporation the character belongs to:
            Client * who = call.client;
	        GPoint location( who->GetPosition() );
            radius = 1500.0;
            theta = MakeRandomFloat( 0.0, (2*M_PI) );
            phi = MakeRandomFloat( 0.0, (2*M_PI) );
            location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
            location.y += radius * sin(theta) * sin(phi);
            location.z += radius * cos(theta);

	        //cargoItem = m_manager->item_factory.GetItem( itemID );
	        if( !cargoItem )
                throw PyException( MakeCustomError( "Unable to spawn Deployable item of type %u.", cargoItem->typeID() ) );

            // Properly add the deployable to the system manager
	        SystemManager* sys = who->System();
	        DeployableEntity* deployableObj = new DeployableEntity( cargoItem, sys, *m_manager, location );
	        sys->AddEntity( deployableObj );

            // Move item from cargo bay to space:
            cargoItem->Relocate( location );
            cargoItem->Move( call.client->GetLocationID(), flagAutoFit, true );
            //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
            // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?

            // Send notification SFX effects.jettison for the jettisoned Deployable object:
            call.client->Destiny()->SendJettisonCargo( cargoItem );

            successfully_dropped.ints.push_back( cargoItem->itemID() );
        }
        else if( cargoItem->flag() == flagDroneBay && cargoItem->categoryID() == EVEDB::invCategories::Drone )
        {
            // This item is a drone, so launch it into space:
            if( call.client->LaunchDrone( cargoItem ) )
                successfully_dropped.ints.push_back( cargoItem->itemID() );
        }
        //else if ()    // Handle other types of cargo, such as launching assembled ships?
        else
        {
            // Reject launch for this item
        }
    }

/*
    std::vector<int32>::const_iterator cur, end;
    uint32 contID = 0;

    PyTuple * t = call.tuple;
    bool d2 = t->items.size() == 2;

    if (d2)
    {
        if(!dropargs.Decode(&call.tuple))
        {
            codelog(SERVICE__ERROR, "Failed to decode arguments");
            //TODO: throw exception
            return NULL;
        }

        cur = dropargs.toDrop.begin();
        end = dropargs.toDrop.end();
    }
    else
    {
        if(!args.Decode(&call.tuple))
        {
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
*/
    return(successfully_dropped.Encode());
}

PyResult ShipBound::Handle_Scoop(PyCallArgs &call) {
    if( !(call.tuple->items.at( 0 )->IsInt()) ) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    uint32 objectItemID = call.tuple->items.at( 0 )->AsInt()->value();

    SystemManager *sm = call.client->System();
    SystemEntity *object = sm->get( objectItemID );
    if(object == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to find object %u to scoop.", call.client->GetName(), objectItemID);
        return NULL;
    }

    InventoryItemRef item = object->Item();

    // TODO: check ownership of this object, ie does this character/corporation own this object?
    // do we really need to do this for anything except for drones that are under control of another player?

    // TODO: check to see if this object is anchored and if so, refuse to scoop it

    // DONT NEED TO CHECK RANGE, CLIENT DOES THIS
    //// Check range to drone:
    //GVector rangeVector( call.client->GetShip()->position(), drone->GetPosition() );
    //double rangeToDrone = rangeVector.length();   // This doesn't work
    //if( rangeToDrone > 1500 )
    //    return NULL;        

    // Check cargo bay capacity:
    double capacity = call.client->GetShip()->GetCapacity( flagCargoHold );
    double volume = item->GetAttribute(AttrVolume).get_float();
    if( capacity < volume )
        throw PyException( MakeCustomError( "%s is too large to fit in remaining Cargo bay capacity.", item->itemName().c_str() ) );
    else
    {
        // We have enough Cargo bay capacity to hold the item being scooped,
        // so take ownership of it and move it into the cargo bay:
        item->ChangeOwner( call.client->GetCharacterID(), true );

        call.client->MoveItem( item->itemID(), call.client->GetShipID(), flagCargoHold );
        //item->Move(call.client->GetShipID(), flagCargoHold);

        // Remove object entity from SystemManager:
        call.client->System()->RemoveEntity( object );

        //I am not happy with this memory management model...
        //Aknor agrees because somehow this caused my server to crash...
        delete object; //should remove itself from everything.
    }

    return NULL;
}

PyResult ShipBound::Handle_ScoopDrone(PyCallArgs &call) {
    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    std::vector<int32>::const_iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();
    for(; cur != end; cur++) {
        SystemManager *sm = call.client->System();
        SystemEntity *drone = sm->get(*cur);
        if(drone == NULL) {
            _log(SERVICE__ERROR, "%s: Unable to find drone %u to scoop.", call.client->GetName(), *cur);
            continue;
        }

        InventoryItemRef item = drone->Item();

        // Check to see that this is really a drone:
        call.client->GetShip()->ValidateAddItem( flagDroneBay, item, call.client );

        //TODO: check ownership/control.

        // DONT NEED TO CHECK RANGE, CLIENT DOES THIS
        //// Check range to drone:
        //GVector rangeVector( call.client->GetShip()->position(), drone->GetPosition() );
        //double rangeToDrone = rangeVector.length();   // This doesn't work
        //if( rangeToDrone > 1500 )
        //    return NULL;        

        // Check drone bay capacity:
        double capacity = call.client->GetShip()->GetCapacity( flagDroneBay );
        double volume = item->GetAttribute(AttrVolume).get_float();
        if( capacity < volume )
            throw PyException( MakeCustomError( "%s is too large to fit in remaining Drone bay capacity.", item->itemName().c_str() ) );
        else
        {
            // We have enough Drone bay capacity to hold the drone,
            // so take ownership of it and move it into the Drone bay:
            item->ChangeOwner( call.client->GetCharacterID(), true );

            call.client->MoveItem( item->itemID(), call.client->GetShipID(), flagDroneBay );
            //item->Move(call.client->GetShipID(), flagDroneBay);

            // Remove drone entity from SystemManager:
            call.client->System()->RemoveEntity( drone );

            //I am not happy with this memory management model...
            delete drone; //should remove itself from everything.
        }
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
    
    //Get location of our ship
    GPoint location(call.client->GetPosition());
    double radius;
    double theta;
    double phi;

    //args contains id's of items to jettison
    std::vector<int32>::const_iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();

    InventoryItemRef cargoItem;
    CargoContainerRef newJettisonCargoContainerItem;
    CargoContainerRef cargoContainerItem;
    StructureRef structureItem;
    ItemData * p_idata;
    uint32 groupID = 0;
    uint32 categoryID = 0;

    for(; cur != end; cur++)
    {
        // Get groupID and categoryID for item '*cur' to determine if it is a kind of cargo container, structure, or deployable item
        groupID = m_manager->item_factory.GetItem( *cur )->groupID();
        categoryID = m_manager->item_factory.GetItem( *cur )->categoryID();

        if( (groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
            || (groupID == EVEDB::invGroups::Secure_Cargo_Container)
            || (groupID == EVEDB::invGroups::Freight_Container) )
        {
            // This item IS a cargo container, so move it from the ship's cargo into space:
            Client * who = call.client;
	        GPoint location( who->GetPosition() );
            radius = 1500.0;
            theta = MakeRandomFloat( 0.0, (2*M_PI) );
            phi = MakeRandomFloat( 0.0, (2*M_PI) );
            location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
            location.y += radius * sin(theta) * sin(phi);
            location.z += radius * cos(theta);

            cargoContainerItem = m_manager->item_factory.GetCargoContainer( *cur );
	        if( !cargoContainerItem )
                throw PyException( MakeCustomError( "Unable to spawn item of type %u.", cargoContainerItem->typeID() ) );

            // Properly add the container to the system manager
	        SystemManager* sys = who->System();
	        ContainerEntity* containerObj = new ContainerEntity( cargoContainerItem, sys, *m_manager, location );
	        sys->AddEntity( containerObj );

            // Move item from cargo bay to space:
            cargoContainerItem->Relocate( location );
            cargoContainerItem->Move( call.client->GetLocationID(), flagAutoFit, true );

            // Send notification SFX effects.jettison for the jettisoned Container object:
            call.client->Destiny()->SendJettisonCargo( cargoContainerItem );
        }
        else if( categoryID == EVEDB::invCategories::Structure )
        {
            // This item is a POS structure of some kind, so move it from the ship's cargo into space
            // whilst keeping ownership of it to the character not using the corporation the character belongs to:
            Client * who = call.client;
	        GPoint location( who->GetPosition() );
            radius = 1500.0;
            theta = MakeRandomFloat( 0.0, (2*M_PI) );
            phi = MakeRandomFloat( 0.0, (2*M_PI) );
            location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
            location.y += radius * sin(theta) * sin(phi);
            location.z += radius * cos(theta);

            structureItem = m_manager->item_factory.GetStructure( *cur );
	        if( !structureItem )
                throw PyException( MakeCustomError( "Unable to spawn Structure item of type %u.", structureItem->typeID() ) );

            // Properly add the structure item to the system manager
	        SystemManager* sys = who->System();
	        StructureEntity* structureObj = new StructureEntity( structureItem, sys, *m_manager, location );
	        sys->AddEntity( structureObj );

            // Move item from cargo bay to space:
            structureItem->Relocate( location );
            structureItem->Move( call.client->GetLocationID(), flagAutoFit, true );

            // Send notification SFX effects.jettison for the jettisoned Structure object:
            call.client->Destiny()->SendJettisonCargo( structureItem );
        }
        else if( categoryID == EVEDB::invCategories::Deployable )
        {
            // This item is a Deployable item of some kind, so move it from the ship's cargo into space
            // whilst keeping ownership of it to the character not using the corporation the character belongs to:
            Client * who = call.client;
	        GPoint location( who->GetPosition() );
            radius = 1500.0;
            theta = MakeRandomFloat( 0.0, (2*M_PI) );
            phi = MakeRandomFloat( 0.0, (2*M_PI) );
            location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
            location.y += radius * sin(theta) * sin(phi);
            location.z += radius * cos(theta);

	        cargoItem = m_manager->item_factory.GetItem( *cur );
	        if( !cargoItem )
                throw PyException( MakeCustomError( "Unable to spawn Deployable item of type %u.", cargoItem->typeID() ) );

            // Properly add the deployable to the system manager
	        SystemManager* sys = who->System();
	        DeployableEntity* deployableObj = new DeployableEntity( cargoItem, sys, *m_manager, location );
	        sys->AddEntity( deployableObj );

            // Move item from cargo bay to space:
            cargoItem->Relocate( location );
            cargoItem->Move( call.client->GetLocationID(), flagAutoFit, true );
            //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
            // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?

            // Send notification SFX effects.jettison for the jettisoned Deployable object:
            call.client->Destiny()->SendJettisonCargo( cargoItem );
        }
        //else if ()    // Handle other types of cargo, such as jettisoning assembled ships?
        else
        {
            // This item is NOT a type of cargo container, so spawn a new cargo container in space and place the item
            // inside of it OR place it in the cargo container already created in this function:
            if( !cargoItem )
            {
                // Spawn cargo container for the first time and only time in this function:
                Client * who = call.client;
	            GPoint location( who->GetPosition() );
                radius = 1500.0;
                theta = MakeRandomFloat( 0.0, (2*M_PI) );
                phi = MakeRandomFloat( 0.0, (2*M_PI) );
                location.x += radius * sin(theta) * cos(phi);   // Randomize placement on a 1500m radius sphere about player ship
                location.y += radius * sin(theta) * sin(phi);
                location.z += radius * cos(theta);

	            p_idata = new ItemData(
		            23,                     // 23 = cargo container
                    who->GetCharacterID(),  //owner is Character
		            who->GetLocationID(),
		            flagAutoFit,
		            "Cargo Container",
		            location
	            );

                newJettisonCargoContainerItem = m_manager->item_factory.SpawnCargoContainer( *p_idata );
	            if( !newJettisonCargoContainerItem )
		            throw PyException( MakeCustomError( "Unable to spawn item of type %u.", 23 ) );

	            SystemManager* sys = who->System();
	            ContainerEntity* containerObj = new ContainerEntity( newJettisonCargoContainerItem, sys, *m_manager, location );
	            sys->AddEntity( containerObj );

                // Send notification SFX effects.jettison for the new cargo container:
                call.client->Destiny()->SendJettisonCargo( newJettisonCargoContainerItem );
            }

            // Check to see if this item is allowed to be jettisoned based on categoryID and/or groupID:
            // IDEAS:
            // * Modules, Charges, Skillbooks, Ore, Blueprints, Materials, Corpses...
            if( 1 )
            {
                // Move item into cargo Container
                call.client->MoveItem(*cur, newJettisonCargoContainerItem->itemID(), flagAutoFit);
            }
            else
            {
                // TODO: Handle NON-jettisonable cargo
            }
        }
    }
    return NULL;
}


PyResult ShipBound::Handle_Eject(PyCallArgs &call) {
    //no arguments.

	//get character name
	std::string name = call.client->GetName();
	name += "'s Capsule";

	//save old ship position and itemID
    GPoint shipPosition = call.client->GetPosition();
    uint32 oldShipItemID = call.client->GetShipID();
    GPoint capsulePosition = call.client->GetPosition();

    //set capsule position 500m off from old ship:
    capsulePosition.x += call.client->GetShip()->GetAttribute(AttrRadius).get_float() + 100.0;
    capsulePosition.y += call.client->GetShip()->GetAttribute(AttrRadius).get_float() + 100.0;
    capsulePosition.z += call.client->GetShip()->GetAttribute(AttrRadius).get_float() - 100.0;

    //spawn capsule (inside ship, flagCapsule, singleton)
	ItemData idata(
		name.c_str(),
		itemTypeCapsule,
		call.client->GetCharacterID(),
		call.client->GetLocationID(),
		(EVEItemFlags)flagCapsule,
		false, true, 1,
		capsulePosition,
		""
	);

    // Spawn a new capsule ship inventory item:
    ShipRef capsuleRef = call.client->services().item_factory.SpawnShip( idata );
    if( !capsuleRef )
		throw PyException( MakeCustomError ( "Unable to generate escape pod" ) );

    // Change location of capsule from old ship to SystemManager inventory:
	capsuleRef->Move(call.client->GetLocationID(), (EVEItemFlags)flagCapsule, true);

	ShipRef updatedCapsuleRef = call.client->services().item_factory.GetShip( capsuleRef->itemID() );

    // Remove ball from bubble manager for this client's character's system for the old ship and then
    // board the capsule:
    call.client->System()->bubbles.Remove( call.client, true );
	call.client->BoardShip( updatedCapsuleRef );

    // Add ball to bubble manager for this client's character's system for the new capsule object:
    call.client->System()->bubbles.Add( call.client, true );
    call.client->Destiny()->SetPosition( capsulePosition, true );

    // Get old ship ItemRef
    ShipRef oldShipRef = call.client->services().item_factory.GetShip( oldShipItemID );

    // Set ownership of old ship to EVE system:
    oldShipRef->ChangeOwner( 1 );

    // Move the ItemRef to SystemManagers' Inventory:
    oldShipRef->Move( call.client->System()->GetID(), flagShipOffline );

    // Set position of the actual ShipRef of the old ship to the same position as the ShipEntity:
    oldShipRef->Relocate( shipPosition );

    // Create new ShipEntity for old ship and add it to the SystemManager:
    ShipEntity * oldShipObj = new ShipEntity( oldShipRef, call.client->System(), *(call.client->System()->GetServiceMgr()), shipPosition );
    call.client->System()->AddEntity( oldShipObj );

    // Add ball to bubble manager for this client's character's system for the old pilot-less ship:
    call.client->System()->bubbles.Add( oldShipObj, true );

    // Do Destiny Updates:
    call.client->Destiny()->SendEjectShip( updatedCapsuleRef, oldShipRef );

    return NULL;
}

PyResult ShipBound::Handle_LeaveShip(PyCallArgs &call){
	
	//leave ship into capsule in station

    // TODO:  THIS HAS ISSUES AS THE VIEW IN STATION IS SUCH THAT WHEN A PLAYER LEAVES A SHIP
    // THEIR POD APPEARS IN THE SHIP LIST, WHICH IS FINE, HOWEVER THE SHIP VIEW DOES NOT SHOW THE POD,
    // BUT RATHER THE SHIP THAT WAS JUST "LEFT".  SOMETHING ELSE NEEDS UPDATING TO THE CLIENT.
    // *** There could be an OnItemChangeNotify that is missing...

	//remember the old ship
	ShipRef old_ship = call.client->GetShip();
	
	//get character name
	std::string name = call.client->GetName();
	name += "'s Capsule";

	GPoint capsulePosition;
    capsulePosition.x = 0.0;
    capsulePosition.y = 0.0;
    capsulePosition.z = 0.0;

    //spawn capsule (inside ship, flagCapsule, singleton)
	ItemData idata(
		name.c_str(),
		itemTypeCapsule,
		call.client->GetCharacterID(),
		call.client->GetLocationID(),
		(EVEItemFlags)flagCapsule,
		false, true, 1,
		capsulePosition,
		""
	);
	
	//build the capsule
	ShipRef capsuleRef = call.client->services().item_factory.SpawnShip( idata );

	if( !capsuleRef )
		throw PyException( MakeCustomError ( "Unable to generate escape pod" ) );

	//move capsule into the players hangar
	capsuleRef->Move(call.client->GetLocationID(), (EVEItemFlags)flagHangar, true);

    //send session change for shipID change
	call.client->BoardShip( capsuleRef );

	ShipRef updatedCapsuleRef = call.client->services().item_factory.GetShip( capsuleRef->itemID() );

    // Remove ball from bubble manager for this client's character's system for the old ship and then
    // board the capsule:
    call.client->System()->bubbles.Remove( call.client, true );
	call.client->BoardShip( updatedCapsuleRef );

    // Add ball to bubble manager for this client's character's system for the new capsule object:
    call.client->System()->bubbles.Add( call.client, true );

	return NULL;
}



























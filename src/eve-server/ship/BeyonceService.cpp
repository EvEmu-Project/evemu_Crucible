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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(BeyonceService)

class BeyonceBound
: public PyBoundObject {
public:

	PyCallable_Make_Dispatcher(BeyonceBound)
	
	BeyonceBound(PyServiceMgr *mgr, Client *c)
	: PyBoundObject(mgr),
	  m_dispatch(new Dispatcher(this))
	{
		_SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "BeyonceBound";
		
		PyCallable_REG_CALL(BeyonceBound, FollowBall)
		PyCallable_REG_CALL(BeyonceBound, Orbit)
		PyCallable_REG_CALL(BeyonceBound, AlignTo)
		PyCallable_REG_CALL(BeyonceBound, GotoDirection)
        PyCallable_REG_CALL(BeyonceBound, GotoBookmark)
		PyCallable_REG_CALL(BeyonceBound, SetSpeedFraction)
		PyCallable_REG_CALL(BeyonceBound, Stop)
		PyCallable_REG_CALL(BeyonceBound, WarpToStuff)
		PyCallable_REG_CALL(BeyonceBound, Dock)
		PyCallable_REG_CALL(BeyonceBound, StargateJump)
		PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest)
		PyCallable_REG_CALL(BeyonceBound, WarpToStuffAutopilot)

		if(c->Destiny() != NULL)
			c->Destiny()->SendSetState(c->Bubble());
	}
	virtual ~BeyonceBound() {delete m_dispatch;}
	virtual void Release() {
		//I hate this statement
		delete this;
	}
	
	PyCallable_DECL_CALL(FollowBall)
	PyCallable_DECL_CALL(Orbit)
	PyCallable_DECL_CALL(AlignTo)
	PyCallable_DECL_CALL(GotoDirection)
    PyCallable_DECL_CALL(GotoBookmark)
	PyCallable_DECL_CALL(SetSpeedFraction)
	PyCallable_DECL_CALL(Stop)
	PyCallable_DECL_CALL(WarpToStuff)
	PyCallable_DECL_CALL(Dock)
	PyCallable_DECL_CALL(StargateJump)
	PyCallable_DECL_CALL(UpdateStateRequest)
	PyCallable_DECL_CALL(WarpToStuffAutopilot)

protected:
	Dispatcher *const m_dispatch;
};


BeyonceService::BeyonceService(PyServiceMgr *mgr)
: PyService(mgr, "beyonce"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	//PyCallable_REG_CALL(BeyonceService, )
	PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
	delete m_dispatch;
}


PyBoundObject* BeyonceService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
	_log( CLIENT__MESSAGE, "BeyonceService bind request for:" );
	bind_args->Dump( CLIENT__MESSAGE, "    " );

	return new BeyonceBound( m_manager, c );
}


PyResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
	ObjectCachedMethodID method_id(GetName(), "GetFormations");

	//check to see if this method is in the cache already.
	if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
		//this method is not in cache yet, load up the contents and cache it.
		PyRep *res = m_db.GetFormations();
		if(res == NULL) {
			codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
			res = new PyNone();
		}

		m_manager->cache_service->GiveCache(method_id, &res);
	}
	
	//now we know its in the cache one way or the other, so build a 
	//cached object cached method call result.
	return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
}

/*
PyResult BeyonceService::Handle_(PyCallArgs &call) {
	PyRep *result = NULL;

	return result;
}
*/

PyResult BeyonceBound::Handle_FollowBall(PyCallArgs &call) {
	Call_FollowBall args;
	if(!args.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
		return NULL;
	}

	double distance;
	if( args.distance->IsInt() )
		distance = args.distance->AsInt()->value();
    else if( args.distance->IsFloat() )
		distance = args.distance->AsFloat()->value();
    else
    {
		codelog(CLIENT__ERROR, "%s: Invalid type %s for distance argument received.", call.client->GetName(), args.distance->TypeString());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	
	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return NULL;
	}
	SystemEntity *entity = system->get(args.ballID);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.ballID);
		return NULL;
	}
	
	destiny->Follow(entity, distance);

	return NULL;
}

PyResult BeyonceBound::Handle_SetSpeedFraction(PyCallArgs &call) {
	Call_SingleRealArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}
		
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	
	destiny->SetSpeedFraction(arg.arg);

	return NULL;
}

/* AlignTo
 * This will look up the entityID to get it's position in space, then call
 * AlignTo to have it respond with gotopoint.
 * @author Xanarox
*/
PyResult BeyonceBound::Handle_AlignTo(PyCallArgs &call) {
	CallAlignTo arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return NULL;
	}

	SystemEntity *entity = system->get(arg.entityID);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
		return NULL;
	}

	const GPoint &position = entity->GetPosition();
	destiny->AlignTo( position );

	return NULL;
}

PyResult BeyonceBound::Handle_GotoDirection(PyCallArgs &call) {
	Call_PointArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	destiny->GotoDirection( GPoint( arg.x, arg.y, arg.z ) );

	return NULL;
}

PyResult BeyonceBound::Handle_GotoBookmark(PyCallArgs &call) {

	if( !(call.tuple->GetItem( 0 )->IsInt()) )
    {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: Invalid type %s for bookmarkID received.", call.client->GetName(), call.tuple->GetItem( 0 )->TypeString() );
		return NULL;
	}
    uint32 bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value();

	DestinyManager *destiny = call.client->Destiny();
	if( destiny == NULL )
    {
		sLog.Error( "%s: Client has no destiny manager!", call.client->GetName() );
		return NULL;
	}

    double x,y,z;
    uint32 itemID;
    uint32 typeID;
    GPoint bookmarkPosition;

    BookmarkService *bkSrvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));
        
    if( bkSrvc == NULL )
    {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "Attempt to access BookmarkService via (BookmarkService *)(call.client->services().LookupService(\"bookmark\")) returned NULL pointer." );
		return NULL;
    }
    else
    {
        bkSrvc->LookupBookmark( call.client->GetCharacterID(),bookmarkID,itemID,typeID,x,y,z );

        if( typeID == 5 )
        {
            // Bookmark type is coordinate, so use these directly from the bookmark system call:
            bookmarkPosition.x = x;     // From bookmark x
            bookmarkPosition.y = y;     // From bookmark y
            bookmarkPosition.z = z;     // From bookmark z
            
            destiny->GotoDirection( bookmarkPosition );
        }
        else
        {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
	        SystemManager *sm = call.client->System();
	        if(sm == NULL) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: no system manager found", call.client->GetName() );
		        return NULL;
	        }
            SystemEntity *se = sm->get( itemID );
	        if(se ==  NULL) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: unable to find location %d", call.client->GetName(), itemID );
		        return NULL;
	        }

            destiny->GotoDirection( se->GetPosition() );
        }
    }

    return NULL;
}

PyResult BeyonceBound::Handle_Orbit(PyCallArgs &call) {
	Call_Orbit arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	double distance;
	if( arg.distance->IsInt() )
		distance = arg.distance->AsInt()->value();
    else if( arg.distance->IsFloat() )
		distance = arg.distance->AsFloat()->value();
    else
    {
		codelog(CLIENT__ERROR, "%s: Invalid type %s for distance argument received.", call.client->GetName(), arg.distance->TypeString());
		return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

	SystemManager *system = call.client->System();
	if(system == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
		return NULL;
	}
	SystemEntity *entity = system->get(arg.entityID);
	if(entity == NULL) {
		_log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), arg.entityID);
		return NULL;
	}

	destiny->Orbit(entity, distance);
	return NULL;
}

PyResult BeyonceBound::Handle_WarpToStuff(PyCallArgs &call) {
	CallWarpToStuff arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

    if( arg.type == "item" )
    {
        // This section handles Warping to any object in the Overview
	    double distance;
	    std::map<std::string, PyRep *>::const_iterator res = call.byname.find("minRange");
	    if(res == call.byname.end()) {
		    //Not needed, this is the correct behavior
		    //codelog(CLIENT__ERROR, "%s: range not found, using 15 km.", call.client->GetName());
		    distance = 0.0;
	    } else if(!res->second->IsInt() && !res->second->IsFloat()) {
		    codelog(CLIENT__ERROR, "%s: range of invalid type %s, expected Integer or Real; using 15 km.", call.client->GetName(), res->second->TypeString());
		    distance = 0.0;
	    } else {
		    distance =
			    res->second->IsInt()
			    ? res->second->AsInt()->value()
			    : res->second->AsFloat()->value();
	    }

	    //we need to delay the destiny updates until after we return

	    SystemManager *sm = call.client->System();
	    if(sm == NULL) {
		    codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
		    return NULL;
	    }
        SystemEntity *se = sm->get(arg.ID);
	    if(se ==  NULL) {
		    codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), arg.ID);
		    return NULL;
	    }

		GPoint origin(0.0,0.0,0.0);
		double distanceFromBodyOrigin = 0.0;
		double distanceFromSystemOrigin = 0.0;
		GPoint warpToPoint(se->GetPosition());								// Make a warp-in point variable
        if( IsStaticMapItem(se->GetID()) )
		{
            switch( ((SimpleSystemEntity *)(se))->data.groupID )
			{
				case EVEDB::invGroups::Sun:
				case EVEDB::invGroups::Planet:
				case EVEDB::invGroups::Moon:
                {
					// Calculate final distance out from origin of celestial body along common warp-to vector:
					distanceFromBodyOrigin = se->GetRadius();	        // Add celestial body's radius
					distanceFromBodyOrigin += 20000000;					// Add 20,000km along common vector from celestial body origin to ensure
																		// client camera rotation about ship does not take camera inside the celestial body's wireframe

					// Calculate final warp-to point along common vector from celestial body's origin and add randomized position adjustment for multiple ships coming out of warp to not bump
					GPoint celestialOrigin(se->GetPosition());							// Make a celestial body origin point variable
                    GVector vectorFromOrigin(celestialOrigin, origin);					// Make a celestial body TO system origin origin vector variable
                    if( vectorFromOrigin.length() == 0 )
                    {
                        // This is the special case where we are warping to the Star, so we have to construct
                        // a vector from the star's center (0,0,0) to the warp-in point using the distanceFromBodyOrigin
                        // calculated earlier:
                        vectorFromOrigin = GVector( celestialOrigin, call.client->GetPosition() );
                        vectorFromOrigin.normalize();
                        vectorFromOrigin *= distanceFromBodyOrigin;
                    }
                    GVector vectorToWarpPoint(vectorFromOrigin);                        // Make a vector to the Warp-In point
					distanceFromSystemOrigin = vectorFromOrigin.length();		        // Calculate distance from system origin to celestial body origin

					// Calculate warp-in point to provide different juxtapositioning of celestial body to the solar system origin, i.e, the sun
					// This also provides a common warp-in point for the sun itself, which is the first case in this if-else if-else clause:
					if( distanceFromSystemOrigin < (5.0 * ONE_AU_IN_METERS) )
					{
						// For all celestial bodies with orbit radius of under 5AU, including the sun,
                        GVector rotationVector( 1.0, 1.0, 0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
					}
					else if( distanceFromSystemOrigin < (15.0 * ONE_AU_IN_METERS) )
					{
						// For all celestial bodies with orbit radius of under 15AU but more than 5AU,
                        GVector rotationVector( -1.0, -1.0, 0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
					}
					else if( distanceFromSystemOrigin < (25.0 * ONE_AU_IN_METERS) )
					{
						// For all celestial bodies with orbit radius of under 25AU but more than 15AU,
                        GVector rotationVector( 1.0, -1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
					}
					else if( distanceFromSystemOrigin < (35.0 * ONE_AU_IN_METERS) )
					{
						// For all celestial bodies with orbit radius of under 35AU but more than 25AU,
                        GVector rotationVector( -1.0, -1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
					}
					else
					{
						// For all celestial bodies with orbit radius of more than 35AU,
                        GVector rotationVector( -1.0, 1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
					}

					// Randomize warp-in point:
					warpToPoint.MakeRandomPointOnSphereLayer(1000.0,(1000.0+call.client->GetRadius()));
					break;
                }
				default:
                    // For all other objects, simply just add radius of ship and object:
                    distance += call.client->GetRadius() + se->GetRadius();
					break;
			}
		}
        else
            distance += call.client->GetRadius() + se->GetRadius();

	    call.client->WarpTo( warpToPoint, distance );
    }
    else if( arg.type == "bookmark" )
    {
        // This section handles Warping to any Bookmark:
        double distance = 0.0;
        double x,y,z;
        uint32 itemID;
        uint32 typeID;
        GPoint bookmarkPosition;
        
        BookmarkService *bkSrvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));
        
        if( bkSrvc == NULL )
        {
            sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Attempt to access BookmarkService via (BookmarkService *)(call.client->services().LookupService(\"bookmark\")) returned NULL pointer." );
		    return NULL;
        }
        else
        {
            bkSrvc->LookupBookmark( call.client->GetCharacterID(),arg.ID,itemID,typeID,x,y,z );

            // Calculate the warp-to distance specified by the client and add this to the final warp-to distance
            std::map<std::string, PyRep *>::const_iterator res = call.byname.find("minRange");
		    distance +=
		        res->second->IsInt()
		        ? res->second->AsInt()->value()
		        : res->second->AsFloat()->value();

            if( typeID == 5 )
            {
                // Bookmark type is coordinate, so use these directly from the bookmark system call:
                bookmarkPosition.x = x;     // From bookmark x
                bookmarkPosition.y = y;     // From bookmark y
                bookmarkPosition.z = z;     // From bookmark z

                call.client->WarpTo( bookmarkPosition, distance );
            }
            else
            {
	            DBQueryResult result;
   	            DBResultRow row;
                uint32 groupID = 0;

                // Query database 'invTypes' table for the supplied typeID and retrieve the groupID for this type:
	            if (!sDatabase.RunQuery(result, 
		            " SELECT "
		            "	groupID "
		            " FROM invTypes "
		            " WHERE typeID = %u ", typeID))
	            {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Error in query: %s", result.error.c_str() );
		            return NULL;
	            }

                // Query went through, but check to see if there were zero rows, ie typeID was invalid,
                // and if not, then get the groupID from the row:
	            if ( !(result.GetRow(row)) )
                {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Invalid typeID: %u, no rows returned in db query.", typeID );
                    return NULL;
                }
                groupID = row.GetUInt( 0 );

                // Calculate distance from target warpable object that the ship will warp to, using minimum safe distance
                // based upon groupID of the target object:
                switch( groupID )
                {
                    case 6: // target object is a SUN
                    case 7: // target object is a PLANET
                    case 8: // target object is a MOON
                        //distance += 200000;
                        break;
                    default:
                        break;
                }

                // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
	            SystemManager *sm = call.client->System();
	            if(sm == NULL) {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "%s: no system manager found", call.client->GetName() );
		            return NULL;
	            }
                SystemEntity *se = sm->get( itemID );
	            if(se ==  NULL) {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "%s: unable to find location %d", call.client->GetName(), itemID );
		            return NULL;
	            }

                // Add radiuses for ship and destination object:
                distance += call.client->GetRadius() + se->GetRadius();

                call.client->WarpTo( se->GetPosition(), distance );
            }
        }
    }
    else
    {
        sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Unexpected arg.type value: '%s'.", arg.type.c_str() );
		return NULL;
    }
	
	return NULL;
}

PyResult BeyonceBound::Handle_WarpToStuffAutopilot(PyCallArgs &call) {
	CallWarpToStuffAutopilot arg;
	
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}
	//Change this to change the default autopilot distance (Faster Autopilot FTW)
	double distance = 15000.0;

	//Don't update destiny until done with warp
	SystemManager *sm = call.client->System();
	if(sm == NULL) {
		codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
		return NULL;
	}
	SystemEntity *se = sm->get(arg.item);
	if(se ==  NULL) {
		codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), arg.item);
		return NULL;
	}
	//Adding in object radius
	distance += call.client->GetRadius() + se->GetRadius();
	call.client->WarpTo(se->GetPosition(), distance);

	return NULL;
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
	codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest! that means we messed up pretty bad.", call.client->GetName());

	//no arguments.
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	
	destiny->SendSetState(call.client->Bubble());
	
	return NULL;
}

PyResult BeyonceBound::Handle_Stop(PyCallArgs &call) {
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

    // Only disallow Stopping ship when in warp state AND ship speed is greater than 0.75 times ship's maxVelocity
    if( (destiny->GetState() == Destiny::DSTBALL_WARP)
        && (destiny->GetVelocity().length() >= (0.75*call.client->GetShip()->GetAttribute(AttrMaxVelocity).get_float())) ) {
		    call.client->SendNotifyMsg( "You can't do this while warping");
		    return NULL;
	}
		

	destiny->Stop();

	return NULL;
}

PyResult BeyonceBound::Handle_Dock(PyCallArgs &call) {
	Call_SingleIntegerArg arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}
	
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}
	SystemManager *sm = call.client->System();
	if(sm == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no system manager.", call.client->GetName());
		return NULL;
	}

    // Attempt to Dock:
    call.client->SetDockStationID( arg.arg );   // Set client to know what station it's trying to dock into just in case docking is delayed
    return destiny->AttemptDockOperation();
}

PyResult BeyonceBound::Handle_StargateJump(PyCallArgs &call) {
	Call_TwoIntegerArgs arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}

	call.client->StargateJump(arg.arg1, arg.arg2);
	
	return NULL;
}

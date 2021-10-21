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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "planet/PlanetDB.h"
#include "system/SystemDB.h"
#include "planet/Moon.h"
#include "pos/Structure.h"
#include "ship/BeyonceService.h"
#include "station/StationDataMgr.h"
#include "system/BookmarkService.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"

class BeyonceBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(BeyonceBound)

    BeyonceBound(PyServiceMgr* mgr, Client* pClient)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "BeyonceBound";

        PyCallable_REG_CALL(BeyonceBound, CmdFollowBall);   //*
        PyCallable_REG_CALL(BeyonceBound, CmdOrbit);   //*
        PyCallable_REG_CALL(BeyonceBound, CmdAlignTo);
        PyCallable_REG_CALL(BeyonceBound, CmdGotoDirection);   //*
        PyCallable_REG_CALL(BeyonceBound, CmdGotoBookmark);
        PyCallable_REG_CALL(BeyonceBound, CmdSetSpeedFraction);
        PyCallable_REG_CALL(BeyonceBound, CmdStop);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuff);
        PyCallable_REG_CALL(BeyonceBound, CmdDock);
        PyCallable_REG_CALL(BeyonceBound, CmdStargateJump);
        PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuffAutopilot); //*
        PyCallable_REG_CALL(BeyonceBound, CmdAbandonLoot);

        PyCallable_REG_CALL(BeyonceBound, CmdFleetRegroup);
        PyCallable_REG_CALL(BeyonceBound, CmdFleetTagTarget);
        PyCallable_REG_CALL(BeyonceBound, CmdBeaconJumpFleet);
        PyCallable_REG_CALL(BeyonceBound, CmdBeaconJumpAlliance);
        PyCallable_REG_CALL(BeyonceBound, CmdJumpThroughFleet);
        PyCallable_REG_CALL(BeyonceBound, CmdJumpThroughAlliance);
        PyCallable_REG_CALL(BeyonceBound, CmdJumpThroughCorporationStructure);


        pClient->SetBeyonce(true);
        if (pClient->IsLogin() and !pClient->IsSetStateSent())
            pClient->SetBallPark();
    }

    virtual ~BeyonceBound()
    {
        delete m_dispatch;
    }

    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(CmdFollowBall);
    PyCallable_DECL_CALL(CmdOrbit);
    PyCallable_DECL_CALL(CmdAlignTo);
    PyCallable_DECL_CALL(CmdGotoDirection);
    PyCallable_DECL_CALL(CmdGotoBookmark);
    PyCallable_DECL_CALL(CmdSetSpeedFraction);
    PyCallable_DECL_CALL(CmdStop);
    PyCallable_DECL_CALL(CmdWarpToStuff);
    PyCallable_DECL_CALL(CmdDock);
    PyCallable_DECL_CALL(CmdStargateJump);
    PyCallable_DECL_CALL(UpdateStateRequest);
    PyCallable_DECL_CALL(CmdWarpToStuffAutopilot);
    PyCallable_DECL_CALL(CmdAbandonLoot);

    PyCallable_DECL_CALL(CmdFleetRegroup);
    PyCallable_DECL_CALL(CmdFleetTagTarget);
    PyCallable_DECL_CALL(CmdJumpThroughFleet);
    PyCallable_DECL_CALL(CmdBeaconJumpFleet);
    PyCallable_DECL_CALL(CmdBeaconJumpAlliance);
    PyCallable_DECL_CALL(CmdJumpThroughAlliance);
    PyCallable_DECL_CALL(CmdJumpThroughCorporationStructure);

protected:
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(BeyonceService)

BeyonceService::BeyonceService(PyServiceMgr *mgr)
: PyService(mgr, "beyonce"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
    delete m_dispatch;
}

PyBoundObject* BeyonceService::CreateBoundObject(Client* pClient, const PyRep* bind_args)
{
    return new BeyonceBound(m_manager, pClient);
}

PyResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
    // this is only called when player enters new system and calls to bind new beyonce
    if (!call.client->IsSetStateSent())
        call.client->CheckBallparkTimer();

    PyTuple* res = new PyTuple( 2 );
        Beyonce_Formation f;
            //Diamond formation
            f.name = "Diamond";
            f.pos1.x = 100;
            f.pos2.y = 100;
            f.pos3.x = -100;
            f.pos4.y = -100;
        res->SetItem( 0, f.Encode() );
            //Arrow formation
            f.name = "Arrow";
            f.pos1.x = 100;
            f.pos1.z = -50;
            f.pos2.x = 50;
            f.pos2.y = 0;
            f.pos3.x = -100;
            f.pos3.z = -50;
            f.pos4.x = -50;
            f.pos4.y = 0;
        res->SetItem( 1, f.Encode() );
    return res;
}

PyResult BeyonceBound::Handle_CmdFollowBall(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Follow. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }
    Call_FollowBall args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SystemEntity* pSE = pSystem->GetSE(args.ballID);
    if (pSE == nullptr) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Follow/Approach.", call.client->GetName(), args.ballID);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    pDestiny->Follow(pSE, PyRep::IntegerValue(args.distance));

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdSetSpeedFraction(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    Call_SingleRealArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }
    /** @todo  rework this...this is to set speed ONLY...NOT to begin moving.  */
    // client should not legally send anything < 0.1 (except on rare occasion a 0.0 instead of Stop.)
    if ((arg.arg != 0) && (arg.arg < 0.1))
        return PyStatic.NewNone();

    //sLog.Warning( "BeyonceBound", "Handle_CmdSetSpeedFraction %.2f", arg.arg );
    if (!call.client->IsUndock()){
        if (pDestiny->IsMoving()) {
            pDestiny->SetSpeedFraction(arg.arg);
        } else {
            pDestiny->SetSpeedFraction(arg.arg, true);
        }
    }

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdAlignTo(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Align. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    CallAlignTo arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SystemEntity* pEntity = pSystem->GetSE(arg.entityID);
    if (pEntity == nullptr) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    pDestiny->AlignTo( pEntity );

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdGotoDirection(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called GotoDirection. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    //call.client->SetAutoPilot(false);

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    Call_PointArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    const GPoint dir = GPoint(arg.x, arg.y, arg.z);
    pDestiny->GotoDirection(dir);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdGotoBookmark(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called GotoBookmark. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    //call.client->SetAutoPilot(false);

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    double x(0.0), y(0.0), z(0.0);
    uint16 typeID(0);
    uint32 itemID(0), locationID(0);

    BookmarkService* pBMSvc = (BookmarkService*)(call.client->services().LookupService( "bookmark" ));

    if (pBMSvc == nullptr) {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "Attempt to access BookmarkService via (BookmarkService*)(call.client->services().LookupService(\"bookmark\")) returned NULL." );
        return PyStatic.NewNone();
    } else {
        pBMSvc->LookupBookmark(arg.arg, itemID, typeID, locationID, x, y, z);

        if (typeID == 5) {
            if (call.client->GetSystemID() != locationID) {
                //  this bm is for different system.  make and send error here.
                return PyStatic.NewNone();
            }

            GPoint point(x, y, z);
            pDestiny->GotoPoint(point);
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            SystemEntity* pSE = call.client->SystemMgr()->GetSE(itemID);
            if (pSE == nullptr) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: unable to find location %d", call.client->GetName(), itemID );
                return PyStatic.NewNone();
            }

            pDestiny->GotoPoint( pSE->GetPosition() );
        }
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdOrbit(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Orbit. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    call.client->SetAutoPilot(false);

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    call.Dump(SERVICE__CALL_DUMP);
    Call_Orbit args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    double range =
        args.range->IsInt()
        ? args.range->AsInt()->value()
        : args.range->AsFloat()->value();

    SystemEntity* pEntity = pSystem->GetSE(args.entityID);
    if (pEntity == nullptr) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.entityID);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    pDestiny->Orbit(pEntity, range);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdWarpToStuff(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called WarpToStuff. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    //call.client->SetAutoPilot(false);

  _log(SERVICE__CALL_DUMP, "BeyonceBound::Handle_CmdWarpToStuff() - size=%li", call.tuple->size());
   call.Dump(SERVICE__CALL_DUMP);

   /** @todo (allan) finish warp scramble system */
   // >0 means ship cannot warp (warp stabs are neg values, warp scrams are pos values)
   if (call.client->GetShip()->GetAttribute(AttrWarpScrambleStatus) > 0)
        throw UserError ("WarpScrambled");

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    }
    if (pDestiny->IsWarping()){
        call.client->SendNotifyMsg( "You are already warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    bool fleet(false);
    if (call.byname.find("fleet") != call.byname.end())
        if (!(call.byname.find("fleet")->second->IsNone()))
            fleet = call.byname.find("fleet")->second->AsBool()->value();

    // get the warp-to distance specified by the client
    int32 distance(0); //Was set to 5000; setting to 0 makes warp in to station and gate more realistic.
    if (call.byname.find("minRange") != call.byname.end())
        distance = PyRep::IntegerValueU32(call.byname.find("minRange")->second);

    GPoint warpToPoint(NULL_ORIGIN);
    SystemEntity* pSE(nullptr);
    double radius(0);
    uint32 toID(0);
    std::string stringArg = "";

    if ((call.tuple->GetItem(1)->IsString())
    or  (call.tuple->GetItem(1)->IsWString())) {
        stringArg = PyRep::StringContent(call.tuple->GetItem(1));
    } else {
        toID = PyRep::IntegerValueU32(call.tuple->GetItem(1));
    }

    std::string type = PyRep::StringContent(call.tuple->GetItem(0));
    if (type == "item" ) {
        pSE = pSystem->GetSE(toID);
        if (pSE == nullptr) {
            codelog(CLIENT__ERROR, "%s: unable to find item location %u in %s(%u)", call.client->GetName(), toID, pSystem->GetName(), pSystem->GetID());
            return PyStatic.NewNone();
        }
    } else if (type == "bookmark" ) {
        double x(0.0), y(0.0), z(0.0);
        uint16 typeID(0);
        uint32 locationID(0);
        uint32 bookmarkID(PyRep::IntegerValueU32(call.tuple->GetItem(1)));

        BookmarkService* bkSrvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));
        if (bkSrvc == nullptr) {
            sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Attempt to access BookmarkService returned NULL." );
            return PyStatic.NewNone();
        }
        bkSrvc->LookupBookmark(bookmarkID, toID, typeID, locationID, x, y, z);

        if ( typeID == 5 ) {
            if (call.client->GetSystemID() != locationID) {
                //  this bm is for different system.  make error here.
                return PyStatic.NewNone();
            }
            warpToPoint.x = x;
            warpToPoint.y = y;
            warpToPoint.z = z;
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            pSE = pSystem->GetSE( toID );
            if (pSE == nullptr) {
                codelog(CLIENT__ERROR, "%s: unable to find bookmark location %u in %s(%u)", call.client->GetName(), toID, pSystem->GetName(), pSystem->GetID());
                return PyStatic.NewNone();
            }
        }
    } else if (type == "scan") {
        uint32 anomID = pSystem->GetAnomMgr()->GetAnomalyID(stringArg);
        pSE = pSystem->GetSE(anomID);
    } else if (type == "launch") {
        //warpToPoint = PlanetDB::GetLaunchPos(toID);
        pSE = pSystem->GetSE(toID);
        // launchpickup - launch, launchid
    }
    // the systems below are not implemented yet.  hold on coding till systems are working and we know what needs to be done here
    // more info can be found in client::menuSvc.py
    else if (type == "epinstance") {
        // epinstance, instanceid
        //stringArg
        call.client->SendErrorMsg("WarpToInstance is not implemented at this time.");
        return PyStatic.NewNone();
    } else if (type == "tutorial") {
        // tutorial, none
        call.client->SendErrorMsg("WarpToTutorial is not implemented at this time.");
        return PyStatic.NewNone();
    } else if (type == "char") {
    //  fleet warping
    // [warptomember] char, charid, minrange
    // [warpfleettomember] char, charid, minrange, fleet=1
        call.client->SendErrorMsg("WarpToChar is not implemented at this time.");
        return PyStatic.NewNone();
    } else {
        sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Unexpected type value: '%s'.", type.c_str() );
        return PyStatic.NewNone();
    }

        /* formulas for warpin points for all objects
         * x,y,z = object coords.  r = object radius
         *
         * for objects <90kr
         * dest = x,y,z + (vector - r)
         *
         * for objects >90kr
         * (x + (r + 5000000) * cos(r), y + 1.3r -7500, z - (r + 5000000) * sin(r))
         *
         * for planets, its a bit different
         * (x + d*sin(t), y + 0.5*r*sin(j), z - d*cos(t))
         * where:
         * j = rand(seed=planetID).rand(0,1) - 1.0/3.0
         * d = r*(s+1) +1000000
         * t = sin^-1(x/abs(x) * z / sqrt(x^2 + z^2) +j)
         * s = 20*((1/40)*(10*log10(r/10^6)-39)^20) +0.5
         * s = max(0.5, min(s, 10.5))
         */
        /** @todo  that may not be right....
         * this is straight from client
         *
         * def GetPlanetWarpInPoint(planetID, locVec, r):
         *    dx = float(locVec[0])
         *    dz = float(locVec[2])
         *    f = float(dz) / float(math.sqrt(dx ** 2 + dz ** 2))
         *    if dz > 0 and dx > 0 or dz < 0 and dx > 0:
         *        f *= -1.0
         *    theta = math.asin(f)
         *    myRandom = random.Random(planetID)
         *    rr = (myRandom.random() - 1.0) / 3.0
         *    theta += rr
         *    offset = 1000000
         *    FACTOR = 20.0
         *    dd = math.pow((FACTOR - 5.0 * math.log10(r / 1000000) - 0.5) / FACTOR, FACTOR) * FACTOR
         *    dd = min(10.0, max(0.0, dd))
         *    dd += 0.5
         *    offset += r * dd
         *    d = r + offset
         *    x = 1000000
         *    z = 0
         *    x = math.sin(theta) * d
         *    z = math.cos(theta) * d
         *    y = r * math.sin(rr) * 0.5
         *    return util.KeyVal(x=x, y=y, z=z)
         */
    if (pSE != nullptr) {
        radius = pSE->GetRadius();
        // this will need adjustment for warping to bookmarks
        warpToPoint = pSE->GetPosition();
        if (pSE->IsPlanetSE()) {
            srandom(toID);  //this is the only place random() is used....other random functions use rand() as it's non-repeatable.
            int rand = random();
            double j = (((rand / RAND_MAX) - 1.0f) / 3.0f);
            double s = 20 * std::pow(0.025f * (10 * std::log10(radius / 1000000) - 39), 20) + 0.5f;
            s = EvE::max(0.5f, EvE::min(s, 10.5f));
            double t = std::asin((warpToPoint.x / std::fabs(warpToPoint.x)) * (warpToPoint.z / std::sqrt(std::pow(warpToPoint.x, 2) + std::pow(warpToPoint.z, 2)))) + j;
            uint32 d = radius * (s + 1) + 1000000;
            warpToPoint.x += (d * std::sin(t));
            warpToPoint.y += (0.5f * radius * std::sin(j));
            warpToPoint.z -= (d * std::cos(t));
        } else if (pSE->IsStationSE()) {
            // this makes ship warp to station dock elevation (y), instead of warping to stations "center point" position (where icon is)
            warpToPoint.y = stDataMgr.GetDockPosY(pSE->GetID());
        } else if (pSE->IsCOSE()) {
            distance += (radius / 2);
        } else if (pSE->IsGateSE()) {
            distance += (radius / 3);  // fudge the distance a bit for gates... its' a lil close by default
        } else if (pSE->IsMoonSE()) {
            if (pSE->GetMoonSE()->HasTower()) {
                // if moon has a tower, make warpin point 20km inside edge of tower's bubble.
                warpToPoint = pSE->GetMoonSE()->GetMyTower()->GetPosition();
                GVector vectorFromOrigin(call.client->GetShipSE()->GetPosition(), warpToPoint);
                vectorFromOrigin.normalize();   //we now have a direction
                GPoint stopPoint = (vectorFromOrigin * (BUBBLE_RADIUS_METERS - 20000));  // 20km inside bubble.
                warpToPoint -= stopPoint;
                distance = 0;
            } else {
                // hack for warping to moons
                // this puts ship at Az: 0.785332, Ele: 0.615505, angle: 1.5708
                warpToPoint -= (radius * 1.25f);
            }
        } else if (pSE->IsWormholeSE()) {
                distance += 20000;  // add 20k for wh
        } else if (radius > 90000) {
            // this doesnt work for moons
            warpToPoint.x += ((radius + 500000) * std::cos(radius));
            warpToPoint.y += ((radius * 1.3f) - 7500);
            warpToPoint.z -= ((radius + 500000) * std::sin(radius));
        }
        if (radius < 90000) {
            // this will include stations (max station radius 60km)
            GVector vectorFromOrigin(call.client->GetShipSE()->GetPosition(), warpToPoint);
            vectorFromOrigin.normalize();   //we now have a direction
            GPoint stopPoint = (vectorFromOrigin * radius);
            warpToPoint -= stopPoint;
        }
    }
    if (warpToPoint.isZero()) {
        // point is zero ....make error and return
        codelog(CLIENT__ERROR, "%s: warpToPoint.isZero() = true.  Cannot find location %u for '%s'", call.client->GetName(), toID, type.c_str());
        call.client->SendErrorMsg("WarpTo: Item location not found.");
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    distance += (call.client->GetShipSE()->GetRadius() * 2); // add ship diameter to distance
    pDestiny->WarpTo(warpToPoint, distance);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdWarpToStuffAutopilot(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called WarpToStuffAutopilot. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

  //  sends targeted celestial itemID as arg.destID
    CallWarpToStuffAutopilot arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
	}

    SystemEntity* pSE = pSystem->GetSE(arg.destID);
    if (pSE == nullptr) {
	  codelog(CLIENT__ERROR, "%s: unable to find destination Entity for ID %u", call.client->GetName(), arg.destID);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);
    // AP shit here.....
    call.client->SetAutoPilot(true);
    call.client->UpdateSessionInt("solarsystemid", pSystem->GetID());
    //call.client->UpdateSession();
    //call.client->SendSessionChange();

    uint16 distance = sConfig.world.apWarptoDistance;    //10km default
    //Adding in ship and target object radius'
    //distance += call.client->GetShipSE()->GetRadius() + pSE->GetRadius();
    pDestiny->WarpTo(pSE->GetPosition(), distance, true, pSE);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdStop(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Stop. AP: %s, Invul: %s", call.client->GetName(), \
            (call.client->IsAutoPilot() ? "true" : "false"), call.client->IsInvul()?"true":"false");

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    }
    if (!pDestiny->IsMoving())
        return PyStatic.NewNone();
    if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    call.client->SetUndock(false);
    call.client->SetAutoPilot(false);

    pDestiny->Stop();

    return PyStatic.NewNone();
}

// CmdTurboDock (in client code)
PyResult BeyonceBound::Handle_CmdDock(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Dock. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager.", call.client->GetName());
        return PyStatic.NewNone();
    }
    Call_TwoIntegerArgs args;  //sends stationID, shipID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    //  this sets m_dockStationID for radius checks and other things
    call.client->SetDockStationID( args.arg1 );

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return pDestiny->AttemptDockOperation();
}

PyResult BeyonceBound::Handle_CmdStargateJump(PyCallArgs &call) {
    /*  jump system messages....
(67187, `{[location]system.name} Traffic Control: your jump-in clearance has expired.`)
(67191, `{[location]system.name} Traffic Control: you have been cleared for jump-in within {[timeinterval]expiration.writtenForm, from=second, to=second}.`)
(67210, `{[location]system.name} Traffic Control: you are at position {[numeric]pos} in queue for jump-in.`)
(258673, `{system} is currently loading. Bizzarre, seeing that you're there already... Please try again in a minute or two.`)
(258674, `The stargates in {system} are currently experiencing minor technical difficulties. Please try again in a moment.`)
(258675, `Jump Prohibited`)
(258676, `Officials have closed the stargates in {system} due to heavy congestion. Travelers are advised to enjoy the local scenery.`)
(258677, `{system} Traffic Control is currently experiencing heavy load and is unable to process your request. Please try again in a moment.`)
(258678, `{system} Traffic Control is currently offline and unable to process your jump request. Please try again in a moment.`)
(258679, `{system} Traffic Control is currently experiencing heavy load and is unable to process your request. You are #{[numeric]position} in queue for jump-in.`)
(258680, `Jump Prohibited`)
(258681, `Officials have closed the stargates in {system} due to heavy congestion. Travelers are advised to seek alternate routes.`)
(258683, `Your character is located within {system}, which is currently loading. Please try again in a moment.`)
(258685, `Your character is located within {system}, which is currently experiencing heavy load. Please try again in a moment.`)
(258687, `Your character is located within {system}, which has reached maximum capacity. You are #{[numeric]position} in queue for entrance. Please try again in a moment.`)
(258689, `Your character is located within {system}, which is currently stuck. Please select a different character or try later.`)
(258691, `The character is located within {system}, which is currently loading. Please try again in a moment.`)
(258693, `The character is located within {system}, which is currently experiencing heavy load. Please try again in a moment.`)
(258695, `The character is located within {system}, which is currently stuck. Please try again later.`)
(258699, `The item <b>{example}</b> was not found in the station <b>{[location]station.name}</b>. Please make sure all items are in the correct hangar before finalizing this contract.`)
(258701, `{system} Traffic Control is currently experiencing heavy load and was unable to process your request. Please try again in a moment.`)
(258704, `You cannot leave {system} yet because of instability in the space-time continuum. Please try again in a moment.`)
*/

    _log(AUTOPILOT__MESSAGE, "%s called Jump. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    Call_StargateJump args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    /** @todo  check distance from ship to gate */
    call.client->StargateJump(args.fromStargateID, args.toStargateID);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult BeyonceBound::Handle_CmdAbandonLoot(PyCallArgs &call) {
	/*  remotePark.CmdAbandonLoot(wrecks)  <- this is pylist from 'abandonAllWrecks'
	 *  remotePark.CmdAbandonLoot([wreckID]) <- single itemID in list
	 */
  sLog.White( "BeyonceBound::Handle_CmdAbandonLoot()", "size=%li", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

	Call_SingleIntList arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
		return PyStatic.NewNone();
	}

	/** @todo  change ownerID to _system for all loot also!!  */
	SystemEntity* pSE(nullptr);
	SystemManager* pSysMgr = call.client->SystemMgr();
	for (auto cur : arg.ints) {
        pSE = pSysMgr->GetSE(cur);
        if (pSE == nullptr)
            continue;
        pSE->Abandon();
        PyTuple* slimData = new PyTuple(2);
            slimData->SetItem(0, new PyLong(pSE->GetID()));
            slimData->SetItem(1, new PyObject( "foo.SlimItem", pSE->MakeSlimItem()));
        PyTuple* itemData = new PyTuple(2);
            itemData->SetItem(0, new PyString("OnSlimItemChange"));
            itemData->SetItem(1, slimData);
        pSE->SysBubble()->BubblecastDestinyUpdate(&itemData, "OnSlimItemChange" );
    }

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
    codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest. Previous call generated a bad return.  Check Logs.", call.client->GetName());

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    }
    if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    }

    call.client->SetStateSent(false);
    pDestiny->SendSetState();

    return PyStatic.NewNone();
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */


/** @todo these will need work....
 *    def BridgeToMember(self, charID):
 *        beaconStuff = sm.GetService('fleet').GetActiveBeaconForChar(charID)
 *        if beaconStuff is None:
 *            return
 *        self.BridgeToBeacon(charID, beaconStuff)
 *
 *    def BridgeToBeaconAlliance(self, solarSystemID, beaconID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bp.CmdBridgeToStructure(beaconID, solarSystemID)
 *
 *    def BridgeToBeacon(self, charID, beacon):
 *        solarsystemID, beaconID = beacon
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bp.CmdBridgeToMember(charID, beaconID, solarsystemID)
 *
 *    def JumpThroughFleet(self, otherCharID, otherShipID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bridge = sm.GetService('fleet').GetActiveBridgeForShip(otherShipID)
 *        if bridge is None:
 *            return
 *        solarsystemID, beaconID = bridge
 *        self.LogNotice('Jump Through Fleet', otherCharID, otherShipID, beaconID, solarsystemID)
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughFleet, otherCharID, otherShipID, beaconID, solarsystemID)
 *
 *    def JumpThroughAlliance(self, otherShipID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bridge = sm.StartService('pwn').GetActiveBridgeForShip(otherShipID)
 *        if bridge is None:
 *            return
 *        solarsystemID, beaconID = bridge
 *        self.LogNotice('Jump Through Alliance', otherShipID, beaconID, solarsystemID)
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughAlliance, otherShipID, beaconID, solarsystemID)
 *
 *    def JumpToMember(self, charid):
 *        beaconStuff = sm.GetService('fleet').GetActiveBeaconForChar(charid)
 *        if beaconStuff is None:
 *            return
 *        self.JumpToBeaconFleet(charid, beaconStuff)
 *
 *    def JumpToBeaconFleet(self, charid, beacon):
 *        solarsystemID, beaconID = beacon
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        self.LogNotice('Jump To Beacon Fleet', charid, beaconID, solarsystemID)
 *        for wnd in uicore.registry.GetWindows()[:]:
 *            if getattr(wnd, '__guid__', None) == 'form.CorpHangarArray':
 *                wnd.CloseByUser()
 *
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpFleet, charid, beaconID, solarsystemID)
 *
 *    def JumpToBeaconAlliance(self, solarSystemID, beaconID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        self.LogNotice('Jump To Beacon Alliance', beaconID, solarSystemID)
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpAlliance, beaconID, solarSystemID)
 *
 *    def ActivateGridSmartBomb(self, charid, effect):
 *        beaconStuff = sm.GetService('fleet').GetActiveBeaconForChar(charid)
 *        if beaconStuff is None:
 *            return
 *        solarsystemID, beaconID = beaconStuff
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        effect.Activate(beaconID, False)
 */
PyResult BeyonceBound::Handle_CmdJumpThroughFleet(PyCallArgs &call) {
    // sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughFleet, otherCharID, otherShipID, beaconID, solarsystemID)
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdJumpThroughFleet");
    call.Dump(SHIP__WARNING);
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdJumpThroughAlliance(PyCallArgs &call) {
    //sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughAlliance, otherShipID, beaconID, solarsystemID)
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdJumpThroughAlliance");
    call.Dump(SHIP__WARNING);
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdJumpThroughCorporationStructure(PyCallArgs &call) {
    //sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughCorporationStructure, itemID, remoteStructureID, remoteSystemID)
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdJumpThroughCorporationStructure");
    call.Dump(SHIP__WARNING);

    _log(AUTOPILOT__MESSAGE, "%s called bridge jump.", call.client->GetName());
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    }

    Call_BridgeJumpAlliance args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    InventoryItemRef beacon = sItemFactory.GetInventoryItemFromID(args.remoteStructureID);

    //Check for jump fuel and make sure there is enough fuel available
    InventoryItemRef bridge = sItemFactory.GetInventoryItemFromID(args.itemID);
    ShipItemRef ship = call.client->GetShip();

    std::vector<InventoryItemRef> fuelBayItems;
    std::vector<InventoryItemRef> requiredItems;
    uint32 fuelType = EVEDB::invTypes::LiquidOzone;
    uint32 fuelQuantity = 500; // Bridges use static fuel quantity

    bridge->GetMyInventory()->GetItemsByFlag(flagHangar, fuelBayItems);
    uint32 quantity = 0;
    for (auto cur : fuelBayItems) {
        if (cur->type().id() == fuelType) {
            quantity += cur->quantity();
            requiredItems.push_back(cur);
            if (quantity >= fuelQuantity) {
                break;
            }
        }
    }
    if (quantity < fuelQuantity) {
        ship->GetPilot()->SendNotifyMsg("This jump requires %u units of %s in the Jump Bridge fuel hold.", fuelQuantity, sItemFactory.GetType(fuelType)->name().c_str());
        return nullptr;
    }

    uint32 quantityLeft = fuelQuantity;
    for (auto cur : requiredItems) {
        if (cur->quantity() >= quantityLeft) {
            //If we have all the quantity we need in the current stack, decrement the amount we need and break
            cur->AlterQuantity(-quantityLeft, true);
            break;
        } else {
            //If the stack doesn't have the full amount, decrement the quantity from what we need and zero out the stack
            quantityLeft -= cur->quantity();
            // Delete item after we zero it's quantity
            cur->SetQuantity(0, true, true);
        }
    }
    GPoint position = beacon->position();
    position.MakeRandomPointOnSphere(2500.0);
    call.client->CynoJump(beacon);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult BeyonceBound::Handle_CmdBeaconJumpFleet(PyCallArgs &call) {
    // sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpFleet, charid, beaconID, solarsystemID)
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdBeaconJumpFleet");
    call.Dump(SHIP__WARNING);

    _log(AUTOPILOT__MESSAGE, "%s called beacon jump.", call.client->GetName());
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    } else if (pDestiny->IsFrozen()) {
        call.client->SendNotifyMsg( "Your ship is frozen and cannot move");
        return PyStatic.NewNone();
    }

    Call_BeaconJumpFleet args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    InventoryItemRef beacon = sItemFactory.GetInventoryItemFromID(args.beaconID);

    //Check for jump fuel and make sure there is enough fuel available
    ShipItemRef ship = call.client->GetShip();

    std::vector<InventoryItemRef> fuelBayItems;
    std::vector<InventoryItemRef> requiredItems;
    uint32 fuelType = ship->GetAttribute(AttrJumpDriveConsumptionType).get_uint32();
    uint32 fuelBaseConsumption = uint32(ceil(ship->GetAttribute(AttrJumpDriveConsumptionAmount).get_float()));
    uint32 fuelQuantity;

    GPoint startPosition = SystemDB::GetSolarSystemPosition(call.client->GetSystemID());
    GPoint endPosition = SystemDB::GetSolarSystemPosition(beacon->locationID());

    GVector heading ( startPosition, endPosition );
    double jumpDistance = EvEMath::Units::MetersToLightYears(heading.length());

    int8 jumpFuelConservationLevel = call.client->GetChar()->GetSkillLevel(EvESkill::JumpFuelConservation);
    int8 jumpFreightersLevel = call.client->GetChar()->GetSkillLevel(EvESkill::JumpFreighters);

    if (ship->groupID() == EVEDB::invGroups::JumpFreighter) {
        fuelQuantity = uint32(ceil(jumpDistance * fuelBaseConsumption * (1 - 0.1 * jumpFuelConservationLevel) * (1 - 0.1 * jumpFreightersLevel)));
    } else {
        fuelQuantity = uint32(ceil(jumpDistance * fuelBaseConsumption * (1 - 0.1 * jumpFuelConservationLevel)));
    }

    ship->GetMyInventory()->GetItemsByFlag(flagFuelBay, fuelBayItems);
    uint32 quantity = 0;
    for (auto cur : fuelBayItems) {
        if (cur->type().id() == fuelType) {
            quantity += cur->quantity();
            requiredItems.push_back(cur);
            if (quantity >= fuelQuantity) {
                break;
            }
        }
    }
    if (quantity < fuelQuantity) {
        ship->GetPilot()->SendNotifyMsg("This jump requires you to have %u units of %s in your fuel bay.", fuelQuantity, sItemFactory.GetType(fuelType)->name().c_str());
        return nullptr;
    }

    for (auto cur : requiredItems) {
        if (cur->quantity() > fuelQuantity) {
            //If we have all the quantity we need in the current stack, decrement the amount we need and break
            cur->AlterQuantity(-fuelQuantity, true);
            break;
        } else {
            //If the stack doesn't have the full amount delete item
            cur->SetQuantity(0, true, true);
        }
    }

    call.client->CynoJump(beacon);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdBeaconJumpAlliance(PyCallArgs &call) {
    // sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpAlliance, beaconID, solarSystemID)
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdBeaconJumpAlliance");
    call.Dump(SHIP__WARNING);

    _log(AUTOPILOT__MESSAGE, "%s called beacon jump.", call.client->GetName());
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return PyStatic.NewNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return PyStatic.NewNone();
    }

    Call_BeaconJumpAlliance args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    InventoryItemRef beacon = sItemFactory.GetInventoryItemFromID(args.beaconID);

    //Check for jump fuel and make sure there is enough fuel available
    ShipItemRef ship = call.client->GetShip();

    std::vector<InventoryItemRef> fuelBayItems;
    std::vector<InventoryItemRef> requiredItems;
    uint32 fuelType = ship->GetAttribute(AttrJumpDriveConsumptionType).get_uint32();
    uint32 fuelBaseConsumption = uint32(ceil(ship->GetAttribute(AttrJumpDriveConsumptionAmount).get_float()));
    uint32 fuelQuantity;

    GPoint startPosition = SystemDB::GetSolarSystemPosition(call.client->GetSystemID());
    GPoint endPosition = SystemDB::GetSolarSystemPosition(beacon->locationID());

    GVector heading ( startPosition, endPosition );
    double jumpDistance = EvEMath::Units::MetersToLightYears(heading.length());

    int8 jumpFuelConservationLevel = call.client->GetChar()->GetSkillLevel(EvESkill::JumpFuelConservation);
    int8 jumpFreightersLevel = call.client->GetChar()->GetSkillLevel(EvESkill::JumpFreighters);

    if (ship->groupID() == EVEDB::invGroups::JumpFreighter) {
        fuelQuantity = uint32(ceil(jumpDistance * fuelBaseConsumption * (1 - 0.1 * jumpFuelConservationLevel) * (1 - 0.1 * jumpFreightersLevel)));
    } else {
        fuelQuantity = uint32(ceil(jumpDistance * fuelBaseConsumption * (1 - 0.1 * jumpFuelConservationLevel)));
    }

    ship->GetMyInventory()->GetItemsByFlag(flagFuelBay, fuelBayItems);
    uint32 quantity = 0;
    for (auto cur : fuelBayItems) {
        if (cur->type().id() == fuelType) {
            quantity += cur->quantity();
            requiredItems.push_back(cur);
            if (quantity >= fuelQuantity) {
                break;
            }
        }
    }
    if (quantity < fuelQuantity) {
        ship->GetPilot()->SendNotifyMsg("This jump requires you to have %u units of %s in your inventory.", fuelQuantity, sItemFactory.GetType(fuelType)->name().c_str());
        return nullptr;
    }

    uint32 quantityLeft = fuelQuantity;
    for (auto cur : requiredItems) {
        if (cur->quantity() >= quantityLeft) {
            //If we have all the quantity we need in the current stack, decrement the amount we need and break
            cur->AlterQuantity(-quantityLeft, true);
            break;
        } else {
            //If the stack doesn't have the full amount, decrement the quantity from what we need and zero out the stack
            quantityLeft -= cur->quantity();
            // Delete item after we zero it's quantity
            cur->SetQuantity(0, true, true);
        }
    }

    GPoint position = beacon->position();
    position.MakeRandomPointOnSphere(2500.0);
    call.client->CynoJump(beacon);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdFleetRegroup(PyCallArgs &call) {
    // not sure what this is supposed to do yet
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdFleetRegroup");
    call.Dump(SHIP__WARNING);
    return nullptr;
}

PyResult BeyonceBound::Handle_CmdFleetTagTarget(PyCallArgs &call) {
    // bp.CmdFleetTagTarget(itemID, tag)
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdFleetTagTarget");
    call.Dump(SHIP__WARNING);
    return PyStatic.NewNone();
}

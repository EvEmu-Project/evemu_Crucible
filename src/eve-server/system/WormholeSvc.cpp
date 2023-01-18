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

#include "Client.h"
#include "system/WormholeSvc.h"

WormHoleSvc::WormHoleSvc()
: Service("wormholeMgr", eAccessLevel_Location)
{
    this->Add("WormholeJump", &WormHoleSvc::WormholeJump);
}

/*{'messageKey': 'CantEnterWormholeGlobalCriminalFlag', 'dataID': 17877563, 'suppressable': False, 'bodyID': 257299, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257298, 'messageID': 2844}
 * {'messageKey': 'CantEnterWormholeWhileCloaked', 'dataID': 17877181, 'suppressable': False, 'bodyID': 257153, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2783}
 * {'messageKey': 'CantEnterWormholeWhileModuleActive', 'dataID': 17877772, 'suppressable': False, 'bodyID': 257379, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2784}
 * {'messageKey': 'CantOnlineSovInWormhole', 'dataID': 17877461, 'suppressable': False, 'bodyID': 257260, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2935}
 * {'messageKey': 'CantTargetWhileEnteringWormhole', 'dataID': 17877231, 'suppressable': False, 'bodyID': 257172, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2798}
 */
PyResult WormHoleSvc::WormholeJump(PyCallArgs& call, PyInt* itemID) {
  /**
      sm.RemoteSvc('wormholeMgr').WormholeJump, itemID)
      */
    sLog.White("WormHoleSvc", "Handle_WormholeJump() size=%lli", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    /*
     *    char ci[25];
     *    snprintf(ci, sizeof(ci), "Jumping:%u", toGate);
     *    GetShip()->SetCustomInfo(ci);
     */

    _log(AUTOPILOT__MESSAGE, "%s called wormhole jump.", call.client->GetName());
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
    } else if (pDestiny->IsCloaked()) {
        throw UserError ("CantEnterWormholeWhileCloaked");
    }

    InventoryItemRef wormhole = sItemFactory.GetItemRefFromID(itemID->value());

    //Check for jump fuel and make sure there is enough fuel available
    ShipItemRef ship = call.client->GetShip();

    if (ship->GetAttribute(AttrMass) > wormhole->GetAttribute(AttrWormholeMaxJumpMass).get_int()) {
        call.client->SendNotifyMsg("Your ship is too large to fit through the wormhole.");
        return PyStatic.NewNone();
    }

    call.client->WormholeJump(wormhole);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    return new PyLong(Win32TimeNow());
}

//                  ----------  wormhole data sent thru "AddBalls2" packet
/*
                              [PyString "AddBalls2"]
                              [PyTuple 1 items]
                                [PyTuple 3 items]
                                  [Destiny Header]
                                    [PacketType: 1]
                                    [Stamp: 956]
                                  [Ball]
                                    [Name: ]
                                    [FormationId: 255]
                                    [Header]
                                      [ItemId: 9000000000000023045]
                                      [Mode: Stop (2)]
                                      [Flags: 0 (0)]
                                      [Radius: 3000]
                                      [Location: (344357570320, -691652812800, 102847488000)]
                                    [ExtraHeader]
                                      [AllianceId: -1]
                                      [CorporationId: -1]
                                      [CloakMode: 0]
                                      [Harmonic: NaN]
                                      [Mass: 10000000000]

                                  [PyList 2 items]
                                    [PyDict 9 kvp]
                                      [PyString "itemID"]
                                      [PyIntegerVar 9000000000000023045]
                                      [PyString "typeID"]
                                      [PyInt 30584]
                                      [PyString "otherSolarSystemClass"]
                                      [PyInt 3]
                                      [PyString "wormholeSize"]
                                      [PyFloat 1]
                                      [PyString "wormholeAge"]
                                      [PyInt 1]
                                      [PyString "count"]
                                      [PyInt 1]
                                      [PyString "dunSpawnID"]
                                      [PyInt 92]
                                      [PyString "ownerID"]
                                      [PyInt 500021]
                                      [PyString "nebulaType"]
                                      [PyInt 253]
                                    [PyDict 4 kvp]
                                      [PyString "itemID"]
                                      [PyIntegerVar 9000000000000016526]
                                      [PyString "typeID"]
                                      [PyInt 19728]
                                      [PyString "dungeonDataID"]
                                      [PyInt 2990651]
                                      [PyString "ownerID"]
                                      [PyInt 500021]

            --------- destiny update
              [PyTuple 2 items]
                [PyList 3 items]
                  [PyTuple 2 items]
                    [PyInt 1019]
                    [PyTuple 2 items]
                      [PyString "Stop"]
                      [PyTuple 1 items]
                        [PyIntegerVar 1002332228246]
                  [PyTuple 2 items]
                    [PyInt 1019]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 10 items]
                        [PyIntegerVar 1002332228246]
                        [PyNone]
                        [PyNone]
                        [PyIntegerVar 9000000000000023045]
                        [PyInt 3]
                        [PyList 0 items]
                        [PyString "effects.JumpOutWormhole"]
                        [PyInt 0]
                        [PyInt 1]
                        [PyInt 0]
                  [PyTuple 2 items]
                    [PyInt 1019]
                    [PyTuple 2 items]
                      [PyString "OnSlimItemChange"]
                      [PyTuple 2 items]
                        [PyIntegerVar 9000000000000023045]
                        [PyObjectData Name: foo.SlimItem]
                          [PyDict 9 kvp]
                            [PyString "itemID"]
                            [PyIntegerVar 9000000000000023045]
                            [PyString "typeID"]
                            [PyInt 30584]
                            [PyString "otherSolarSystemClass"]
                            [PyInt 3]
                            [PyString "wormholeSize"]
                            [PyFloat 1]
                            [PyString "wormholeAge"]
                            [PyInt 1]
                            [PyString "count"]
                            [PyInt 2]
                            [PyString "dunSpawnID"]
                            [PyInt 92]
                            [PyString "ownerID"]
                            [PyInt 500021]
                            [PyString "nebulaType"]
                            [PyInt 253]
                [PyBool False]
                                      */


/*   location is both region and system.
 * mapLocationWormholeClasses
 * Column  Type    Null    Default     Comments
 * locationID    int(11) No
 * wormholeClassID     tinyint(3)  Yes     NULL
 */
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

#include "PyServiceCD.h"
#include "dungeon/DungeonService.h"
#include "dungeon/DungeonDB.h"

DungeonService::DungeonService() :
    Service("dungeon")
{
    // objects
    this->Add("IsObjectLocked", &DungeonService::IsObjectLocked);
    this->Add("AddObject", &DungeonService::AddObject);
    this->Add("RemoveObject", &DungeonService::RemoveObject);
    this->Add("CopyObject", &DungeonService::CopyObject);
    this->Add("EditObject", &DungeonService::EditObject);
    this->Add("EditObjectName", &DungeonService::EditObjectName);
    this->Add("EditObjectRadius", &DungeonService::EditObjectRadius);
    this->Add("EditObjectXYZ", &DungeonService::EditObjectXYZ);
    this->Add("EditObjectYawPitchRoll", &DungeonService::EditObjectYawPitchRoll);
    // templates
    this->Add("TemplateAdd", &DungeonService::TemplateAdd);
    this->Add("TemplateRemove", &DungeonService::TemplateRemove);
    this->Add("TemplateEdit", &DungeonService::TemplateEdit);
    this->Add("AddTemplateObjects", &DungeonService::AddTemplateObjects);
    this->Add("TemplateObjectAddDungeonList", &DungeonService::TemplateObjectAddDungeonList);
    // others
    this->Add("GetArchetypes", &DungeonService::GetArchetypes);
    this->Add("DEGetFactions", &DungeonService::DEGetFactions);
    this->Add("DEGetDungeons", &DungeonService::DEGetDungeons);
    this->Add("DEGetTemplates", &DungeonService::DEGetTemplates);
    this->Add("DEGetRooms", &DungeonService::DEGetRooms);
    this->Add("DEGetRoomObjectPaletteData", &DungeonService::DEGetRoomObjectPaletteData);
}

PyResult DungeonService::IsObjectLocked(PyCallArgs& call, PyInt* objectID)
{
    //return sm.RemoteSvc('dungeon').IsObjectLocked(objectID)
    _log(DUNG__CALL,  "DungeonService::Handle_IsObjectLocked size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::AddObject(PyCallArgs& call, PyInt* roomID, PyInt* tupeID, PyFloat* x, PyFloat* y, PyFloat* z, PyFloat* pitch, PyFloat* roll, PyFloat* radius)
{
    // (newObjectID, revisionID,) = sm.RemoteSvc('dungeon').AddObject(roomID, typeID, x, y, z, yaw, pitch, roll, radius)

    _log(DUNG__CALL,  "DungeonService::Handle_AddObject size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::RemoveObject(PyCallArgs& call, PyInt* objectID)
{
    //sm.RemoteSvc('dungeon').RemoveObject(objectID)
    _log(DUNG__CALL,  "DungeonService::Handle_RemoveObject  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::CopyObject(PyCallArgs& call, PyInt* objectID, PyInt* roomID, PyFloat* offsetX, PyFloat* offsetY, PyFloat* offsetZ)
{
    //newObjectID = sm.RemoteSvc('dungeon').CopyObject(objectID, roomID, offsetX, offsetY, offsetZ)
    _log(DUNG__CALL,  "DungeonService::Handle_CopyObject  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::EditObject(PyCallArgs& call)
{
    _log(DUNG__CALL,  "DungeonService::Handle_EditObject  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::EditObjectName(PyCallArgs& call, PyInt* newObjectID, PyWString* objectName)
{
    //sm.RemoteSvc('dungeon').EditObjectName(newObjectID, objectName)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectName  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::EditObjectRadius(PyCallArgs& call)
{
    //sm.RemoteSvc('dungeon').EditObjectRadius(objectID=objectID, radius=radius)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectRadius  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::EditObjectXYZ(PyCallArgs& call)
{
    //sm.RemoteSvc('dungeon').EditObjectXYZ(objectID=objectID, x=x, y=y, z=z)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectXYZ  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::EditObjectYawPitchRoll(PyCallArgs& call)
{
    //sm.RemoteSvc('dungeon').EditObjectYawPitchRoll(objectID=objectID, yaw=yaw, pitch=pitch, roll=roll)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectYawPitchRoll size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::TemplateAdd(PyCallArgs& call, PyWString* templateName, PyWString* templateDescription)
{
    //templateID = sm.RemoteSvc('dungeon').TemplateAdd(templateName, templateDescription)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateAdd  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::TemplateRemove(PyCallArgs& call, PyInt* templateID)
{
    //sm.RemoteSvc('dungeon').TemplateRemove(self.sr.node.id)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateRemove  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::TemplateEdit(PyCallArgs& call, PyInt* templateID, PyWString* templateName, PyWString* templateDescription)
{
    //dungeonSvc.TemplateEdit(self.templateRow.templateID, templateName, templateDescription)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateEdit  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::AddTemplateObjects(PyCallArgs& call, PyInt* roomID, PyInt* objectID, PyTuple* position)
{
    // objectIDs = sm.RemoteSvc('dungeon').AddTemplateObjects(roomID, self.sr.node.id, (posInRoom.x, posInRoom.y, posInRoom.z)

    _log(DUNG__CALL,  "DungeonService::Handle_AddTemplateObjects  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::TemplateObjectAddDungeonList(PyCallArgs& call, PyInt* templateID, PyList* objectIDs)
{
    //sm.RemoteSvc('dungeon').TemplateObjectAddDungeonList(templateID, objectIDList)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateObjectAddDungeonList  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}


PyResult DungeonService::GetArchetypes(PyCallArgs& call)
{
    /**     archetypes = sm.RemoteSvc('dungeon').GetArchetypes()
     *        archetypeOptions = [ (archetype.archetypeName, archetype.archetypeID) for archetype in archetypes ]
     */
    _log(DUNG__CALL,  "DungeonService::Handle_GetArchetypes  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::DEGetDungeons(PyCallArgs& call)
{
    /* dungeon = sm.RemoteSvc('dungeon').DEGetDungeons(archetypeID=archetypeID, factionID=factionID)
     * dungeon = sm.RemoteSvc('dungeon').DEGetDungeons(dungeonID=dungeonID)[0]
     * dungeon.dungeonNameID, dungeon.dungeonID, dungeon.factionID
     */

    //PyRep *result = NULL;
    //dict args:
    // factionID
    // or dungeonVID

    // rows: status (1=RELEASE,2=TESTING,else Working Copy),
    //       dungeonVName
    //       dungeonVID

    _log(DUNG__CALL,  "DungeonService::Handle_DEGetDungeons  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::DEGetTemplates(PyCallArgs& call)
{
/*
        self.templateRows = sm.RemoteSvc('dungeon').DEGetTemplates()
        for row in self.templateRows:
            data = {'label': row.templateName,
             'hint': row.description != row.templateName and row.description or '',
             'id': row.templateID,
             'form': self}
             */

    _log(DUNG__CALL,  "DungeonService::Handle_DEGetTemplates  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}


PyResult DungeonService::DEGetRooms(PyCallArgs& call)
{
    //rooms = sm.RemoteSvc('dungeon').DEGetRooms(dungeonID=seldungeon.dungeonID)
    //dict arg: dungeonVID
    //PyRep *result = NULL;

    //rows: roomName

    _log(DUNG__CALL,  "DungeonService::Handle_DEGetRooms  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::DEGetRoomObjectPaletteData(PyCallArgs& call)
{
    /*  roomObjectGroups = sm.RemoteSvc('dungeon').DEGetRoomObjectPaletteData()
     * id, name  in either dict or list (client can process both)
     */
    _log(DUNG__CALL,  "DungeonService::Handle_DEGetRoomObjectPaletteData  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult DungeonService::DEGetFactions(PyCallArgs& call)
{
    /* factions = sm.RemoteSvc('dungeon').DEGetFactions()
        factionOptions = [ (cfg.eveowners.Get(faction.factionID).name, faction.factionID) for faction in factions ]
        */
    _log(DUNG__CALL,  "DungeonService::Handle_DEGetFactions  size: %li", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}


/*{'messageKey': 'DunAuthoringError', 'dataID': 17883918, 'suppressable': False, 'bodyID': 259674, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 830}
 * {'messageKey': 'DunBlacklistCannotWarp', 'dataID': 17880454, 'suppressable': False, 'bodyID': 258397, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258396, 'messageID': 2100}
 * {'messageKey': 'DunCantAnchor', 'dataID': 17883867, 'suppressable': False, 'bodyID': 259656, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 831}
 * {'messageKey': 'DunCantUseMWD', 'dataID': 17883442, 'suppressable': False, 'bodyID': 259505, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 832}
 * {'messageKey': 'DunGateLocked_ManyKeys', 'dataID': 17883928, 'suppressable': False, 'bodyID': 259678, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259677, 'messageID': 834}
 * {'messageKey': 'DunGateLocked_OneKey', 'dataID': 17883923, 'suppressable': False, 'bodyID': 259676, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259675, 'messageID': 835}
 * {'messageKey': 'DunGateLocked_ZeroKey', 'dataID': 17883939, 'suppressable': False, 'bodyID': 259682, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259681, 'messageID': 836}
 * {'messageKey': 'DunGateNPCsAround', 'dataID': 17883447, 'suppressable': False, 'bodyID': 259507, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259506, 'messageID': 837}
 * {'messageKey': 'DunGateNoSkill', 'dataID': 17883915, 'suppressable': False, 'bodyID': 259673, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259672, 'messageID': 838}
 * {'messageKey': 'DunGateTooFarAway', 'dataID': 17883844, 'suppressable': False, 'bodyID': 259648, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259647, 'messageID': 839}
 * {'messageKey': 'DunPodsCannotWarp', 'dataID': 17883452, 'suppressable': False, 'bodyID': 259509, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259508, 'messageID': 840}
 * {'messageKey': 'DunShipCannotWarp', 'dataID': 17883907, 'suppressable': False, 'bodyID': 259670, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259669, 'messageID': 841}
 */
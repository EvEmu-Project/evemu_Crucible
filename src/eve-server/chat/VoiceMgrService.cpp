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
*/

#include "eve-server.h"


#include "chat/VoiceMgrService.h"

VoiceMgrService::VoiceMgrService() :
    Service("voiceMgr")
{
    this->Add("VoiceEnabled", &VoiceMgrService::VoiceEnabled);
}

PyResult VoiceMgrService::VoiceEnabled(PyCallArgs &call) {
    //maybe in future, but not now ...
    return(new PyInt(0));
}

/*
        [PySubStream 27 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "GetPassword"]
            [PyTuple 0 items]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]

      [PySubStream 30 bytes]
        [PyString "5B738A603AE44313A696451"]

*/


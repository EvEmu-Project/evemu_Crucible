/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2014 The EVEmu Team
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

#ifndef EVEMU_SYSTEM_WORMHOLESVC_H_
#define EVEMU_SYSTEM_WORMHOLESVC_H_

#include "PyService.h"
//#include "system/SystemDB.h"

class WormHoleSvc
: public PyService
{
public:
    WormHoleSvc(PyServiceMgr *mgr);
    virtual ~WormHoleSvc();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    //WormholeDB m_db;
    PyCallable_DECL_CALL(WormholeJump);
};

#endif  // EVEMU_SYSTEM_WORMHOLESVC_H_
/**
                wormholeClasses = {0: 'UI/Wormholes/Classes/Space',
                 1: 'UI/Wormholes/Classes/UnknownSpace',
                 2: 'UI/Wormholes/Classes/UnknownSpace',
                 3: 'UI/Wormholes/Classes/UnknownSpace',
                 4: 'UI/Wormholes/Classes/UnknownSpace',
                 5: 'UI/Wormholes/Classes/DeepUnknownSpace',
                 6: 'UI/Wormholes/Classes/DeepUnknownSpace',
                 7: 'UI/Wormholes/Classes/HighSecuritySpace',
                 8: 'UI/Wormholes/Classes/LowSecuritySpace',
                 9: 'UI/Wormholes/Classes/NullSecuritySpace'}

                 */
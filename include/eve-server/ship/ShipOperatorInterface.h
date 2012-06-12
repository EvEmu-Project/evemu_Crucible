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
    Author:        Aknor Jaden
*/

#ifndef __SHIPOPERATORINTERFACE_H_INCL__
#define __SHIPOPERATORINTERFACE_H_INCL__

#include "inventory/ItemRef.h"

class Client;
class DestinyManager;
class NPC;

class ShipOperatorInterface
{
public:
    ShipOperatorInterface()
    {
        m_pClient = NULL;
        m_pNPC = NULL;
        //m_pPCP = NULL;
        //sLog.Error( "ShipOperatorInterface default Constructor:", "YOU MUST NOT INVOKE DEFAULT CONSTRUCTOR !" );
        //assert( false );
    }

    ~ShipOperatorInterface()
    {
        m_pClient = NULL;
        m_pNPC = NULL;
        //m_pPCP = NULL;
    }

    void SetOperatorObject(Client * pClient);
    void SetOperatorObject(NPC * pNPC);
    //void SetOperatorObject(PCP * pPCP);

    // Public Interface Functions:
    bool IsClient() { if( m_pClient != NULL ) return true; else return false; }
    bool IsNPC() { if( m_pNPC != NULL ) return true; else return false; }
    //bool IsPCP() { if( m_pPCP != NULL ) return true; else return false; }

    // Pass Through calls to Operator object:
    // CLIENT Object calls:
    void SendNotifyMsg( const char* fmt, va_list args );
    void SendErrorMsg( const char* fmt, va_list args );
    const char *GetName() const;
    ShipRef GetShip() const;
    CharacterRef GetChar() const;
    DestinyManager * GetDestiny();
    uint32 GetLocationID() const;
    void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);

    // NPC Object calls:
    // none

    // PCP Object calls:
    // none

protected:
    Client * m_pClient;        // We do not own this
    NPC * m_pNPC;            // We do not own this
    //PCP * m_pPCP;            // We do not own this

};

#endif  /* SHIPOPERATORINTERFACE_H */



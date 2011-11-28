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

#include "EVEServerPCH.h"

void ShipOperatorInterface::SetOperatorObject(Client * pClient)
{
    m_pClient = pClient;
    m_pNPC = NULL;
//    m_pPCP = NULL;
}

void ShipOperatorInterface::SetOperatorObject(NPC * pNPC)
{
    m_pClient = NULL;
    m_pNPC = pNPC;
//    m_pPCP = NULL;
}

/*
void ShipOperatorInterface::SetOperatorObject(PCP * pPCP)
{
    m_pClient = NULL;
    m_pNPC = NULL;
    m_pPCP = pPCP;
}
*/

void ShipOperatorInterface::SendNotifyMsg( const char* fmt, va_list args )
{
    if( m_pClient != NULL )
        m_pClient->SendNotifyMsg( fmt, args );
//    else if( m_pNPC != NULL )
//        m_pNPC->SendNotifyMsg( fmt, args );
    else
    {
        sLog.Error( "ShipOperatorInterface::SendNotifyMsg()", "ERROR: All Operator types should be accounted for" );
        assert(false);
    }
}

void ShipOperatorInterface::SendErrorMsg( const char* fmt, va_list args )
{
    if( m_pClient != NULL )
        m_pClient->SendErrorMsg( fmt, args );
//    else if( m_pNPC != NULL )
//        m_pNPC->SendErrorMsg( fmt, args );
    else
    {
        sLog.Error( "ShipOperatorInterface::SendErrorMsg()", "ERROR: All Operator types should be accounted for" );
        assert(false);
    }
}

const char * ShipOperatorInterface::GetName() const
{
    if( m_pClient != NULL )
        return m_pClient->GetChar() ? m_pClient->GetChar()->itemName().c_str() : "(null)";
//    else if( m_pNPC != NULL )
//        return m_pNPC->GetChar() ? m_pNPC->GetChar()->itemName().c_str() : "(null)";

    sLog.Error( "ShipOperatorInterface::GetName()", "ERROR: All Operator types should be accounted for" );
    assert(false);
    return NULL;
}

ShipRef ShipOperatorInterface::GetShip() const
{
    if( m_pClient != NULL )
        return ShipRef::StaticCast( m_pClient->Item() );
//    else if( m_pNPC != NULL )
//        return ShipRef::StaticCast( m_pNPC->Item() );

    sLog.Error( "ShipOperatorInterface::GetShip()", "ERROR: All Operator types should be accounted for" );
    assert(false);
    
    // Hack to get rid of warning, code execution should never reach this point
    ShipRef ret;
    return ret;
}

CharacterRef ShipOperatorInterface::GetChar() const
{
    if( m_pClient != NULL )
        return m_pClient->GetChar();
//    else if( m_pNPC != NULL )
//        return ShipRef::StaticCast( m_pNPC->Item() );

    sLog.Error( "ShipOperatorInterface::GetChar()", "ERROR: All Operator types should be accounted for" );
    assert(false);

    // Hack to get rid of warning, code execution should never reach this point
    CharacterRef ret;
    return ret;
}

uint32 ShipOperatorInterface::GetLocationID() const
{
    if( m_pClient != NULL )
        return m_pClient->GetLocationID();
//    else if( m_pNPC != NULL )
//        return ShipRef::StaticCast( m_pNPC->Item() );
    
    sLog.Error( "ShipOperatorInterface::GetLocationID()", "ERROR: All Operator types should be accounted for" );
    assert(false);

    // Hack to get rid of warning, code execution should never reach this point
    return NULL;
}

void ShipOperatorInterface::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag)
{
    if( m_pClient != NULL )
        m_pClient->MoveItem( itemID, location, flag );
//    else if( m_pNPC != NULL )
//        return ShipRef::StaticCast( m_pNPC->Item() );
    else
    {
        sLog.Error( "ShipOperatorInterface::MoveItem()", "ERROR: All Operator types should be accounted for" );
        assert(false);
    }
}

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

#include "EVECommonPCH.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEUnmarshal.h"
#include "network/EVESession.h"
#include "packets/Crypto.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyPacket.h"
#include "python/PyDumpVisitor.h"
#include "EVEVersion.h"

EVEClientSession::EVEClientSession( EVETCPConnection** n )
: mNet( *n ),
  mPacketHandler( NULL )
{
    *n = NULL;
}

EVEClientSession::~EVEClientSession()
{
    // Destroy connection we used
    delete mNet;
}

void EVEClientSession::Reset()
{
    mPacketHandler = NULL;

    if( GetState() != TCPConnection::STATE_CONNECTED )
        // Connection has been lost, there's no point in reset
        return;

    VersionExchange version;
    _GetVersion( version );

    PyRep* r = version.Encode();
    mNet->QueueRep( r );
    PyDecRef( r );

    mPacketHandler = &EVEClientSession::_HandleVersion;
}

void EVEClientSession::QueuePacket( const PyPacket* p )
{
	if (p == NULL)
		return;
    PyPacket* packet = p->Clone();

    if (packet == NULL)
    {
        sLog.Error("Network", "QueuePacket was unable to clone a PyPacket");
        return;
    }

    FastQueuePacket( &packet );
}

void EVEClientSession::FastQueuePacket( PyPacket** p )
{
    if(p == NULL || *p == NULL)
        return;

    PyRep* r = (*p)->Encode();
    // maybe change PyPacket to a object with a reference..
    SafeDelete( *p );
    if( r == NULL )
    {
        sLog.Error("Network", "%s: Failed to encode a Fast queue packet???", GetAddress().c_str());
        return;
    }

    mNet->QueueRep( r );
    PyDecRef( r );
}

PyPacket* EVEClientSession::PopPacket()
{
    PyRep* r = mNet->PopRep();
    if( r == NULL )
        return NULL;

    if(is_log_enabled(NET__PRES_REP)) {
        _log(NET__PRES_REP, "%s: Raw Rep Dump:", GetAddress().c_str());
        r->Dump(NET__PRES_REP, "    ");
    }

    assert( mPacketHandler );
    return ( this->*mPacketHandler )( r );
}

PyPacket* EVEClientSession::_HandleVersion( PyRep* rep )
{
    //we are waiting for their version information...
    VersionExchange ve;
    if( !ve.Decode( &rep ) )
        sLog.Error("Network", "%s: Received invalid version exchange!", GetAddress().c_str());
    else if( _VerifyVersion( ve ) )
        mPacketHandler = &EVEClientSession::_HandleCommand;

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleCommand( PyRep* rep )
{
    //check if it actually is tuple
    if( !rep->IsTuple() )
    {
        sLog.Error("Network", "%s: Invalid packet during waiting for command (tuple expected).", GetAddress().c_str());
    }
    // decode
    else if( rep->AsTuple()->size() == 2 )
    {
        //QC = Queue Check
        NetCommand_QC cmd;
        if( !cmd.Decode( &rep ) )
        {
            sLog.Error("Network", "%s: Failed to decode 2-arg command.", GetAddress().c_str());
        }
        else
        {
            sLog.Debug("Network", "%s: Got Queue Check command.", GetAddress().c_str());

            //they return position in queue
            PyRep* rsp = new PyInt( _GetQueuePosition() );
            mNet->QueueRep( rsp );
            PyDecRef( rsp );

            //now reset connection
            Reset();
        }
    }
    else if( rep->AsTuple()->size() == 3 )
    {
        //this is sent when client is logging in
        NetCommand_VK cmd;
        if( !cmd.Decode( &rep ) )
        {
            sLog.Error("Network", "%s: Failed to decode 3-arg command.", GetAddress().c_str());
        }
        else
        {
            sLog.Debug("Network", "%s: Got VK command, vipKey=%s.", GetAddress().c_str(), cmd.vipKey.c_str());

            if( _VerifyVIPKey( cmd.vipKey ) )
                mPacketHandler = &EVEClientSession::_HandleCrypto;
        }
    }
    else
    {
        _log(NET__PRES_ERROR, "%s: Received invalid command packet:", GetAddress().c_str());
        rep->Dump(NET__PRES_ERROR, "  ");
    }

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleCrypto( PyRep* rep )
{
    CryptoRequestPacket cr;
    if( !cr.Decode( &rep ) )
        sLog.Error("Network", "%s: Received invalid crypto request!", GetAddress().c_str());
    else if( _VerifyCrypto( cr ) )
        mPacketHandler = &EVEClientSession::_HandleAuthentication;

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleAuthentication( PyRep* rep )
{
    //just to be sure
    CryptoChallengePacket ccp;
    if( !ccp.Decode( &rep ) )
		sLog.Error("Network", "%s: Received invalid crypto challenge!", GetAddress().c_str());
    else if( _VerifyLogin( ccp ) )
        mPacketHandler = &EVEClientSession::_HandleFuncResult;

    return PopPacket();
}

PyPacket* EVEClientSession::_HandleFuncResult( PyRep* rep )
{
    CryptoHandshakeResult hr;
    if( !hr.Decode( &rep ) )
		sLog.Error("Network", "%s: Received invalid crypto handshake result!", GetAddress().c_str());
    else if( _VerifyFuncResult( hr ) )
        mPacketHandler = &EVEClientSession::_HandlePacket;

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandlePacket( PyRep* rep )
{
    //take the PyRep and turn it into a PyPacket
    PyPacket* p = new PyPacket;
    if( !p->Decode( &rep ) ) //rep is consumed here
    {
		sLog.Error("Network", "%s: Failed to decode packet rep", GetAddress().c_str());
        SafeDelete( p );
    }
    else
        return p;

    // recurse
    return PopPacket();
}

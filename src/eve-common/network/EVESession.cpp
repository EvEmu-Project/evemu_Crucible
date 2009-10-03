/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
#include "network/packet_functions.h"
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
    //let the server thread know we are done with this connection
    //mNet->Free();
}

void EVEClientSession::Reset()
{
    mPacketHandler = NULL;

    if( !Connected() )
        // Connection has been lost, there's no point in reset
        return;

    VersionExchange version;
    version.birthday = EVEBirthday;
    version.macho_version = MachoNetVersion;
    version.user_count = _GetUserCount();
    version.version_number = EVEVersionNumber;
    version.build_version = EVEBuildVersion;
    version.project_version = EVEProjectVersion;

    PyRep* r = version.FastEncode();
    mNet->QueueRep( r );
    PyDecRef( r );

    mPacketHandler = &EVEClientSession::_HandleVersion;
}

void EVEClientSession::QueuePacket( const PyPacket* p )
{
    PyPacket* packet = p->Clone();

    FastQueuePacket( &packet );
}

void EVEClientSession::FastQueuePacket( PyPacket** p )
{
    if(p == NULL || *p == NULL)
        return;

    PyRep* r = (*p)->Encode();
    SafeDelete( *p );
    if( r == NULL )
    {
        _log(NET__PRES_ERROR, "%s: Failed to encode???", GetAddress().c_str());
        return;
    }

    mNet->QueueRep( r );
    PyDecRef( r );
}

PyPacket* EVEClientSession::PopPacket()
{
    PyRep *r = mNet->PopRep();
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
    {
        _log(NET__PRES_ERROR, "%s: Received invalid version exchange!", GetAddress().c_str());
    }
    else
    {
        _log(NET__PRES_REP, "%s: Received Low Level Version Exchange:", GetAddress().c_str());
        ve.Dump(NET__PRES_REP, "    ");

        if(ve.birthday != EVEBirthday)
            _log(NET__PRES_ERROR, "%s: Client's birthday does not match ours!", GetAddress().c_str());

        if(ve.macho_version != MachoNetVersion)
            _log(NET__PRES_ERROR, "%s: Client's macho_version not match ours!", GetAddress().c_str());

        if(ve.version_number != EVEVersionNumber)
            _log(NET__PRES_ERROR, "%s: Client's version_number not match ours!", GetAddress().c_str());

        if(ve.build_version != EVEBuildVersion)
            _log(NET__PRES_ERROR, "%s: Client's build_version not match ours!", GetAddress().c_str());

        if(ve.project_version != EVEProjectVersion)
            _log(NET__PRES_ERROR, "%s: Client's project_version not match ours!", GetAddress().c_str());

        mPacketHandler = &EVEClientSession::_HandleCommand;
    }

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleCommand( PyRep* rep )
{
    //check if it actually is tuple
    if( !rep->IsTuple() )
    {
        _log(NET__PRES_ERROR, "%s: Invalid packet during waiting for command (tuple expected).", GetAddress().c_str());
    }
    // decode
    else if( rep->AsTuple().size() == 2 )
    {
        //QC = Queue Check
        NetCommand_QC cmd;
        if( !cmd.Decode( &rep ) )
        {
            _log(NET__PRES_ERROR, "%s: Failed to decode 2-arg command.", GetAddress().c_str());
        }
        else if( cmd.command != "QC" )
        {
            _log(NET__PRES_ERROR, "%s: Unknown 2-arg command '%s'.", GetAddress().c_str(), cmd.command.c_str());
        }
        else
        {
            _log(NET__PRES_DEBUG, "%s: Got Queue Check command.", GetAddress().c_str());

            //they return position in queue
            PyRep* rsp = new PyInt( _GetQueuePosition() );
            mNet->QueueRep( rsp );
            PyDecRef( rsp );

            //now reset connection
            Reset();
        }
    }
    else if( rep->AsTuple().size() == 3 )
    {
        //this is sent when client is logging in
        NetCommand_VK cmd;
        if( !cmd.Decode( &rep ) )
        {
            _log(NET__PRES_ERROR, "%s: Failed to decode 3-arg command.", GetAddress().c_str());
        }
        else if( cmd.command != "VK" )
	    {
            _log(NET__PRES_ERROR, "%s: Unknown 3-arg command '%s'.", GetAddress().c_str(), cmd.command.c_str());
        }
        else
        {
            _log(NET__PRES_DEBUG, "%s: Got VK command, vipKey=%s.", GetAddress().c_str(), cmd.vipKey.c_str());

            // Pass the VIP key to child
            _VIPKeyReceived( cmd.vipKey );

	        // Continue in handshake
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
    {
        _log(NET__PRES_ERROR, "%s: Received invalid crypto request!", GetAddress().c_str());
    }
    else if( cr.keyVersion != "placebo" )
    {
        //I'm sure cr.keyVersion can specify either CryptoAPI or PyCrypto, but its all binary so im not sure how.
        CryptoAPIRequestParams car;
        if( !car.Decode( cr.keyParams ) )
        {
            _log(NET__PRES_ERROR, "%s: Received invalid CryptoAPI request!", GetAddress().c_str());
        }
        else
        {
            _log(NET__PRES_ERROR, "%s: Unhandled CryptoAPI request: hashmethod=%s sessionkeylength=%d provider=%s sessionkeymethod=%s", GetAddress().c_str(), car.hashmethod.c_str(), car.sessionkeylength, car.provider.c_str(), car.sessionkeymethod.c_str());
            _log(NET__PRES_ERROR, "%s: You must change your client to use Placebo crypto in common.ini to talk to this server!\n", GetAddress().c_str());
        }
    }
    else
    {
        _log(NET__PRES_DEBUG, "%s: Received Placebo crypto request, accepting.", GetAddress().c_str());

        //send out accept response
        PyRep* rsp = new PyString( "OK CC" );
        mNet->QueueRep( rsp );
        PyDecRef( rsp );

        mPacketHandler = &EVEClientSession::_HandleAuthentication;
    }

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleAuthentication( PyRep* rep )
{
    //just to be sure
    CryptoChallengePacket ccp;
    if( !ccp.Decode( &rep ) )
    {
        _log(NET__PRES_ERROR, "%s: Received invalid crypto challenge!", GetAddress().c_str());
    }
    else
    {
        _log(NET__PRES_DEBUG, "%s: Received Client Challenge.", GetAddress().c_str());

        if( _Authenticate( ccp ) )
            // Continue in handshake
            mPacketHandler = &EVEClientSession::_HandleFuncResult;
    }

    return PopPacket();
}

PyPacket* EVEClientSession::_HandleFuncResult( PyRep* rep )
{
    CryptoHandshakeResult hr;
    if( !hr.Decode( &rep ) )
    {
        _log(NET__PRES_ERROR, "%s: Received invalid crypto handshake result!", GetAddress().c_str());
    }
    else
    {
        _log(NET__PRES_DEBUG, "%s: Handshake result received.", GetAddress().c_str());

        // Pass it to child
        _FuncResultReceived( hr );

        // Continue in handshake
        mPacketHandler = &EVEClientSession::_HandlePacket;
    }

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandlePacket( PyRep* rep )
{
    //take the PyRep and turn it into a PyPacket
    PyPacket* p = new PyPacket;
    if( !p->Decode( &rep ) ) //rep is consumed here
    {
        _log(NET__PRES_ERROR, "%s: Failed to decode packet rep", GetAddress().c_str());
        SafeDelete( p );
    }
    else
    {
        return p;
    }

    // recurse
    return PopPacket();
}

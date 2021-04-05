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
    Author:     Zhur
*/

#include "eve-common.h"

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
mPacketHandler( nullptr )
{
    *n = nullptr;
}

EVEClientSession::~EVEClientSession() {
    // Destroy connection we used
    delete mNet;
}

void EVEClientSession::Reset() {
    mPacketHandler = nullptr;

    if ( GetState() != TCPConnection::STATE_CONNECTED )
        // Connection has been lost, there's no point in reset
        return;

    VersionExchangeServer version;
    _GetVersion( version );

    PyRep* res(version.Encode());
    mNet->QueueRep(res);

    mPacketHandler = &EVEClientSession::_HandleVersion;
}

void EVEClientSession::QueuePacket( PyPacket* packet ) {
    if (packet == nullptr)
        return;

    PyRep* res(packet->Encode());

    if (res == nullptr) {
        sLog.Error("QueuePacket", "%s: Failed to encode a packet.", GetAddress().c_str());
        return;
    }

    mNet->QueueRep( res );
}

PyPacket* EVEClientSession::PopPacket() {
    PyRep* rep(mNet->PopRep());
    if (rep == nullptr)
        return nullptr;

    if (is_log_enabled(NET__PRES_REP)) {
        _log(NET__PRES_REP, "%s: Raw Rep Dump:", GetAddress().c_str());
        rep->Dump(NET__PRES_REP, "    ");
    }

    assert( mPacketHandler );
    return ( this->*mPacketHandler )( rep );
}

PyPacket* EVEClientSession::_HandleVersion( PyRep* rep ) {
    //we are waiting for their version information...
    VersionExchangeClient ve;
    if ( !ve.Decode( &rep ) )
        sLog.Error("_HandleVersion", "%s: Received invalid version exchange!", GetAddress().c_str());
    else if ( _VerifyVersion( ve ) )
        mPacketHandler = &EVEClientSession::_HandleCommand;

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleCommand( PyRep* rep ) {
    //check if it actually is tuple
    if ( !rep->IsTuple() )
        sLog.Error("_HandleCommand", "%s: Invalid packet during waiting for command (tuple expected).", GetAddress().c_str());
    else if ( rep->AsTuple()->size() == 2 ) {    // decode
        //QC = Queue Check
        NetCommand_QC cmd;
        if ( !cmd.Decode( &rep ) )
            sLog.Error("_HandleCommand", "%s: Failed to decode 2-arg command.", GetAddress().c_str());
        else {
            sLog.Debug("_HandleCommand", "%s: Got Queue Check command.", GetAddress().c_str());

            //they return position in queue
            PyRep* rsp = new PyInt( _GetQueuePosition() );
            mNet->QueueRep( rsp );

            //now reset connection
            Reset();
        }
    } else if ( rep->AsTuple()->size() == 3 ) {
        //this is sent when client is logging in
        NetCommand_VK cmd;
        if ( !cmd.Decode( &rep ) ) {
            sLog.Error("_HandleCommand", "%s: Failed to decode 3-arg command.", GetAddress().c_str());
        } else {
            sLog.Debug("_HandleCommand", "%s: Got VK command, vipKey=%s.", GetAddress().c_str(), cmd.vipKey.c_str());

            if ( _VerifyVIPKey( cmd.vipKey ) )
                mPacketHandler = &EVEClientSession::_HandleCrypto;
        }
    } else {
        if (is_log_enabled(NET__PRES_ERROR)) {
            _log(NET__PRES_ERROR, "%s: Received invalid command packet:", GetAddress().c_str());
            rep->Dump(NET__PRES_ERROR, "  ");
        }
    }

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleCrypto( PyRep* rep ) {
    CryptoRequestPacket cr;
    if ( !cr.Decode( &rep ) )
        sLog.Error("_HandleCrypto", "%s: Received invalid crypto request!", GetAddress().c_str());
    else if ( _VerifyCrypto( cr ) )
        mPacketHandler = &EVEClientSession::_HandleAuthentication;

    // recurse
    return PopPacket();
}

PyPacket* EVEClientSession::_HandleAuthentication( PyRep* rep ) {
    //just to be sure
    CryptoChallengePacket ccp;
    if ( !ccp.Decode( &rep ) )
        sLog.Error("_HandleAuthentication", "%s: Received invalid crypto challenge!", GetAddress().c_str());
    else if ( _VerifyLogin( ccp ) )
        mPacketHandler = &EVEClientSession::_HandleFuncResult;

    return PopPacket();
}

PyPacket* EVEClientSession::_HandleFuncResult( PyRep* rep ) {
    CryptoHandshakeResult hr;
    if ( !hr.Decode( &rep ) )
        sLog.Error("_HandleFuncResult", "%s: Received invalid crypto handshake result!", GetAddress().c_str());
    else if ( _VerifyFuncResult( hr ) )
        mPacketHandler = &EVEClientSession::_HandlePacket;

    return PopPacket();
}

PyPacket* EVEClientSession::_HandlePacket( PyRep* rep ) {
    //take the PyRep and turn it into a PyPacket
    PyPacket* p = new PyPacket();
    if ( !p->Decode( &rep ) ) { //rep is consumed here
        sLog.Error("_HandlePacket", "%s: Failed to decode packet rep", GetAddress().c_str());
        SafeDelete( p );
        PySafeDecRef(rep);
        return PopPacket();
    }

    return p;
}

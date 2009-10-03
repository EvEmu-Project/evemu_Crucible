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
	Author:		Zhur
*/

#ifndef __EVE_SESSION_H__INCL__
#define __EVE_SESSION_H__INCL__

#include "network/EVETCPConnection.h"

class PyPacket;
class PyRep;

class CryptoChallengePacket;
class CryptoHandshakeResult;

class EVEClientSession
{
public:
	EVEClientSession( EVETCPConnection** n );
	virtual ~EVEClientSession();

    bool Connected() const { return mNet->Connected(); }
	std::string GetAddress() const { return mNet->GetAddress(); }
	void Disconnect() { return mNet->Disconnect(); }

	void Reset();

	void QueuePacket( const PyPacket* p );
	void FastQueuePacket( PyPacket** p );

	PyPacket* PopPacket();

protected:
	virtual uint32 _GetUserCount() = 0;
	virtual uint32 _GetQueuePosition() = 0;

	virtual bool _Authenticate( CryptoChallengePacket& ccp ) = 0;

	virtual void _VIPKeyReceived( const std::string& vipKey ) = 0;
	virtual void _FuncResultReceived( CryptoHandshakeResult& result ) = 0;

	EVETCPConnection* const mNet;	//we do not own this, is never NULL

private:
    // State machine facility:
	PyPacket* ( EVEClientSession::*mPacketHandler )( PyRep* rep );

	PyPacket* _HandleVersion( PyRep* rep );
	PyPacket* _HandleCommand( PyRep* rep );
	PyPacket* _HandleCrypto( PyRep* rep );
	PyPacket* _HandleAuthentication( PyRep* rep );
	PyPacket* _HandleFuncResult( PyRep* rep );
	PyPacket* _HandlePacket( PyRep* rep );
};

#endif /* !__EVE_SESSION_H__INCL__ */


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

#ifndef EVE_PRESENTATION_H
#define EVE_PRESENTATION_H

#include <string>

#include "types.h"
#include "EVETCPConnection.h"

class PyPacket;
class PyRep;
class CryptoChallengePacket;
class Client;

class EVETCPConnection;

static const uint32 EVESocketMaxNumberOfBytes = 1000000; //this is a limit hard coded in the eve client! (NetClient.dll)

class EVEPresentation {
	friend class Client;	//TODO: get rid of this crap
public:
	EVEPresentation(EVETCPConnection *n, Client *c);
	virtual ~EVEPresentation();
	
	void QueuePacket(PyPacket *p);
	void FastQueuePacket(PyPacket **p);
	
	bool Connected() const;
	void Disconnect();

	PyPacket *PopPacket();

	std::string GetConnectedAddress() const;

	//hack for now:
	void SendHandshake(uint32 user_count) { m_userCount = user_count; _SendHandshake(); }
	
protected:
	void _QueueRep(const PyRep *rep);
	void _NetQueuePacket(EVENetPacket **p);
	PyPacket *_Dispatch(PyRep *r);
	void _SendHandshake();

	EVETCPConnection *const net;	//we do not own this, is never NULL
	Client *const client;	//we do not own this, never NULL; used for low-level login (see state CryptoHandshakeSent)
	CryptoChallengePacket *m_request;
	uint32 m_userCount;

	enum State {
		VersionNotReceived,
		WaitingForCommand,
		CryptoRequestNotReceived,
		CryptoRequestReceived_ChallengeWait,
		CryptoHandshakeSent,	//waiting for handshake result
		AcceptingPackets
	};
	State m_state;
};

#endif


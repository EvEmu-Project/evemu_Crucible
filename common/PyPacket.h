/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef EVE_PY_PACKET_H
#define EVE_PY_PACKET_H

#include <stdio.h>
#include <string>

#include "packet_types.h"
#include "logsys.h"

class PyRep;
class PyRepTuple;
class PyRepDict;
class PyVisitor;

class PyAddress {
public:
	PyAddress();
	
	void Dump(FILE *into, const char *pfx) const;
	void Dump(LogType type, const char *pfx) const;
	bool Decode(PyRep *&object);	//consumes object
	PyRep *Encode();
	void operator=(const PyAddress &right);


	typedef enum {
		Any 	= 'A',
			/*                                                                              
            * [1]   service
            * [2]   callID
            */
		Node	= 'N',
			/*                                                                              
            * [1]   nodeID
            * [2]   service
            * [3]   callID
            */
		Client	= 'C',
			/*                                                                              
            * [1]   clientID
            * [2]   callID
            * [3]   service
            */
		Broadcast = 'B',
			/*                                                                              
            * [1]   broadcastID
            * [2]   narrowcast??
            * [3]   idtype
            */
		Invalid = 'I'	//not real
	} AddrType;
	
	AddrType type;
	uint64 typeID;	//node id, client id, etc...
	uint64 callID;

	
	std::string service;	//broadcastID for a broadcast.
	std::string bcast_idtype;

protected:
	bool _DecodeService(PyRep *rep);
	bool _DecodeCallID(PyRep *rep);
	bool _DecodeTypeID(PyRep *rep);
};

class PyPacket {
public:
	PyPacket();
	~PyPacket();
	
	void Dump(LogType type, PyVisitor *dumper);
	bool Decode(PyRep *&packet);	//consumes packet
	PyRep *Encode();
	PyPacket *Clone() const;

	//the "type" of object this represents
	std::string type_string;

	//the contents tuple:
	MACHONETMSG_TYPE	type;
	PyAddress	source;
	PyAddress	dest;
	uint32		userid;
	PyRepTuple	*payload;
	PyRepDict	*named_payload;	//oob data mostly

#if 0
	//options:
	enum {
		oCompressedPayload		= 0x0001,
		oMachoVersion			= 0x0002,
		oChannel				= 0x0004,
		oSequenceNumber			= 0x0008
		//joinExisting: bool
		//temporary: bool
	};
	uint32 included_options;
	uint8 macho_version;	//if oMachoVersion. this is used for cached  calls. If the value of this is a single 
                            // digit/entry, it is ignored, but if it is a list, with a timestamp and
                            //(something else, prolly version), then it is telling the server what cached
                            //version of this call the client has cached, so the server can return "cache
                            //OK" if possible
	std::string channel;
	uint32 sequence_number;
#endif
};

class PyCallStream {
public:
	PyCallStream();
	~PyCallStream();

	void Dump(LogType type, PyVisitor *dumper);
	bool Decode(const std::string &type, PyRepTuple *&payload);	//consumes substream
	PyRepTuple *Encode();
	PyCallStream *Clone() const;

	uint32 remoteObject;		//seen 1, hack: 0 means it was a string
	std::string remoteObjectStr;
	
	std::string method;
	PyRepTuple *arg_tuple;
	PyRepDict  *arg_dict;	//named parameters

#if 0
	//options:
	enum {
		oMachoVersion			= 0x0001
	};
	uint32 included_options;
	uint8 macho_version;	//if oMachoVersion

    //there is a flag in the first element of the inner tuple which seems to
    //tell their netcode to process the message where it is received, or forward
    //the call up to a higher layer... This seems to be set to true if the call is
    //to a bound object (remoteObject==0), so using that for now.
    //
    //there is also 'channel' info in the 2nd entry of the upper most dict but I
    //havent seen it used out side of authentication...
#endif
};

class EVENotificationStream {
public:
	EVENotificationStream();
	~EVENotificationStream();

	void Dump(LogType type, PyVisitor *dumper);
	bool Decode(const std::string &pkt_type, const std::string &notify_type, PyRepTuple *&payload);	//consumes substream
	PyRepTuple *Encode();
	EVENotificationStream *Clone() const;

	std::string notifyType;	//not encoded by Encode() since it is in the address part, mainly here for convenience.
	
	uint32 remoteObject;		//seen 1, hack: 0 means it was a string
	std::string remoteObjectStr;
	
	PyRepTuple *args;
};


#endif


/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
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
    Author:     Zhur
*/

#ifndef EVE_PY_PACKET_H
#define EVE_PY_PACKET_H

#include "network/packet_types.h"

class PyRep;
class PyTuple;
class PyDict;
class PyVisitor;

class PyAddress {
public:
    PyAddress();

    void Dump(FILE *into, const char *pfx) const;
    void Dump(LogType type, const char *pfx) const;
    bool Decode(PyRep *&object);    //consumes object
    PyRep *Encode();
    void operator=(const PyAddress &right);

    /*
    def __setstate__(self, state):
        if state[0] == const.ADDRESS_TYPE_CLIENT:
            self.addressType, self.clientID, self.callID, self.service = state
        elif state[0] == const.ADDRESS_TYPE_BROADCAST:
            self.addressType, self.broadcastID, self.narrowcast, self.idtype = state
        elif state[0] == const.ADDRESS_TYPE_ANY:
            self.addressType, self.service, self.callID = state
        else:
            self.addressType, self.nodeID, self.service, self.callID = state
    */
    typedef enum {
        Node    = 1,
            /*
             * [0]   addressType
             * [1]   nodeID
             * [2]   service
             * [3]   callID
             */
        Client  = 2,
            /* "because it's ID doesn't match"
             * [0]   addressType
             * [1]   clientID
             * [2]   callID
             * [3]   service
             */
        Broadcast = 4,
            /*
             * [0]   addressType
             * [1]   broadcastID
             * [2]   narrowcast         // this part is not understood yet... something about specific nodes for bcast types
             * [3]   idtype
             */
        Any     = 8,
            /*
             * [0]   addressType
             * [1]   service
             * [2]   callID
             */
        Invalid = 'I'   //not real
    } AddrType;

    AddrType type;
    int64 objectID;  //node id, client id, etc...
    int64 callID;

    std::string service;    //broadcastID for a broadcast.
    std::string bcast_idtype;

protected:
    bool _DecodeService(PyRep *rep);
    bool _DecodeCallID(PyRep *rep);
    bool _DecodeObjectID(PyRep *rep);
};

class PyPacket {
public:
    PyPacket();
    ~PyPacket();

    void Dump(LogType type, PyVisitor& dumper);
    bool Decode(PyRep **packet);    //consumes packet
    PyRep *Encode();
    PyPacket *Clone() const;

    //the "type" of object this represents
    std::string type_string;

    //the contents tuple:
    MACHONETMSG_TYPE    type;
    PyAddress   source;
    PyAddress   dest;
    uint32      userid;
    PyTuple  *payload;
    PyDict   *named_payload;

#if 0
    //options:
    enum {
        oCompressedPayload      = 0x0001,
        oMachoVersion           = 0x0002,
        oChannel                = 0x0004,
        oSequenceNumber         = 0x0008
        //joinExisting: bool
        //temporary: bool
    };
    uint32 included_options;
    uint8 macho_version;    //if oMachoVersion. this is used for cached  calls. If the value of this is a single
                            // digit/entry, it is ignored, but if it is a list, with a timestamp and
                            //(something else, probably version), then it is telling the server what cached
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

    void Dump(LogType type, PyVisitor& dumper);
    bool Decode(const std::string &type, PyTuple *&payload); //consumes substream
    PyTuple *Encode();
    PyCallStream *Clone() const;

    uint32 remoteObject;        //seen 1, hack: 0 means it was a string
    std::string remoteObjectStr;

    std::string method;
    PyTuple *arg_tuple;
    PyDict  *arg_dict;   //named parameters
};

class EVENotificationStream {
public:
    EVENotificationStream();
    ~EVENotificationStream();

    void Dump(LogType type, PyVisitor& dumper);
    bool Decode(const std::string &pkt_type, const std::string &notify_type, PyTuple *&payload); //consumes substream
    PyTuple *Encode();
    EVENotificationStream *Clone() const;

    std::string notifyType; //not encoded by Encode() since it is in the address part, mainly here for convenience.

    uint32 remoteObject;        //seen 1, hack: 0 means it was a string
    std::string remoteObjectStr;

    PyTuple *args;
};


#endif

/*{'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251801, 'label': u'ACCOUNTBANNED'}(u'Your connection to the server has been terminated because your account has been banned. Contact customer support for further information.', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251802, 'label': u'ACCOUNTNOTACTIVE'}(u'Your connection to the server has been terminated because your account is no longer active. Please check your subscription details.', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251803, 'label': u'ACL_ACCEPTDELAY'}(u'Starting up...({[numeric]seconds} sec.)', None, {u'{[numeric]seconds}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'seconds'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251804, 'label': u'ACL_CLUSTERFULL'}(u'Cluster full (limit={[numeric]limit})', None, {u'{[numeric]limit}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251809, 'label': u'ACL_IPADDRESSBAN'}(u'Your IP address ({address}) has been banned', None, {u'{address}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'address'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251810, 'label': u'ACL_IPADDRESSNOTALLOWED'}(u'Your IP address ({address}) has not been specifically allowed and the cluster is running in exclusive mode', None, {u'{address}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'address'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251811, 'label': u'ACL_NOTACCEPTING'}(u'The cluster is not currently accepting connections', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251812, 'label': u'ACL_PROXYFULL'}(u'Proxy full (limit={[numeric]limit})', None, {u'{[numeric]limit}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251813, 'label': u'ACL_PROXYNOTCONNECTED'}(u'Proxy not connected to sol servers', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251814, 'label': u'ACL_SHUTTINGDOWN'}(u'The cluster is shutting down @ {when}', None, {u'{when}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'when'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251815, 'label': u'CLIENTDISCONNECTED'}(u'Connection Closed - Client initiated disconnect', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251816, 'label': u'GAMETIMEEXPIRED'}(u'Connection Closed - Game Time Expired', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251817, 'label': u'HANDSHAKE_CORRUPT'}(u'Connection Fault - Handshake Corrupt', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251818, 'label': u'HANDSHAKE_FAILEDHASHMISMATCH'}(u'Handshake Failed - Incorrect Response to Server Challenge - Hash Mismatch', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251819, 'label': u'HANDSHAKE_FAILEDVERIFICATION'}(u'Connection Failure - Handshake Error', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251820, 'label': u'HANDSHAKE_FAILEDVIPKEY'}(u'Handshake Failed - VIP Key must match user name', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251821, 'label': u'HANDSHAKE_INCOMPATIBLEBUILD'}(u'Incompatible (Build)', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251822, 'label': u'HANDSHAKE_INCOMPATIBLEPROTOCOL'}(u'Incompatible (Protocol)', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251823, 'label': u'HANDSHAKE_INCOMPATIBLEPUBLICKEY'}(u'Incompatible (Public Key)', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251824, 'label': u'HANDSHAKE_INCOMPATIBLEREGION'}(u'Incompatible (Region)', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251825, 'label': u'HANDSHAKE_INCOMPATIBLERELEASE'}(u'Incompatible (Release)', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251826, 'label': u'HANDSHAKE_INCOMPATIBLEVERSION'}(u'Incompatible (Version)', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251827, 'label': u'HANDSHAKE_INCORRECTPADDING'}(u'Handshake Failed - Incorrect Handshake Padding - Length is {[numeric]current} but should be {[numeric]expected}', None, {u'{[numeric]expected}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'expected'}, u'{[numeric]current}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'current'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251828, 'label': u'HANDSHAKE_SERVERFAILURE'}(u'Handshake Failed - Server Failure', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251829, 'label': u'HANDSHAKE_TIMEOUT_AUTHENTICATED'}(u'Connection Timeout - The client stalled after authentication for an unreasonable amount of time', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251830, 'label': u'HANDSHAKE_TIMEOUT_CLIENTCOMPATIBILITYHANDSHAKE'}(u'Connection Timeout - The client did not respond to the compatibility-handshake within a reasonable amount of time', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251831, 'label': u'HANDSHAKE_TIMEOUT_CLIENTRESPONSETOSERVERCHALLENGE'}(u"Connection Timeout - The client did not respond to the server's secure challenge within a reasonable amount of time", None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251832, 'label': u'HANDSHAKE_TIMEOUT_CLIENTSECUREHANDSHAKE'}(u'Connection Timeout - The client did not initiate the secure-handshake within a reasonable amount of time', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251833, 'label': u'HANDSHAKE_TIMEOUT_FAILEDSERVERINITIATECOMPATIBILITYHANDSHAKE'}(u'Connection Timeout - The server did not initiate the compatibility-handshake within a reasonable amount of time', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251834, 'label': u'HANDSHAKE_TIMEOUT_SERVERDIDNOTACKNOWLEDGECHALLENGERESPONSE'}(u"Server didn't Acknowledge our Challenge-Response", None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251835, 'label': u'HANDSHAKE_TIMEOUT_SERVERKEYEXCHANGE'}(u"The server failed to respond to the client's queuing and key exchange request within a reasonable amount of time", None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251836, 'label': u'HANDSHAKE_TIMEOUT_SERVERRESPONSETOCLIENTCHALLENGE'}(u"Connection Timeout - The server failed to acknowledge the client's response to the server's challenge within a reasonable amount of time", None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251837, 'label': u'HANDSHAKE_TIMEOUT_SERVERSECUREHANDSHAKE'}(u"Connection Timeout - The server did not respond to the client's secure-handshake request within a reasonable amount of time", None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251838, 'label': u'HANDSHAKE_VERIFICATIONFAILURE'}(u'Handshake Failed - Incorrect Response to Server Challenge - Verification Failure - {verification}', None, {u'{verification}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'verification'}})
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251839, 'label': u'HANDSHAKE_VERSIONCHECKCOMPLETE'}(u'OK', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251840, 'label': u'KEEPALIVEEXPIRED'}(u"Connection Closed - The connection's keep-alive timer expired", None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251841, 'label': u'NODEDEATH'}(u'Connection Lost - A server process that you were using has gone offline', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251842, 'label': u'SOCKETCLOSED'}(u'Connection Lost - Socket Closed', None, None)
 * {'FullPath': u'/Carbon/MachoNet/TransportClosed', 'messageID': 251843, 'label': u'UNHANDLEDEXCEPTION'}(u'Connection Lost - An unexpected error has occurred', None, None)
 */
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
    Rewrite:    Allan
*/

/** @todo  update this to unique_ptr/shared_ptr if possible  shared_ptr<T> p(new Y); or make_unique<>() */

#include "eve-common.h"

#include "python/PyPacket.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyDumpVisitor.h"
#include "python/PyPacketValidator.h"

const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT] =
{
    "AUTHENTICATION_REQ",
    "AUTHENTICATION_RSP",
    "IDENTIFICATION_REQ",
    "IDENTIFICATION_RSP",
    "U4",
    "U5",
    "CALL_REQ",
    "CALL_RSP",
    "TRANSPORTCLOSED",
    "U9",
    "RESOLVE_REQ",
    "RESOLVE_RSP",
    "NOTIFICATION",
    "U13",
    "U14",
    "ERRORRESPONSE",
    "SESSIONCHANGENOTIFICATION",
    "U17",
    "SESSIONINITIALSTATENOTIFICATION",
    "U19",
    "PING_REQ",
    "PING_RSP"
};

PyPacket::PyPacket()
: type_string("none"),
type(__Fake_Invalid_Type),
userid(0),
payload(nullptr),
named_payload(nullptr),
contextKey(nullptr)
{
}

PyPacket::~PyPacket()
{
    PySafeDecRef(payload);
    PySafeDecRef(named_payload);
}

PyPacket *PyPacket::Clone() const
{
    PyPacket *res = new PyPacket();
        res->type_string = type_string;
        res->type = type;
        res->source = source;
        res->dest = dest;
        res->userid = userid;
        res->payload = payload->Clone()->AsTuple();
    if (named_payload == nullptr)
        res->named_payload = nullptr;
    else
        res->named_payload = named_payload->Clone()->AsDict();
    return res;
}

void PyPacket::Dump(LogType ltype, PyVisitor& dumper)
{
    _log(ltype, "Packet:");
    _log(ltype, "  Type: %s", type_string.c_str());
    _log(ltype, "  Command: %s (%d)", MACHONETMSG_TYPE_NAMES[type], type);
    _log(ltype, "  Source:");
    source.Dump(ltype, "    ");
    _log(ltype, "  Dest:");
    dest.Dump(ltype, "    ");
    _log(ltype, "  User ID: %u", userid);
    _log(ltype, "  Payload:");
    payload->visit( dumper );
    if (named_payload == nullptr) {
        _log(ltype, "  Named Payload: None (null)");
    } else {
        _log(ltype, "  Named Payload:");
        named_payload->visit( dumper );
    }
}

bool PyPacket::Decode(PyRep **in_packet)
{
    PyRep *packet = *in_packet; //assign
    *in_packet = nullptr;       //consume

    PySafeDecRef(payload);
    PySafeDecRef(named_payload);
    payload = nullptr;
    named_payload = nullptr;

    if (packet == nullptr) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet is null.");
        return false;
    }

    // Enhanced validation: Check packet integrity before processing
    if (!PyPacketValidator::ValidatePacketStructure(packet, "PyPacket::Decode()")) {
        PyDecRef(packet);
        return false;
    }

    if (packet->IsChecksumedStream()) {
        //TODO: check cs->checksum
        packet = packet->AsChecksumedStream()->stream();
    }
    //Dragon nuance... it gets wrapped again
    if (packet->IsSubStream()) {
        PySubStream* ss = packet->AsSubStream();
        ss->DecodeData();
        if (ss->decoded() == nullptr) {
            codelog(NET__PACKET_ERROR, "PyPacket::Decode() - unable to decode initial packet substream.");
            PyDecRef(packet);
            return false;
        }

        packet = ss->decoded();
    }

    if (!packet->IsObject()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet body is not PyObject: %s", packet->TypeString());
        PyDecRef(packet);
        return false;
    }

    type_string = packet->AsObject()->type()->content();
    if (!packet->AsObject()->arguments()->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - packet body does not contain a tuple");
        return false;
    }

    PyTuple *tuple = packet->AsObject()->arguments()->AsTuple();
    if (tuple == nullptr) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - tuple is null.");
        return false;
    }

    if (!PyPacketValidator::ValidateTupleSize(tuple, 7, "PyPacket::Decode() - main tuple") ||
        !PyPacketValidator::ValidateTupleElements(tuple, "PyPacket::Decode() - main tuple")) {
        PyDecRef(packet);
        return false;
    }

    if (!tuple->items[0]->IsInt()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - First main tuple element is not an integer");
        PyDecRef(packet);
        return false;
    }

    switch(PyRep::IntegerValue(tuple->items[0])) {
        case AUTHENTICATION_REQ:
        case AUTHENTICATION_RSP:
        case IDENTIFICATION_REQ:
        case IDENTIFICATION_RSP:
        case CALL_REQ:
        case CALL_RSP:
        case TRANSPORTCLOSED:
        case RESOLVE_REQ:
        case RESOLVE_RSP:
        case NOTIFICATION:
        case ERRORRESPONSE:
        case SESSIONCHANGENOTIFICATION:
        case SESSIONINITIALSTATENOTIFICATION:
        case PING_REQ:
        case PING_RSP: {
            type = (MACHONETMSG_TYPE) PyRep::IntegerValue(tuple->items[0]);
        } break;
        default: {
            codelog(NET__PACKET_ERROR, "PyPacket::Decode() - Unknown message type %i", PyRep::IntegerValue(tuple->items[0]));
            PyDecRef(packet);
            return false;
        } break;
    }

    //source address
    if (!source.Decode(tuple->items[1]))  {
        //error printed in decoder
        PyDecRef(packet);
        return false;
    }
    //dest address
    if (!dest.Decode(tuple->items[2])) {
        //error printed in decoder
        PyDecRef(packet);
        return false;
    }

    userid = PyRep::IntegerValue(tuple->items[3]);

    //payload
    if (!tuple->items[4]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - Fifth main tuple element is not a tuple");
        PyDecRef(packet);
        return false;
    }
    payload = tuple->items[4]->AsTuple();

    //options dict
    if (tuple->items[5]->IsNone()) {
        named_payload = nullptr;
    } else if (tuple->items[5]->IsDict()) {
        named_payload = tuple->items[5]->AsDict();
    } else {
        codelog(NET__PACKET_ERROR, "PyPacket::Decode() - Sixth main tuple element is neither dict or none.");
        PyDecRef(packet);
        return false;
    }

    PyDecRef(packet);
    return true;
}

PyRep *PyPacket::Encode() {
    PyTuple *arg_tuple = new PyTuple(7);
    //command
    arg_tuple->items[0] = new PyInt(type);
    //source
    arg_tuple->items[1] = source.Encode();
    //dest
    arg_tuple->items[2] = dest.Encode();
    //userid
    if (userid == 0)
        arg_tuple->items[3] = PyStatic.NewNone();
    else
        arg_tuple->items[3] = new PyInt(userid);

    //payload
    arg_tuple->items[4] = payload;     // dont clone here.  set actual rep in item, and it will be cleaned up by d'tor later

    //named arguments (OID+ or sn)
    if (named_payload == nullptr) {
        arg_tuple->items[5] = PyStatic.NewNone();
    } else {
        arg_tuple->items[5] = named_payload;    // dont clone here.  set actual rep in item, and it will be cleaned up by d'tor later
    }

    // contextKey, gives the client extra information on what exactly is going on
    // this is PyNone almost 100% of the time and seems to be used in the node<->proxy communication
    if (contextKey == nullptr) {
        arg_tuple->items[6] = PyStatic.NewNone();
    } else {
        arg_tuple->items[6] = contextKey;
    }

    return new PyObject( type_string.c_str(), arg_tuple );
}

PyAddress::PyAddress()
: type(Invalid),
objectID(0),
callID(0),
service("")
{
}

void PyAddress::Dump(FILE *into, const char *pfx) const {
    switch(type) {
        case Any:
            fprintf(into, "%sAny: service='%s' callID=%lli", pfx, service.c_str(), callID);
        break;
        case Node:
            fprintf(into, "%sNode: nodeID=%lli service='%s' callID=%lli", pfx, objectID, service.c_str(), callID);
        break;
        case Client:
            fprintf(into, "%sClient: clientID=%lli service='%s' callID=%lli", pfx, objectID, service.c_str(), callID);
        break;
        case Broadcast:
            fprintf(into, "%sBroadcast: broadcastID='%s' narrowcast=(not implemented) idtype='%s'", pfx, service.c_str(), bcast_idtype.c_str());
        break;
        case Invalid:
        break;
    //no default on purpose
    }
}

void PyAddress::Dump(LogType ltype, const char *pfx) const {
    switch(type) {
        case Any:
            _log(ltype, "%sAny: service='%s' callID=%lli", pfx, service.c_str(), callID);
        break;
        case Node:
            _log(ltype, "%sNode: nodeID=%lli service='%s' callID=%lli", pfx, objectID, service.c_str(), callID);
        break;
        case Client:
            _log(ltype, "%sClient: clientID=%lli callID=%lli service='%s'", pfx, objectID, callID, service.c_str());
        break;
        case Broadcast:
            _log(ltype, "%sBroadcast: broadcastID='%s' narrowcast=(not implemented) idtype='%s'", pfx, service.c_str(), bcast_idtype.c_str());
        break;
        case Invalid:
        break;
    //no default on purpose
    }
}

void PyAddress::operator=(const PyAddress &right) {
    type = right.type;
    objectID = right.objectID;
    callID = right.callID;
    service = right.service;
    bcast_idtype = right.bcast_idtype;
}

bool PyAddress::Decode(PyRep *&in_object) {
    PyRep *base = in_object;
    in_object = nullptr;

    if (base == nullptr) {
        codelog(NET__PACKET_ERROR, "PyAddress::Decode() - base is null.");
        return false;
    }

    if (!base->IsObject()) {
        codelog(NET__PACKET_ERROR, "Invalid element type, expected object but got %s", base->TypeString());
        PyDecRef(base);
        return false;
    }

    PyTuple *tuple = base->AsObject()->arguments()->AsTuple();
    if (tuple == nullptr) {
        codelog(NET__PACKET_ERROR, "PyAddress::Decode() - tuple is null.");
        PyDecRef(base);
        return false;
    }

    if (tuple->items.size() < 3) {
        codelog(NET__PACKET_ERROR, "PyAddress::Decode() - not enough elements in address tuple: %u", tuple->items.size());
        tuple->Dump(NET__PACKET_ERROR, "  ");
        PyDecRef(base);
        PyDecRef(tuple);
        return false;
    }

    // Enhanced validation: Check tuple elements exist
    if (!PyPacketValidator::ValidateTupleElements(tuple, "PyAddress::Decode() - address tuple")) {
        PyDecRef(base);
        PyDecRef(tuple);
        return false;
    }

    //decode the address type.
    if (!tuple->items[0]->IsInt()) {
        codelog(NET__PACKET_ERROR, "Wrong type on address element (0)");
        tuple->items[0]->Dump(NET__PACKET_ERROR, "  ");
        PyDecRef(base);
        PySafeDecRef(tuple);
        return false;
    }

    switch(PyRep::IntegerValue(tuple->items[0])) {
        case Any: {
            if (tuple->items.size() != 3) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Any address tuple: %llu", tuple->items.size());
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
            type = Any;

            if (!_DecodeService(tuple->items[1])
            or !_DecodeCallID(tuple->items[2])) {
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
        }  break;
        case Node: {
            if (tuple->items.size() != 4) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Node address tuple: %llu", tuple->items.size());
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
            type = Node;

            if (!_DecodeObjectID(tuple->items[1])
            or !_DecodeService(tuple->items[2])
            or !_DecodeCallID(tuple->items[3])) {
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
        }  break;
        case Client: {
            if (tuple->items.size() != 4) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Client address tuple: %llu", tuple->items.size());
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
            type = Client;

            if (!_DecodeObjectID(tuple->items[1])
            or !_DecodeCallID(tuple->items[2])
            or !_DecodeService(tuple->items[3])) {
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
        }  break;
        case Broadcast: {
            if (tuple->items.size() != 4) {
                codelog(NET__PACKET_ERROR, "Invalid number of elements in Broadcast address tuple: %llu", tuple->items.size());
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
            type = Broadcast;

            if (!tuple->items[1]->IsString()) {
                codelog(NET__PACKET_ERROR, "Invalid type %s for brodcastID", tuple->items[1]->TypeString());
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }
            if (!tuple->items[3]->IsString()) {
                codelog(NET__PACKET_ERROR, "Invalid type %s for idtype", tuple->items[3]->TypeString());
                PyDecRef(base);
                PySafeDecRef(tuple);
                return false;
            }

            service = PyRep::StringContent(tuple->items[1]);       //assign op
            bcast_idtype = PyRep::StringContent(tuple->items[3]);  //assign op

            //items[2] is either a list or a tuple.
            /*
            //PyList *nclist = (PyList *) tuple->items[2];
            if (!nclist->items.empty()) {
                printf("Not decoding narrowcast list:");
                nclist->Dump(NET__PACKET_ERROR, "     ");
            }*/
        }   break;
        default: {
            codelog(NET__PACKET_ERROR, "Unknown address type: %i", PyRep::IntegerValue(tuple->items[0]));
            PyDecRef(base);
            PySafeDecRef(tuple);
            return false;
        }
    }

    //PyDecRef(base);
    //PySafeDecRef(tuple);
    return true;
}

PyRep *PyAddress::Encode() {
    PyTuple *t(nullptr);
    switch(type) {
        case Any: {
            t = new PyTuple(3);
            t->items[0] = new PyInt((int)type);

            if (service.empty())
                t->items[1] = PyStatic.NewNone();
            else
                t->items[1] = new PyString(service.c_str());

            if (objectID == 0)
                t->items[2] = PyStatic.NewNone();
            else
                t->items[2] = new PyLong(objectID);
        } break;
        case Node: {
            t = new PyTuple(4);
            t->items[0] = new PyInt((int)type);
            t->items[1] = new PyLong(objectID);

            if (service.empty())
                t->items[2] = PyStatic.NewNone();
            else
                t->items[2] = new PyString(service.c_str());

            if (callID == 0)
                t->items[3] = PyStatic.NewNone();
            else
                t->items[3] = new PyLong(callID);
        } break;
        case Client: {
            t = new PyTuple(4);
            t->items[0] = new PyInt((int)type);
            t->items[1] = new PyLong(objectID);

            if (callID == 0)
                t->items[2] = PyStatic.NewNone();
            else
                t->items[2] = new PyLong(callID);

            if (service.empty())
                t->items[3] = PyStatic.NewNone();
            else
                t->items[3] = new PyString(service.c_str());
        } break;
        case Broadcast: {
            t = new PyTuple(4);
            t->items[0] = new PyInt((int)type);
            //broadcastID
            if (service.empty())
                t->items[1] = PyStatic.NewNone();
            else
                t->items[1] = new PyString(service.c_str());
            //narrowcast
            t->items[2] = new PyList(); // LSC uses tuple here, others None() or empty List()
            //typeID
            t->items[3] = new PyString(bcast_idtype.c_str());
        } break;
        case Invalid:
        default: {
            //this still needs to be something which will not crash us.
            t = new_tuple(PyStatic.NewNone());
        } break;
    }

    return new PyObject( "macho.MachoAddress", t );
}

bool PyAddress::_DecodeService(PyRep *rep) {
    service = PyRep::StringContent(rep);
    return true;
}

bool PyAddress::_DecodeCallID(PyRep *rep) {
    callID = PyRep::IntegerValue(rep);
    return true;
}

bool PyAddress::_DecodeObjectID(PyRep *rep) {
    objectID = PyRep::IntegerValue(rep);
    return true;
}


PyCallStream::PyCallStream()
: remoteObject(0),
  method(""),
  arg_tuple(nullptr),
  arg_dict(nullptr)
{
}

PyCallStream::~PyCallStream() {
    PySafeDecRef(arg_tuple);
    //PySafeDecRef(arg_dict);
}

PyCallStream *PyCallStream::Clone() const {
    PyCallStream *res = new PyCallStream();
    res->remoteObject = remoteObject;
    res->remoteObjectStr = remoteObjectStr;
    res->method = method;
    res->arg_tuple = arg_tuple->Clone()->AsTuple();
    if (arg_dict == nullptr)
        res->arg_dict = nullptr;
    else
        res->arg_dict = arg_dict->Clone()->AsDict();

    return res;
}

void PyCallStream::Dump(LogType type, PyVisitor& dumper)
{
    _log(type, "Call Stream:");
    if (remoteObject == 0)
        _log(type, "  Remote Object: '%s'", remoteObjectStr.c_str());
    else
        _log(type, "  Remote Object: %u", remoteObject);
    _log(type, "  Method: %s", method.c_str());
    _log(type, "  Arguments:");
    arg_tuple->visit( dumper );
    if (arg_dict == nullptr) {
        _log(type, "  Named Arguments: None");
    } else {
        _log(type, "  Named Arguments:");
        arg_dict->visit( dumper );
    }
}

bool PyCallStream::Decode(const std::string &type, PyTuple *&in_payload) {
    PyTuple *payload = in_payload;   //copy
    in_payload = nullptr;            //consume

    PySafeDecRef(arg_tuple);
    PySafeDecRef(arg_dict);
    arg_tuple = nullptr;
    arg_dict = nullptr;

    if (payload == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - payload is null.");
        return false;
    }

    if (type != "macho.CallReq") {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - packet payload has unknown string type '%s'", type.c_str());
        PyDecRef(payload);
        return false;
    }

    if (payload->items.size() != 1) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - invalid tuple length %llu", payload->items.size());
        PyDecRef(payload);
        return false;
    }
    if (!payload->items[0]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - non tuple payload[0]");
        PyDecRef(payload);
        return false;
    }

    PyTuple *payload2 = payload->items[0]->AsTuple();
    if (payload2 == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - payload2 is null.");
        PyDecRef(payload);
        PySafeDecRef(payload2);
        return false;
    }

    if (!PyPacketValidator::ValidateTupleSize(payload2, 2, "PyCallStream::Decode() - payload2") ||
        !PyPacketValidator::ValidateTupleElements(payload2, "PyCallStream::Decode() - payload2")) {
        PyDecRef(payload);
        PySafeDecRef(payload2);
        return false;
    }

    //decode inner payload tuple
    //ignore tuple 0, it should be an int, dont know what it is
    if (!payload2->items[1]->IsSubStream()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - non-substream type");
        PyDecRef(payload);
        PySafeDecRef(payload2);
        return false;
    }

    PySubStream *ss = payload2->items[1]->AsSubStream();
    if (ss == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - ss is null.");
        PyDecRef(payload);
        PySafeDecRef(payload2);
        return false;
    }

    ss->DecodeData();
    if (ss->decoded() == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - Unable to decode call stream");
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(payload2);
        return false;
    }

    if (!ss->decoded()->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - packet body does not contain a tuple");
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(payload2);
        return false;
    }

    PyTuple *maint = ss->decoded()->AsTuple();
    if (maint == nullptr) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - maint is null.");
        return false;
    }
    if (!PyPacketValidator::ValidateTupleSize(maint, 4, "PyCallStream::Decode() - maint tuple") ||
        !PyPacketValidator::ValidateTupleElements(maint, "PyCallStream::Decode() - maint tuple")) {
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(maint);
        PySafeDecRef(payload2);
        return false;
    }

    //parse first tuple element, unknown
    if (maint->items[0]->IsInt()) {
        remoteObject = PyRep::IntegerValue(maint->items[0]);
        remoteObjectStr = "";
    } else if (maint->items[0]->IsString()) {
        remoteObject = 0;
        remoteObjectStr = PyRep::StringContent(maint->items[0]);
    } else {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - maint->items[0] has invalid type %s", maint->items[0]->TypeString());
        codelog(NET__PACKET_ERROR, " in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(maint);
        PySafeDecRef(payload2);
        return false;
    }

    //parse tuple[1]: method name
    if (maint->items[1]->IsString()) {
        method = PyRep::StringContent(maint->items[1]);
    } else {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - maint->items[1] has non-string type");
        maint->items[1]->Dump(NET__PACKET_ERROR, " --> ");
        codelog(NET__PACKET_ERROR, " in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(maint);
        PySafeDecRef(payload2);
        return false;
    }

    //grab argument list.
    if (!maint->items[2]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - argument list has non-tuple type");
        maint->items[2]->Dump(NET__PACKET_ERROR, " --> ");
        codelog(NET__PACKET_ERROR, "in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(maint);
        PySafeDecRef(payload2);
        return false;
    }
    arg_tuple = maint->items[2]->AsTuple();

    //options dict
    if (maint->items[3]->IsNone()) {
        arg_dict = nullptr;
    } else if (maint->items[3]->IsDict()) {
        arg_dict = maint->items[3]->AsDict();
    } else {
        codelog(NET__PACKET_ERROR, "PyCallStream::Decode() - tuple[3] has non-dict type");
        maint->items[3]->Dump(NET__PACKET_ERROR, " --> ");
        codelog(NET__PACKET_ERROR, "in:");
        payload->Dump(NET__PACKET_ERROR, "    ");
        PyDecRef(payload);
        PySafeDecRef(ss);
        PySafeDecRef(maint);
        PySafeDecRef(payload2);
        return false;
    }

    //PyDecRef(payload);
    //PySafeDecRef(ss);
    //PySafeDecRef(maint);
    //PySafeDecRef(payload2);
    return true;
}

PyTuple *PyCallStream::Encode() {
    PyTuple *res_tuple = new PyTuple(4);

    //remoteObject
    if (remoteObject == 0)
        res_tuple->items[0] = new PyString(remoteObjectStr.c_str());
    else
        res_tuple->items[0] = new PyInt(remoteObject);

    //method name
    res_tuple->items[1] = new PyString(method.c_str());

    //args
    res_tuple->items[2] = arg_tuple;     // no need to clone here.  set actual rep in item, and it will be cleaned up by d'tor later

    //options
    if (arg_dict == nullptr)
        res_tuple->items[3] = PyStatic.NewNone();
    else
        res_tuple->items[3] = arg_dict;     // no need to clone here.  set actual rep in item, and it will be cleaned up by d'tor later

    //now that we have the main arg tuple, build the unknown stuff around it...
    PyTuple *it2 = new PyTuple(2);
        it2->items[0] = new PyInt(remoteObject==0?1:0); /* some sort of flag, "process here or call UP"....*/
        it2->items[1] = new PySubStream(res_tuple);
    PyTuple *it1 = new PyTuple(2);
        it1->items[0] = it2;
        it1->items[1] = PyStatic.NewNone();    //this is the "channel" dict if populated.
    return it1;
}

EVENotificationStream::EVENotificationStream()
: notifyType("NO TYPE SET"),
  remoteObject(0),
  args(nullptr)
{
}

EVENotificationStream::~EVENotificationStream() {
    PySafeDecRef(args);
}

EVENotificationStream *EVENotificationStream::Clone() const {
    EVENotificationStream *res = new EVENotificationStream();
    res->args = args->Clone()->AsTuple();
    return res;
}

void EVENotificationStream::Dump(LogType type, PyVisitor& dumper)
{
    _log(type, "Notification: %s", notifyType.c_str());
    if (remoteObject == 0)
        _log(type, "  Remote Object: %s", remoteObjectStr.c_str());
    else
        _log(type, "  Remote Object: %u", remoteObject);

    _log(type, "  Arguments:");
    args->visit( dumper );
}

bool EVENotificationStream::Decode(const std::string &pkt_type, const std::string &notify_type, PyTuple *&in_payload) {
    PyTuple *payload = in_payload;      //copy
    in_payload = nullptr;               //consume

    PySafeDecRef(args);
    args = nullptr;

    if (payload == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - payload is null.");
        return false;
    }

    if (pkt_type != "macho.Notification") {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - notification payload has unknown string type %s", pkt_type.c_str());
        PyDecRef(payload);
        return false;
    }

    //decode payload tuple
    if (payload->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - invalid tuple length %llu", payload->items.size());
        PyDecRef(payload);
        return false;
    }
    if (!payload->items[0]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - non-tuple payload[0]");
        PyDecRef(payload);
        return false;
    }
    PyTuple *payload2 = payload->items[0]->AsTuple();
    if (payload2 == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - payload2 is null.");
        PyDecRef(payload);
        return false;
    }

    if (payload2->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - invalid tuple2 length %llu", payload2->items.size());
        PyDecRef(payload);
        PyDecRef(payload2);
        return false;
    }

    //decode inner payload tuple
    //ignore tuple 0, it should be an int, dont know what it is
    if (!payload2->items[1]->IsSubStream()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - non-substream type");
        PyDecRef(payload);
        PyDecRef(payload2);
        return false;
    }

    PySubStream *ss(payload2->items[1]->AsSubStream());
    if (ss == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - ss is null.");
        PyDecRef(payload);
        PyDecRef(payload2);
        return false;
    }
    ss->DecodeData();
    if (ss->decoded() == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - Unable to decode call stream");
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        return false;
    }

    if (!ss->decoded()->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - packet body does not contain a tuple");
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        return false;
    }

    PyTuple *robjt = ss->decoded()->AsTuple();
    if (robjt == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - robjt is null.");
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        return false;
    }
    if (robjt->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - packet body has %llu elements, expected %d", robjt->items.size(), 2);
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(robjt);
        return false;
    }

    //parse first tuple element, remote object
    if (robjt->items[0]->IsInt()) {
        remoteObject = PyRep::IntegerValue(robjt->items[0]);
        remoteObjectStr = "";
    } else if (robjt->items[0]->IsString()) {
        remoteObject = 0;
        remoteObjectStr = PyRep::StringContent(robjt->items[0]);
    } else {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - main tuple[0] has invalid type %s", robjt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " in:");
        payload->Dump( NET__PACKET_ERROR, "" );
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(robjt);
        return false;
    }

    if (!robjt->items[1]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - main tuple[1] has non-tuple type %s", robjt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " it is:");
        payload->Dump( NET__PACKET_ERROR, "" );
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(robjt);
        return false;
    }

    PyTuple *subt(robjt->items[1]->AsTuple());
    if (subt == nullptr) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - subt is null.");
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(robjt);
        return false;
    }
    if (subt->items.size() != 2) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - packet body has %llu elements, expected %d", subt->items.size(), 2);
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(subt);
        PySafeDecRef(robjt);
        return false;
    }

    //parse first tuple element, remote object
    if (subt->items[0]->IsInt()) {
        //PyInt *tuple0 = (PyInt *) maint->items[0];
        //no idea what this is.
    } else {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - sub tuple[0] has invalid type %s", subt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " in:");
        payload->Dump( NET__PACKET_ERROR, "" );
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(subt);
        PySafeDecRef(robjt);
        return false;
    }

    if (!subt->items[1]->IsTuple()) {
        codelog(NET__PACKET_ERROR, "EVENotificationStream::Decode() - subt tuple[1] has non-tuple type %s", robjt->items[0]->TypeString());
        _log(NET__PACKET_ERROR, " it is:");
        payload->Dump( NET__PACKET_ERROR, "" );
        PyDecRef(ss);
        PyDecRef(payload);
        PyDecRef(payload2);
        PySafeDecRef(subt);
        PySafeDecRef(robjt);
        return false;
    }

    args = subt->items[1]->AsTuple();
    notifyType = notify_type;

    PyDecRef(ss);
    PyDecRef(payload);
    PyDecRef(payload2);
    PySafeDecRef(subt);
    PySafeDecRef(robjt);
    return true;
}

PyTuple *EVENotificationStream::Encode() {
    PyTuple *t4 = new PyTuple(2);
        t4->SetItem(0, PyStatic.NewOne());
        t4->SetItem(1, args);       // no need to clone here.  set actual rep in item, and it will be cleaned up by d'tor later
    PyTuple *t3 = new PyTuple(2);
        t3->SetItem(0, new PyInt(0));
        t3->SetItem(1, t4);
    PyTuple *t2 = new PyTuple(2);
        t2->SetItem(0, new PyInt(0));
        t2->SetItem(1, new PySubStream(t3));
    PyTuple *t1 = new PyTuple(1);
        t1->SetItem(0, t2);
    return t1;
}

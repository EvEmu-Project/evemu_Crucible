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


#include "common.h"
#include "PyPacket.h"
#include "PyRep.h"
#include "packet_dump.h"
#include "packet_functions.h"
#include "logsys.h"
#include "PyDumpVisitor.h"
#include <string>

const char *MACHONETMSG_TYPE_str[_MACHONETMSG_TYPE_next]
= {
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
  payload(NULL),
  named_payload(NULL)
{
}

PyPacket::~PyPacket() {
	delete payload;
	delete named_payload;
}

PyPacket *PyPacket::Clone() const {
	PyPacket *res = new PyPacket();
	res->type_string = type_string;
	res->type = type;
	res->source = source;
	res->dest = dest;
	res->userid = userid;
	res->payload = (PyRepTuple *) payload->Clone();
	if(named_payload == NULL) {
		res->named_payload = NULL;
	} else {
		res->named_payload = (PyRepDict *) named_payload->Clone();
	}
	return(res);
}

void PyPacket::Dump(LogType ltype, PyVisitor *dumper) {
	_log(ltype, "Packet:");
	_log(ltype, "  Type: %s", type_string.c_str());
	_log(ltype, "  Command: %s (%d)", MACHONETMSG_TYPE_str[type], type);
	_log(ltype, "  Source:");
	source.Dump(ltype, "    ");
	_log(ltype, "  Dest:");
	dest.Dump(ltype, "    ");
	_log(ltype, "  User ID: %lu", userid);
	_log(ltype, "  Payload:");
	payload->visit(dumper);
	if(named_payload == NULL) {
		_log(ltype, "  Named Payload: None");
	} else {
		_log(ltype, "  Named Payload:");
		named_payload->visit(dumper);
	}
}

bool PyPacket::Decode(PyRep *&in_packet) {
	PyRep *packet = in_packet;	//consume
	in_packet = NULL;
	
	delete payload;
	delete named_payload;
	payload = NULL;
	named_payload = NULL;

	if(packet->CheckType(PyRep::ChecksumedStream)) {
		PyRepChecksumedStream *cs = (PyRepChecksumedStream *) packet;
		//TODO: check cs->checksum
		packet = cs->stream;
		cs->stream = NULL;
		delete cs;
	}

	//Dragon nuance... it gets wrapped again
	if(packet->CheckType(PyRep::SubStream)) {
		PyRepSubStream *cs = (PyRepSubStream *) packet;
		cs->DecodeData();
		if(cs->decoded == NULL) {
			codelog(NET__PACKET_ERROR, "failed: unable to decode initial packet substream.");
			delete packet;
			return(false);
		}
		packet = cs->decoded;
		cs->decoded = NULL;
		delete cs;
	}

	if(!packet->CheckType(PyRep::Object)) {
		codelog(NET__PACKET_ERROR, "failed: packet body is not an 'Object': %s", packet->TypeString());
		delete packet;
		return(false);
	}

	PyRepObject *packeto = (PyRepObject *) packet;
	type_string = packeto->type;

	if(!packeto->arguments->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "failed: packet body does not contain a tuple");
		delete packet;
		return(false);
	}
	
	PyRepTuple *tuple = (PyRepTuple *) packeto->arguments;

	if(tuple->items.size() != 6) {
		codelog(NET__PACKET_ERROR, "failed: packet body does not contain a tuple of length 6");
		delete packet;
		return(false);
	}

	if(!tuple->items[0]->CheckType(PyRep::Integer)) {
		codelog(NET__PACKET_ERROR, "failed: First main tuple element is not an integer");
		delete packet;
		return(false);
	}
	PyRepInteger *typer = (PyRepInteger *) tuple->items[0];
	switch(typer->value) {
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
	case PING_RSP:
		type = (MACHONETMSG_TYPE) typer->value;
		break;
	default:
		codelog(NET__PACKET_ERROR, "failed: Unknown message type %lld", typer->value);
		delete packet;
		return(false);
		break;
	}

	//source address
	if(!source.Decode(tuple->items[1])) {
		//error printed in decoder
		delete packet;
		return(false);
	}
	//dest address
	if(!dest.Decode(tuple->items[2])) {
		//error printed in decoder
		delete packet;
		return(false);
	}

	if(tuple->items[3]->CheckType(PyRep::Integer)) {
		PyRepInteger *i = (PyRepInteger *) tuple->items[3];
		userid = i->value;
	} else if(tuple->items[3]->CheckType(PyRep::None)) {
		userid = 0;
	} else {
		codelog(NET__PACKET_ERROR, "failed: User ID has invalid type");
		delete packet;
		return(false);
	}

	//payload
	if(!tuple->items[4]->CheckType(PyRep::Buffer, PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "failed: Fifth main tuple element is not a tuple");
		delete packet;
		return(false);
	}
	payload = (PyRepTuple *) tuple->items[4];
	tuple->items[4] = NULL;	//we keep this one


	//options dict
	if(tuple->items[5]->CheckType(PyRep::None)) {
		named_payload = NULL;
	} else if(tuple->items[5]->CheckType(PyRep::Dict)) {
		named_payload = (PyRepDict *) tuple->items[5];
		tuple->items[5] = NULL;	//we keep this too.
	} else {
		codelog(NET__PACKET_ERROR, "failed: Sixth main tuple element is not a dict");
		delete packet;
		return(false);
	}

	delete packet;
	
	return(true);
}



PyRep *PyPacket::Encode() {
	PyRepTuple *arg_tuple = new PyRepTuple(6);
	
	//command
	arg_tuple->items[0] = new PyRepInteger(type);
	
	//source
	arg_tuple->items[1] = source.Encode();
	
	//dest
	arg_tuple->items[2] = dest.Encode();
	
	//unknown3
	if(userid == 0)
		arg_tuple->items[3] = new PyRepNone();
	else
		arg_tuple->items[3] = new PyRepInteger(userid);
	
	//payload
	//TODO: we dont really need to clone this if we can figure out a way to say "this is read only"
	//or if we can change this encode method to consume the PyPacket (which will almost always be the case)
	arg_tuple->items[4] = payload->Clone();
	
	//named arguments
	if(named_payload == NULL) {
		arg_tuple->items[5] = new PyRepNone();
	} else {
		arg_tuple->items[5] = named_payload->Clone();
	}
	
	return(new PyRepObject(type_string, arg_tuple));
}





PyAddress::PyAddress()
: type(Invalid),
  typeID(0),
  callID(0),
  service("")
{
}

void PyAddress::Dump(FILE *into, const char *pfx) const {
	switch(type) {
	case Any:
		fprintf(into, "%sAny: service='%s' callID=%lld", pfx, service.c_str(), callID);
		break;
	case Node:
		fprintf(into, "%sNode: node=0x%llx service='%s' callID=%lld", pfx, typeID, service.c_str(), callID);
		break;
	case Client:
		fprintf(into, "%sClient: node=0x%llx service='%s' callID=%lld", pfx, typeID, service.c_str(), callID);
		break;
	case Broadcast:
		fprintf(into, "%sBroadcast: broadcastID='%s' narrowcast=(not decoded yet) idtype='%s'", pfx, service.c_str(), bcast_idtype.c_str());
		break;
	case Invalid:
		break;
	//no default on purpose
	}
}

void PyAddress::Dump(LogType ltype, const char *pfx) const {
	switch(type) {
	case Any:
		_log(ltype, "%sAny: service='%s' callID=%lld", pfx, service.c_str(), callID);
		break;
	case Node:
		_log(ltype, "%sNode: node=0x%llx service='%s' callID=%lld", pfx, typeID, service.c_str(), callID);
		break;
	case Client:
		_log(ltype, "%sClient: node=0x%llx service='%s' callID=%lld", pfx, typeID, service.c_str(), callID);
		break;
	case Broadcast:
		_log(ltype, "%sBroadcast: broadcastID='%s' narrowcast=(not decoded yet) idtype='%s'", pfx, service.c_str(), bcast_idtype.c_str());
		break;
	case Invalid:
		break;
	//no default on purpose
	}
}

void PyAddress::operator=(const PyAddress &right) {
	type = right.type;
	typeID = right.typeID;
	callID = right.callID;
	service = right.service;
	bcast_idtype = right.bcast_idtype;
}

bool PyAddress::Decode(PyRep *&in_object) {
	PyRep *base = in_object;
	in_object = NULL;

	if(!base->CheckType(PyRep::Object)) {
		codelog(NET__PACKET_ERROR, "Invalid element type, expected object");
		delete base;
		return(false);
	}

	PyRepObject *obj = (PyRepObject *) base;
	//do we care about the object type? should be "macho.MachoAddress"

	if(!obj->arguments->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "Invalid argument type, expected tuple");
		delete base;
		return(false);
	}
	
	PyRepTuple *args = (PyRepTuple *) obj->arguments;
	if(args->items.size() < 3) {
		codelog(NET__PACKET_ERROR, "Not enough elements in address tuple: %d", args->items.size());
		args->Dump(NET__PACKET_ERROR, "  ");
		delete base;
		return(false);
	}

	//decode the address type.
	if(!args->items[0]->CheckType(PyRep::String)) {
		codelog(NET__PACKET_ERROR, "Wrong type on address type element (0)");
		args->items[0]->Dump(NET__PACKET_ERROR, "  ");
		delete base;
		return(false);
	}
	PyRepString *types = (PyRepString *) args->items[0];
	if(types->value.empty()) {
		codelog(NET__PACKET_ERROR, "Empty string for address type element (0)");
		delete base;
		return(false);
	}
	switch(types->value[0]) {
	case Any: {
		if(args->items.size() != 3) {
			codelog(NET__PACKET_ERROR, "Invalid number of elements in Any address tuple: %d", args->items.size());
			delete base;
			return(false);
		}
		type = Any;
		
		if(!_DecodeService(args->items[1])
		|| !_DecodeCallID(args->items[2])) {
			delete base;
			return(false);
		}
		
		break;
	}
	case Node: {
		if(args->items.size() != 4) {
			codelog(NET__PACKET_ERROR, "Invalid number of elements in Node address tuple: %d", args->items.size());
			delete base;
			return(false);
		}
		type = Node;
		
		if(!_DecodeTypeID(args->items[1])
		|| !_DecodeService(args->items[2])
		|| !_DecodeCallID(args->items[3])) {
			delete base;
			return(false);
		}
		break;
	}
	case Client: {
		if(args->items.size() != 4) {
			codelog(NET__PACKET_ERROR, "Invalid number of elements in Client address tuple: %d", args->items.size());
			delete base;
			return(false);
		}
		type = Client;

		if(!_DecodeTypeID(args->items[1])
		|| !_DecodeCallID(args->items[2])
		|| !_DecodeService(args->items[3])) {
			delete base;
			return(false);
		}
		
		break;
	}
	case Broadcast: {
		if(args->items.size() != 4) {
			codelog(NET__PACKET_ERROR, "Invalid number of elements in Broadcast address tuple: %d", args->items.size());
			delete base;
			return(false);
		}
		type = Broadcast;

		if(!args->items[1]->CheckType(PyRep::String)) {
			codelog(NET__PACKET_ERROR, "Invalid type %s for brodcastID", args->items[1]->TypeString());
			delete base;
			return(false);
		} else if(!args->items[3]->CheckType(PyRep::String)) {
			codelog(NET__PACKET_ERROR, "Invalid type %s for idtype", args->items[3]->TypeString());
			delete base;
			return(false);
		}

		PyRepString *bid = (PyRepString *) args->items[1];
		PyRepString *idt = (PyRepString *) args->items[3];

		service = bid->value;
		bcast_idtype = idt->value;

		//items[2] is either a list or a tuple.
		/*
		//PyRepList *nclist = (PyRepList *) args->items[2];
		if(!nclist->items.empty()) {
			printf("Not decoding narrowcast list:");
			nclist->Dump(NET__PACKET_ERROR, "     ");
		}*/
		
		break;
	}
	default:
		codelog(NET__PACKET_ERROR, "Unknown address type: %c", types->value[0]);
		delete base;
		return(false);
	}

	delete base;
	return(true);
}

PyRep *PyAddress::Encode() {
	PyRepObject *r = new PyRepObject();
	r->type = "macho.MachoAddress";
	
	PyRepTuple *t;
	switch(type) {
	case Any:
		t = new PyRepTuple(3);
		t->items[0] = new PyRepString("A");

		if(service == "")
			t->items[1] = new PyRepNone();
		else
			t->items[1] = new PyRepString(service.c_str());
		
		if(typeID == 0)
			t->items[2] = new PyRepNone();
		else
			t->items[2] = new PyRepInteger(typeID);
		
		r->arguments = t;
		break;
	case Node:
		t = new PyRepTuple(4);
		t->items[0] = new PyRepString("N");
		t->items[1] = new PyRepInteger(typeID);
		
		if(service == "")
			t->items[2] = new PyRepNone();
		else
			t->items[2] = new PyRepString(service.c_str());
		
		if(callID == 0)
			t->items[3] = new PyRepNone();
		else
			t->items[3] = new PyRepInteger(callID);
		
		r->arguments = t;
		break;
	case Client:
		t = new PyRepTuple(4);
		t->items[0] = new PyRepString("C");
		t->items[1] = new PyRepInteger(typeID);
		t->items[2] = new PyRepInteger(callID);
		if(service == "")
			t->items[3] = new PyRepNone();
		else
			t->items[3] = new PyRepString(service.c_str());
		r->arguments = t;
		break;
	case Broadcast:
		t = new PyRepTuple(4);
		t->items[0] = new PyRepString("B");
		//broadcastID
		if(service == "")
			t->items[1] = new PyRepNone();
		else
			t->items[1] = new PyRepString(service.c_str());
		//narrowcast
		t->items[2] = new PyRepList();
		//typeID
		t->items[3] = new PyRepString(bcast_idtype.c_str());
		r->arguments = t;
		break;
	case Invalid:
		//this still needs to be something which will not crash us.
		r->arguments = new PyRepNone();
		break;
	}
	return(r);
}

bool PyAddress::_DecodeService(PyRep *rep) {
	if(rep->CheckType(PyRep::String)) {
		PyRepString *s = (PyRepString *) rep;
		service = s->value;
	} else if(rep->CheckType(PyRep::None)) {
		service = "";
	} else {
		codelog(NET__PACKET_ERROR, "Wrong type on service field");
		rep->Dump(NET__PACKET_ERROR, "  ");
		return(false);
	}
	return(true);
}

bool PyAddress::_DecodeCallID(PyRep *rep) {
	if(rep->CheckType(PyRep::Integer)) {
		PyRepInteger *s = (PyRepInteger *) rep;
		callID = s->value;
	} else if(rep->CheckType(PyRep::None)) {
		callID = 0;
	} else {
		codelog(NET__PACKET_ERROR, "Wrong type on callID field");
		rep->Dump(NET__PACKET_ERROR, "  ");
		return(false);
	}
	return(true);
}

bool PyAddress::_DecodeTypeID(PyRep *rep) {
	if(rep->CheckType(PyRep::Integer)) {
		PyRepInteger *s = (PyRepInteger *) rep;
		typeID = s->value;
	} else if(rep->CheckType(PyRep::None)) {
		typeID = 0;
	} else {
		codelog(NET__PACKET_ERROR, "Wrong type on typed ID field");
		rep->Dump(NET__PACKET_ERROR, "  ");
		return(false);
	}
	return(true);
}


PyCallStream::PyCallStream()
: remoteObject(0),
  method(""),
  arg_tuple(NULL),
  arg_dict(NULL)
{
}

PyCallStream::~PyCallStream() {
	delete arg_tuple;
	delete arg_dict;
}

PyCallStream *PyCallStream::Clone() const {
	PyCallStream *res = new PyCallStream();
	res->remoteObject = remoteObject;
	res->remoteObjectStr = remoteObjectStr;
	res->method = method;
	res->arg_tuple = arg_tuple->TypedClone();
	if(arg_dict == NULL) {
		res->arg_dict = NULL;
	} else {
		res->arg_dict = arg_dict->TypedClone();
	}
	return(res);
}

void PyCallStream::Dump(LogType type, PyVisitor *dumper) {
	_log(type, "Call Stream:");
	if(remoteObject == 0)
		_log(type, "  Remote Object: '%s'", remoteObjectStr.c_str());
	else
		_log(type, "  Remote Object: %d", remoteObject);
	_log(type, "  Method: %s", method.c_str());
	_log(type, "  Arguments:");
	arg_tuple->visit(dumper);
	if(arg_dict == NULL) {
		_log(type, "  Named Arguments: None");
	} else {
		_log(type, "  Named Arguments:");
		arg_dict->visit(dumper);
	}
}

bool PyCallStream::Decode(const std::string &type, PyRepTuple *&in_payload) {
	PyRepTuple *payload = in_payload;	//consume
	in_payload = NULL;
	
	delete arg_tuple;
	delete arg_dict;
	arg_tuple = NULL;
	arg_dict = NULL;
	
	if(type != "macho.CallReq") {
		codelog(NET__PACKET_ERROR, "failed: packet payload has unknown string type '%s'", type.c_str());
		delete payload;
		return(false);
	}

	//decode payload tuple
	if(payload->items.size() != 2) {
		codelog(NET__PACKET_ERROR, "invalid tuple length %d", payload->items.size());
		delete payload;
		return(false);
	}
	if(!payload->items[0]->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "non-tuple payload[0]");
		delete payload;
		return(false);
	}
	PyRepTuple *payload2 = (PyRepTuple *) payload->items[0];
	if(payload2->items.size() != 2) {
		codelog(NET__PACKET_ERROR, "invalid tuple2 length %d", payload2->items.size());
		delete payload;
		return(false);
	}

	//decode inner payload tuple
	//ignore tuple 0, it should be an int, dont know what it is
	if(!payload2->items[1]->CheckType(PyRep::SubStream)) {
		codelog(NET__PACKET_ERROR, "non-substream type");
		delete payload;
		return(false);
	}
	PyRepSubStream *ss = (PyRepSubStream *) payload2->items[1];
	
	ss->DecodeData();
	if(ss->decoded == NULL) {
		codelog(NET__PACKET_ERROR, "Unable to decode call stream");
		delete payload;
		return(false);
	}
	
	if(!ss->decoded->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "packet body does not contain a tuple");
		delete payload;
		return(false);
	}

	PyRepTuple *maint = (PyRepTuple *) ss->decoded;
	if(maint->items.size() != 4) {
		codelog(NET__PACKET_ERROR, "packet body has %d elements, expected %d", maint->items.size(), 4);
		delete payload;
		return(false);
	}

	//parse first tuple element, unknown
	if(maint->items[0]->CheckType(PyRep::Integer)) {
		PyRepInteger *tuple0 = (PyRepInteger *) maint->items[0];
		remoteObject = tuple0->value;
		remoteObjectStr = "";
	} else if(maint->items[0]->CheckType(PyRep::String)) {
		PyRepString *tuple0 = (PyRepString *) maint->items[0];
		remoteObject = 0;
		remoteObjectStr = tuple0->value;
	} else {
		codelog(NET__PACKET_ERROR, "tuple[0] has invalid type %s", maint->items[0]->TypeString());
		codelog(NET__PACKET_ERROR, " in:");
		payload->Dump(NET__PACKET_ERROR, "    ");
		delete payload;
		return(false);
	}

	//parse tuple[1]: method name
	if(maint->items[1]->CheckType(PyRep::String)) {
		PyRepString *i = (PyRepString *) maint->items[1];
		method = i->value;
	} else {
		codelog(NET__PACKET_ERROR, "tuple[1] has non-string type");
		maint->items[1]->Dump(NET__PACKET_ERROR, " --> ");
		codelog(NET__PACKET_ERROR, " in:");
		payload->Dump(NET__PACKET_ERROR, "    ");
		delete payload;
		return(false);
	}

	//grab argument list.
	if(!maint->items[2]->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "argument list has non-tuple type");
		maint->items[2]->Dump(NET__PACKET_ERROR, " --> ");
		codelog(NET__PACKET_ERROR, "in:");
		payload->Dump(NET__PACKET_ERROR, "    ");
		delete payload;
		return(false);
	}
	arg_tuple = (PyRepTuple *) maint->items[2];
	maint->items[2] = NULL;	//we keep this one
	
	//options dict
	if(maint->items[3]->CheckType(PyRep::None)) {
		arg_dict = NULL;
	} else if(maint->items[3]->CheckType(PyRep::Dict)) {
		arg_dict = (PyRepDict *) maint->items[3];
		maint->items[3] = NULL;	//we keep this too.
	} else {
		codelog(NET__PACKET_ERROR, "tuple[3] has non-dict type");
		maint->items[3]->Dump(NET__PACKET_ERROR, " --> ");
		codelog(NET__PACKET_ERROR, "in:");
		payload->Dump(NET__PACKET_ERROR, "    ");
		delete payload;
		return(false);
	}
	
	delete payload;
	return(true);
}

PyRepTuple *PyCallStream::Encode() {
	PyRepTuple *res_tuple = new PyRepTuple(4);
	
	//remoteObject
	if(remoteObject == 0)
		res_tuple->items[0] = new PyRepString(remoteObjectStr.c_str());
	else
		res_tuple->items[0] = new PyRepInteger(remoteObject);

	//method name
	res_tuple->items[1] = new PyRepString(method.c_str());

	//args
	//TODO: we dont really need to clone this if we can figure out a way to say "this is read only"
	//or if we can change this encode method to consume the PyCallStream (which will almost always be the case)
	res_tuple->items[2] = arg_tuple->TypedClone();
	
	//options
	if(arg_dict == NULL) {
		res_tuple->items[3] = new PyRepNone();
	} else {
		res_tuple->items[3] = arg_dict->TypedClone();
	}

	//now that we have the main arg tuple, build the unknown stuff around it...
	PyRepTuple *it2 = new PyRepTuple(2);
	it2->items[0] = new PyRepInteger(remoteObject==0?1:0); /* some sort of flag, "process here or call UP"....*/
	it2->items[1] = new PyRepSubStream(res_tuple);

	PyRepTuple *it1 = new PyRepTuple(2);
	it1->items[0] = it2;
	it1->items[1] = new PyRepNone();	//this is the "channel" dict if populated.
	
	return(it1);
}



EVENotificationStream::EVENotificationStream()
: notifyType("NO TYPE SET"),
  remoteObject(0),
  args(NULL)
{
}

EVENotificationStream::~EVENotificationStream() {
	delete args;
}

EVENotificationStream *EVENotificationStream::Clone() const {
	EVENotificationStream *res = new EVENotificationStream();
	res->args = (PyRepTuple *) args->Clone();
	return(res);
}

void EVENotificationStream::Dump(LogType type, PyVisitor *dumper) {
	_log(type, "Notification: %s", notifyType.c_str());
	if(remoteObject == 0) {
		_log(type, "  Remote Object: %s", remoteObjectStr.c_str());
	} else {
		_log(type, "  Remote Object: %lu", remoteObject);
	}
	_log(type, "  Arguments:");
	args->visit(dumper);
}

bool EVENotificationStream::Decode(const std::string &pkt_type, const std::string &notify_type, PyRepTuple *&in_payload) {
	PyRepTuple *payload = in_payload;	//consume
	in_payload = NULL;
	
	delete args;
	args = NULL;
	
	if(pkt_type != "macho.Notification") {
		codelog(NET__PACKET_ERROR, "notification payload has unknown string type %s", pkt_type.c_str());
		delete payload;
		return(false);
	}

	//decode payload tuple
	if(payload->items.size() != 2) {
		codelog(NET__PACKET_ERROR, "invalid tuple length %d", payload->items.size());
		delete payload;
		return(false);
	}
	if(!payload->items[0]->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "non-tuple payload[0]");
		delete payload;
		return(false);
	}
	PyRepTuple *payload2 = (PyRepTuple *) payload->items[0];
	if(payload2->items.size() != 2) {
		codelog(NET__PACKET_ERROR, "invalid tuple2 length %d", payload2->items.size());
		delete payload;
		return(false);
	}

	//decode inner payload tuple
	//ignore tuple 0, it should be an int, dont know what it is
	if(!payload2->items[1]->CheckType(PyRep::SubStream)) {
		codelog(NET__PACKET_ERROR, "non-substream type");
		delete payload;
		return(false);
	}
	PyRepSubStream *ss = (PyRepSubStream *) payload2->items[1];
	
	ss->DecodeData();
	if(ss->decoded == NULL) {
		codelog(NET__PACKET_ERROR, "Unable to decode call stream");
		delete payload;
		return(false);
	}
	
	if(!ss->decoded->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "packet body does not contain a tuple");
		delete payload;
		return(false);
	}

	PyRepTuple *robjt = (PyRepTuple *) ss->decoded;
	if(robjt->items.size() != 2) {
		codelog(NET__PACKET_ERROR, "packet body has %d elements, expected %d", robjt->items.size(), 2);
		delete payload;
		return(false);
	}

	//parse first tuple element, remote object
	if(robjt->items[0]->CheckType(PyRep::Integer)) {
		PyRepInteger *tuple0 = (PyRepInteger *) robjt->items[0];
		remoteObject = tuple0->value;
		remoteObjectStr = "";
	} else if(robjt->items[0]->CheckType(PyRep::String)) {
		PyRepString *tuple0 = (PyRepString *) robjt->items[0];
		remoteObject = 0;
		remoteObjectStr = tuple0->value;
	} else {
		codelog(NET__PACKET_ERROR, "main tuple[0] has invalid type %s", robjt->items[0]->TypeString());
		_log(NET__PACKET_ERROR, " in:");
		PyLogsysDump d(NET__PACKET_ERROR);
		payload->visit(&d);
		delete payload;
		return(false);
	}

	if(!robjt->items[1]->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "main tuple[1] has non-tuple type %s", robjt->items[0]->TypeString());
		_log(NET__PACKET_ERROR, " it is:");
		PyLogsysDump d(NET__PACKET_ERROR);
		payload->visit(&d);
		delete payload;
		return(false);
	}



	PyRepTuple *subt = (PyRepTuple *) robjt->items[1];
	if(subt->items.size() != 2) {
		codelog(NET__PACKET_ERROR, "packet body has %d elements, expected %d", subt->items.size(), 2);
		delete payload;
		return(false);
	}

	//parse first tuple element, remote object
	if(subt->items[0]->CheckType(PyRep::Integer)) {
		//PyRepInteger *tuple0 = (PyRepInteger *) maint->items[0];
		//no idea what this is.
	} else {
		codelog(NET__PACKET_ERROR, "sub tuple[0] has invalid type %s", subt->items[0]->TypeString());
		_log(NET__PACKET_ERROR, " in:");
		PyLogsysDump d(NET__PACKET_ERROR);
		payload->visit(&d);
		delete payload;
		return(false);
	}


	
	if(!subt->items[1]->CheckType(PyRep::Tuple)) {
		codelog(NET__PACKET_ERROR, "subt tuple[1] has non-tuple type %s", robjt->items[0]->TypeString());
		_log(NET__PACKET_ERROR, " it is:");
		PyLogsysDump d(NET__PACKET_ERROR);
		payload->visit(&d);
		delete payload;
		return(false);
	}
	
	args = (PyRepTuple *) subt->items[1];
	subt->items[1] = NULL;

	notifyType = notify_type;
	
	delete payload;
	return(true);
}

PyRepTuple *EVENotificationStream::Encode() {
	
	PyRepTuple *t4 = new PyRepTuple(2);
	t4->items[0] = new PyRepInteger(1);
	//see notes in other objects about what we could do to avoid this clone.
	t4->items[1] = args->Clone();
	
	PyRepTuple *t3 = new PyRepTuple(2);
	t3->items[0] = new PyRepInteger(0);
	t3->items[1] = t4;
	
	PyRepTuple *t2 = new PyRepTuple(2);
	t2->items[0] = new PyRepInteger(0);
	t2->items[1] = new PyRepSubStream(t3);
	
	PyRepTuple *t1 = new PyRepTuple(2);
	t1->items[0] = t2;
	t1->items[1] = new PyRepNone();

	return(t1);
/*	
	//remoteObject
	if(remoteObject == 0)
		arg_tuple->items[0] = new PyRepString(remoteObjectStr.c_str());
	else
		arg_tuple->items[0] = new PyRepInteger(remoteObject);

	//method name
	arg_tuple->items[1] = new PyRepString(method.c_str());

	//args
	//TODO: we dont really need to clone this if we can figure out a way to say "this is read only"
	//or if we can change this encode method to consume the PyCallStream (which will almost always be the case)
	arg_tuple->items[2] = args->Clone();
	
	//options
	if(included_options == 0) {
		arg_tuple->items[3] = new PyRepNone();
	} else {
		PyRepDict *d = new PyRepDict();
		arg_tuple->items[3] = d;
		if(included_options & oMachoVersion) {
			d->items[ new PyRepString("machoVersion") ] = new PyRepInteger( macho_version );
		}
	}
	return(arg_tuple);
	*/
}


/*


      Args:   [ 4] Tuple: 2 elements
      Args:   [ 4]   [ 0] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1] Substream: length 82 from data
      Args:   [ 4]   [ 0]   [ 1]     Tuple: 2 elements
		  remote object:
      Args:   [ 4]   [ 0]   [ 1]       [ 0] Integer field: 0

		  
      Args:   [ 4]   [ 0]   [ 1]       [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 0] Integer field: 1
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1] Tuple: 5 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0] Integer field: 1
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1] Integer field: 2228
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 2] String: 'SendMessage'
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3] Tuple: 5 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 0] (None)
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 1] Integer field: 1000166
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 2] List: 3 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 2]   [ 0] Integer field: 106271822
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 2]   [ 1] String: 'Amarr Citizen 84528'
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 2]   [ 2] Integer field: 1374
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 3] Integer field: 268435459
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 3]   [ 4] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 4] Tuple: 1 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 4]   [ 0] String: 'ok thx'
      Args:   [ 4]   [ 1] (None)




      Args:   [ 4] Tuple: 2 elements
      Args:   [ 4]   [ 0] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1] Substream: length 213 from data
      Args:   [ 4]   [ 0]   [ 1]     Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1]       [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 0] Integer field: 1
		  
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0] Integer field: 30001407
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1] Dictionary: 20 entries
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 0] Key: Integer field: 734233532
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 0] Value: Dictionary: 0 entries
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 1] Key: Integer field: 293063150
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 1] Value: Dictionary: 0 entries
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 2] Key: Integer field: 164866354
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 2] Value: Dictionary: 0 entries
...
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [18] Value:   [ 0] Value: Integer field: 127943639122103341
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [19] Key: Integer field: 229111461
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [19] Value: Dictionary: 0 entries 
      Args:   [ 4]   [ 1] (None)
      Args:   [ 5] Dictionary: 1 entries
      Args:   [ 5]   [ 0] Key: String: 'sn'
      Args:   [ 5]   [ 0] Value: Integer field: 1


      Args:   [ 4] Tuple: 2 elements
      Args:   [ 4]   [ 0] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1] Substream: length 276 from data
      Args:   [ 4]   [ 0]   [ 1]     Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1]       [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 0] Integer field: 1
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1] Tuple: 1 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0] List: 6 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0] Tuple: 7 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 0] String: 'OnModuleAttributeChange'
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 1] Integer field: 104954155
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 2] Integer field: 104954155
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 3] Integer field: 219
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 4] Integer field: 127943650195891707
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 5] Real Field: 0.960000
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 0]   [ 6] Real Field: 0.960000
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1] Tuple: 7 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 0] String: 'OnModuleAttributeChange'
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 1] Integer field: 104954155
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 2] Integer field: 104954155
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 3] Integer field: 378
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 4] Integer field: 127943650195909572
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 5] Real Field: 0.765000
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   [ 1]   [ 6] Real Field: 0.765000
...
      Args:   [ 4]   [ 1] (None)
      Args:   [ 5] Dictionary: 1 entries
      Args:   [ 5]   [ 0] Key: String: 'sn'
      Args:   [ 5]   [ 0] Value: Integer field: 2



OnCharNowInStation(686980647):
      Args:   [ 4] Tuple: 2 elements
      Args:   [ 4]   [ 0] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1] Substream: length 15 from data
      Args:   [ 4]   [ 0]   [ 1]     Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1]       [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 0] Integer field: 1
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1] Tuple: 1 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0] Integer field: 686980647

ScatterEvent( OnItemChange ,*args= (Row(itemID: 104947580,typeID: 670,ownerID: 2,locationID: 6,flag: 0,contraband: 0,singleton: 1,quantity: 1,groupID: 29,categoryID: 6,customInfo: None), {2: 104954155, 3: 60004420, 4: 4}) ,**kw= {} )
      Args:   [ 2] Object:
      Args:   [ 2]   Type: String: 'macho.MachoAddress'
      Args:   [ 2]   Args: Tuple: 4 elements
      Args:   [ 2]   Args:   [ 0] String: 'B'
      Args:   [ 2]   Args:   [ 1] String: 'OnItemChange'
      Args:   [ 2]   Args:   [ 2] List: Empty
      Args:   [ 2]   Args:   [ 3] String: 'charid'
      Args:   [ 3] Integer field: 1152568
      Args:   [ 4] Tuple: 2 elements
      Args:   [ 4]   [ 0] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1] Substream: length 86 from data
      Args:   [ 4]   [ 0]   [ 1]     Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 0] Integer field: 0
      Args:   [ 4]   [ 0]   [ 1]       [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 0] Integer field: 1
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1] Tuple: 2 elements
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0] Object:
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 0]   Type: String: 'util.Row'
....
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1] Dictionary: 3 entries
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 0] Key: Integer field: 2
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 0] Value: Integer field: 104954155
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 1] Key: Integer field: 4
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 1] Value: Integer field: 4
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 2] Key: Integer field: 3           
      Args:   [ 4]   [ 0]   [ 1]       [ 1]   [ 1]   [ 1]   [ 2] Value: Integer field: 60004420        
      Args:   [ 4]   [ 1] (None)

*/















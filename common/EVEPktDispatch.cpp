
#include "EVEPktDispatch.h"
#include "../common/logsys.h"
#include "../common/PyPacket.h"
#include "../common/PyRep.h"
#include "../packets/AccountPkts.h"
#include "../packets/General.h"

EVEPacketDispatcher::EVEPacketDispatcher() {
}

EVEPacketDispatcher::~EVEPacketDispatcher() {
}

void EVEPacketDispatcher::DispatchPacket(PyPacket **in_p) {
	PyPacket *packet = *in_p;
	*in_p = NULL;
	
	switch(packet->type) {
	case AUTHENTICATION_REQ: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.AuthenticationReq") {
			_log(NET__DISPATCH_ERROR, "Received AUTHENTICATION_RSP with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		AuthenticationReq *obj = new AuthenticationReq();
		if(!obj->Decode(&packet->payload)) {
			_log(NET__DISPATCH_ERROR, "Failed to decode AuthenticationReq");
			delete obj;
			break;
		}
		Handle_AuthenticationReq(packet, &obj);
		delete obj;	//in case they didnt
	} break;
	
	case AUTHENTICATION_RSP: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.AuthenticationRsp") {
			_log(NET__DISPATCH_ERROR, "Received AUTHENTICATION_RSP with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		AuthenticationRsp *obj = new AuthenticationRsp();
		if(!obj->Decode(&packet->payload)) {
			_log(NET__DISPATCH_ERROR, "Failed to decode AuthenticationRsp");
			delete obj;
			break;
		}
		Handle_AuthenticationRsp(packet, &obj);
		delete obj;	//in case they didnt
	} break;

	case CALL_REQ: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.CallReq") {
			_log(NET__DISPATCH_ERROR, "Received CALL_REQ with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		PyCallStream *call = new PyCallStream();
		if(!call->Decode(packet->type_string, packet->payload)) {	//payload is consumed
			_log(NET__DISPATCH_ERROR, "Failed to convert packet into a call stream");
			delete call;
			break;
		}
		Handle_CallReq(packet, &call);
		delete call;	//in case they didnt
	} break;
	
	case CALL_RSP: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.CallRsp") {
			_log(NET__DISPATCH_ERROR, "Received CALL_RSP with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		Handle_CallRsp(packet, &packet->payload);
	} break;
	
	case NOTIFICATION: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.Notification") {
			_log(NET__DISPATCH_ERROR, "Received CALL_RSP with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		if(packet->dest.type != PyAddress::Broadcast) {
			_log(NET__DISPATCH_ERROR, "Notification received with non-broadcast destination:");
			packet->dest.Dump(NET__DISPATCH_ERROR, "  Dest: ");
			packet->source.Dump(NET__DISPATCH_ERROR, "  Src:  ");
			break;
		}
		EVENotificationStream *obj = new EVENotificationStream;
		if(!obj->Decode(packet->type_string, packet->dest.service, packet->payload)) {
			_log(NET__DISPATCH_ERROR, "Failed to decode notification of type '%s'", packet->dest.service.c_str());
			delete obj;
			break;
		}
		Handle_Notify(packet, &obj);
		delete obj;	//in case they didnt
	} break;
	
	case ERRORRESPONSE: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.ErrorResponse") {
			_log(NET__DISPATCH_ERROR, "Received ERRORRESPONSE with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		ErrorResponseBody *obj = new ErrorResponseBody;
		if(!obj->Decode(&packet->payload)) {
			_log(NET__DISPATCH_ERROR, "Failed to decode Error Response");
			delete obj;
			break;
		}
		Handle_ErrorResponse(packet, &obj);
		delete obj;	//in case they didnt
	} break;
	
	case SESSIONCHANGENOTIFICATION: {
		//check the string part, just for good measure
		if(packet->type_string != "macho.SessionChangeNotification") {
			_log(NET__DISPATCH_ERROR, "Received SESSIONCHANGENOTIFICATION with invalid type string '%s'", packet->type_string.c_str());
			break;
		}
		
		SessionChangeNotification *obj = new SessionChangeNotification();
		if(!obj->Decode(&packet->payload)) {
			_log(NET__DISPATCH_ERROR, "Failed to decode session change notification");
			delete obj;
			break;
		}
		Handle_SessionChange(packet, &obj);
		delete obj;	//in case they didnt
	} break;
	
	default: {
		Handle_Other(&packet);
	} break;
	
	}
	delete packet;
}

/* default handlers do nothing but print */
void EVEPacketDispatcher::Handle_AuthenticationReq(const PyPacket *packet, AuthenticationReq **call) {
	_log(NET__DISPATCH_ERROR, "Unhandled Authentication Request");
}

void EVEPacketDispatcher::Handle_AuthenticationRsp(const PyPacket *packet, AuthenticationRsp **call) {
	_log(NET__DISPATCH_ERROR, "Unhandled Authentication Response");
}

void EVEPacketDispatcher::Handle_CallReq(const PyPacket *packet, PyCallStream **call) {
	_log(NET__DISPATCH_ERROR, "Unhandled Call Request");
}

void EVEPacketDispatcher::Handle_CallRsp(const PyPacket *packet, PyRepTuple **res) {
	_log(NET__DISPATCH_ERROR, "Unhandled Call Response");
}

void EVEPacketDispatcher::Handle_Notify(const PyPacket *packet, EVENotificationStream **notify) {
	_log(NET__DISPATCH_ERROR, "Unhandled Notification");
}

void EVEPacketDispatcher::Handle_SessionChange(const PyPacket *packet, SessionChangeNotification **notify) {
	_log(NET__DISPATCH_ERROR, "Unhandled SessionChange");
}

void EVEPacketDispatcher::Handle_ErrorResponse(const PyPacket *packet, ErrorResponseBody **body) {
	_log(NET__DISPATCH_ERROR, "Unhandled Error Response");
}

void EVEPacketDispatcher::Handle_Other(PyPacket **packet) {
	_log(NET__DISPATCH_ERROR, "Unhandled Packet of type %d", (*packet)->type);
}


















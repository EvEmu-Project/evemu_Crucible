
#include "EVECollectDisp.h"
#include "../common/logsys.h"
#include "../common/PyPacket.h"
#include "../common/PyRep.h"
#include "../packets/AccountPkts.h"
#include "../packets/General.h"
#include "../packets/LSCPkts.h"
#include "../common/PyDumpVisitor.h"
#include "../common/PyXMLGenerator.h"
#include "../common/CachedObjectMgr.h"
#include "../common/RowsetReader.h"



EVECollectDispatcher::EVECollectDispatcher()
: EVEPacketDispatcher()
{
#define CALL_REQ_HANDLER(c) m_callReqDisp[#c] = &EVECollectDispatcher::Req_##c;
#define CALL_RSP_HANDLER(c) m_callRspDisp[#c] = &EVECollectDispatcher::Rsp_##c;
#define BOUND_CALL_RSP_HANDLER(c) m_callRspDisp[#c] = &EVECollectDispatcher::Bound_Rsp_##c;
#define NOTIFY_HANDLER(c) m_notifyDisp[#c] = &EVECollectDispatcher::Notify_##c;
	#include "method_types.h"

#define DESTINY_HANDLER(c) m_destinyDisp[#c] = &EVECollectDispatcher::Destiny_##c;
	#include "destiny_types.h"
}

EVECollectDispatcher::~EVECollectDispatcher() {
}


void EVECollectDispatcher::Handle_AuthenticationReq(const PyPacket *packet, AuthenticationReq **call) {
	_log(COLLECT__MESSAGE, "Saw AuthenticationReq for %s", (*call)->login.c_str());
}

void EVECollectDispatcher::Handle_AuthenticationRsp(const PyPacket *packet, AuthenticationRsp **rsp) {
	_log(COLLECT__MESSAGE, "Saw AuthenticationRsp");
}

void EVECollectDispatcher::Handle_CallReq(const PyPacket *packet, PyCallStream **in_call) {
	PyCallStream *call = *in_call;
	*in_call = NULL;
	
	pendingCalls[packet->source.callID] = call->method;

	
	_log(COLLECT__CALL_SUMMARY, "Call ID "I64u": %s          (svc %s)", packet->source.callID, call->method.c_str(), packet->dest.service.c_str());
	packet->source.Dump(COLLECT__PACKET_SRC, "  Src:  ");
	packet->dest.Dump(COLLECT__PACKET_DEST, "  Dest: ");
	if(is_log_enabled(COLLECT__CALL_DUMP)) {
		PyLookupDump dumper(&lookResolver, COLLECT__CALL_DUMP);
		call->Dump(COLLECT__CALL_DUMP, &dumper);
	}

	
	if(call->method != "MachoBindObject") {	//this is handled elsewhere...
		if(is_log_enabled(COLLECT__CALL_XML)) {
			printf("<element name=\"Call%s\">\n", call->method.c_str());
			PyXMLGenerator gen(stdout);
			call->arg_tuple->visit(&gen);
			printf("</element>\n");
		}
	}
	
	//send it to a dispatcher if there is one registered
	std::map<std::string, _CallReqDispatch>::const_iterator res;
	res = m_callReqDisp.find(call->method);
	if(res != m_callReqDisp.end()) {
		_CallReqDispatch dsp = res->second;
		(this->*dsp)(packet, &call);
	}
	
	delete call;
}

void EVECollectDispatcher::Handle_CallRsp(const PyPacket *packet, PyRepTuple **in_result) {
	PyRepTuple *result = *in_result;
	*in_result = NULL;
	
	std::map<uint32, std::string>::iterator res;
	res = pendingCalls.find(packet->dest.callID);
	if(res != pendingCalls.end()) {
		const char *pfx = "";
		if(result->items.size() == 1 && result->items[0]->CheckType(PyRep::SubStream)) {
			PyRepSubStream *ss = (PyRepSubStream *) result->items[0];
			ss->DecodeData();
			if(ss->decoded != NULL) {
				if(ss->decoded->CheckType(PyRep::None))
					pfx = "Empty ";

				//decode any nested substreams.
				SubStreamDecoder ssd;
				ss->decoded->visit(&ssd);
			}
		}
		
		_log(COLLECT__CALL_SUMMARY, "%sCall Response for ID "I64u": %s           (svc %s)", pfx, packet->dest.callID, res->second.c_str(), packet->source.service.c_str());

		packet->source.Dump(COLLECT__PACKET_SRC, "  Src:  ");
		packet->dest.Dump(COLLECT__PACKET_DEST, "  Dest: ");

		if(is_log_enabled(COLLECT__CALL_DUMP)) {
			if(res->second != "GetCachableObject") {	//these get dumped elsewhere
				SubStreamDecoder v;
				result->visit(&v);
				PyLookupDump dumper(&lookResolver, COLLECT__CALL_DUMP);
				result->visit(&dumper);
			}
		} //end "is dump enabled"

		if(is_log_enabled(COLLECT__CALLRSP_SQL) 
		   && result != NULL 
		   && res->second != "MachoBindObject"	//bind object nested calls dumped elsewhere.
		) {	
			//run through the result looking for SQL tables, and dump them.
			SetSQLDumper sql_dumper(stdout, res->second.c_str());
			result->visit(&sql_dumper);
		}
		
		if(is_log_enabled(COLLECT__CALLRSP_XML)
		   && result != NULL 
		   && res->second != "MachoBindObject"	//bind object nested calls dumped elsewhere.
		) {	
			printf("<element name=\"Rsp%s\">\n", res->second.c_str());
			PyXMLGenerator gen(stdout);
			result->visit(&gen);
			printf("</element>\n");
		}

		//send it to a dispatcher if there is one registered
		std::map<std::string, _CallRspDispatch>::const_iterator res2;
		res2 = m_callRspDisp.find(res->second);
		if(res2 != m_callRspDisp.end()) {
			_CallRspDispatch dsp = res2->second;
			(this->*dsp)(packet, &result);
		}/* else {
			_log(COLLECT__ERROR, "Unable to find handler for '%s'", res->second.c_str());
		}*/

		pendingCalls.erase(res);
	} else {
		_log(COLLECT__CALL_SUMMARY, "Call Response for unknonw call ID "I64u, packet->dest.callID);
	}
	delete result;
}

void EVECollectDispatcher::Handle_Notify(const PyPacket *packet, EVENotificationStream **in_notify) {
	EVENotificationStream *notify = *in_notify;
	*in_notify = NULL;
	
	_log(COLLECT__NOTIFY_SUMMARY, "Notification of type %s", packet->dest.service.c_str());
	packet->source.Dump(COLLECT__PACKET_SRC, "  Src:  ");
	packet->dest.Dump(COLLECT__PACKET_DEST, "  Dest: ");
	if(is_log_enabled(COLLECT__NOTIFY_DUMP)) {
		PyLookupDump dumper(&lookResolver, COLLECT__NOTIFY_DUMP);
		notify->Dump(COLLECT__NOTIFY_DUMP, &dumper);
	}
	
	if(is_log_enabled(COLLECT__NOTIFY_XML)) {
		printf("<element name=\"Notify%s\">\n", packet->dest.service.c_str());
		PyXMLGenerator gen(stdout);
		notify->args->visit(&gen);
		printf("</element>\n");
	}

	//send it to a dispatcher if there is one registered
	std::map<std::string, _NotifyDispatch>::const_iterator res;
	res = m_notifyDisp.find(packet->dest.service);
	if(res != m_notifyDisp.end()) {
		_NotifyDispatch dsp = res->second;
		(this->*dsp)(packet, &notify);
	}
	
	delete notify;
}

void EVECollectDispatcher::Handle_SessionChange(const PyPacket *packet, SessionChangeNotification **notify) {
	_log(COLLECT__ERROR, "Unprocessed SessionChange");
}

void EVECollectDispatcher::Handle_ErrorResponse(const PyPacket *packet, ErrorResponseBody **body) {
	_log(COLLECT__ERROR, "Unprocessed ErrorResponse");
}

void EVECollectDispatcher::Handle_Other(PyPacket **in_packet) {
	PyPacket *packet = *in_packet;
	*in_packet = NULL;
	
	//everything else...
	if(is_log_enabled(COLLECT__CALL_DUMP)) {
		//semi-hack: if we are dumping general stuff, dump the misc packets directly.

		//decode substreams to facilitate dumping better:
		SubStreamDecoder v;
		packet->payload->visit(&v);
		
		_log(COLLECT__OTHER_DUMP, "Misc Packet:");
		
		packet->source.Dump(COLLECT__PACKET_SRC, "  Src:  ");
		packet->dest.Dump(COLLECT__PACKET_DEST, "  Dest: ");
		
		PyLookupDump dumper(&lookResolver, COLLECT__CALL_DUMP);
		packet->Dump(COLLECT__CALL_DUMP, &dumper);
	}
	if(is_log_enabled(COLLECT__MISC_XML)) {
			printf("<element name=\"??\">\n");
			PyXMLGenerator gen(stdout);
			packet->payload->visit(&gen);
			printf("</element>\n");
	}

	delete packet;
}


void EVECollectDispatcher::Rsp_MachoBindObject(const PyPacket *packet, PyRepTuple **res) {
	if(		(*res)->items.size() != 1
	   ||	!(*res)->items[0]->CheckType(PyRep::SubStream)) {
		_log(COLLECT__ERROR, "Invalid MachoBindObject RSP");
		return;
	}
	PyRepSubStream *ss = (PyRepSubStream *) (*res)->items[0];
	ss->DecodeData();
	if(ss->decoded == NULL) {
		_log(COLLECT__ERROR, "Unable to decode MachoBindObject RSP");
		return;
	}

	std::string method_name = "UNKNOWN";
	std::map<uint32, std::string>::iterator resp;
	resp = pendingBinds.find(packet->dest.callID);
	if(resp != pendingBinds.end()) {
		method_name = resp->second;
		pendingBinds.erase(resp);
	}
	
	RspMachoBindObject c;
	if(c.Decode(res)) {
		_log(COLLECT__CALL_SUMMARY, "Call Response for Bind Call ID "I64u": %s", packet->dest.callID, method_name.c_str());
		
		if(is_log_enabled(COLLECT__CALLRSP_SQL)) {	
			//run through the result looking for SQL tables, and dump them.
			SetSQLDumper sql_dumper(stdout, method_name.c_str());
			c.call_return->visit(&sql_dumper);
		}
		
		if(is_log_enabled(COLLECT__CALLRSP_XML)) {
			printf("<element name=\"Call%s\">\n", method_name.c_str());
			PyXMLGenerator gen(stdout);
			c.call_return->visit(&gen);
			printf("</element>\n");
		}
	}
}

void EVECollectDispatcher::Req_MachoBindObject(const PyPacket *packet, PyCallStream **call) {
	//TODO: dispatch the call
	_log(COLLECT__ERROR, "UN-dispatched MachoBindObject call");
	CallMachoBindObject obj;
	if(obj.Decode(&(*call)->arg_tuple)) {
		if(!obj.call->CheckType(PyRep::None)) {
			CallMachoBindObject_call c;
			if(c.Decode(&obj.call)) {
				_log(COLLECT__CALL_SUMMARY, "Nested Call ID "I64u": %s", packet->source.callID, c.method_name.c_str());

				//register the bind
				pendingBinds[packet->source.callID] = c.method_name;
				
				if(is_log_enabled(COLLECT__CALL_XML)) {
					printf("<element name=\"Call%s\">\n", c.method_name.c_str());
					PyXMLGenerator gen(stdout);
					c.arguments->visit(&gen);
					printf("</element>\n");
				}
			}
		}
	}
}

void EVECollectDispatcher::Rsp_GetCachableObject(const PyPacket *packet, PyRepTuple **in_result) {
	PyRepTuple *result = *in_result;
	*in_result = NULL;
	
	if(result->items.size() == 1 &&
	   result->items[0]->CheckType(PyRep::SubStream)) {
		PyRepSubStream *s = (PyRepSubStream *) result->items[0];
		result->items[0] = NULL;
		
		PyCachedObjectDecoder cobj;
		if(!cobj.Decode(&s)) {
			_log(CLIENT__ERROR, "Failed to decode GetCachableObject response");
		} else {
			
			//decode the body
			SubStreamDecoder v;
			cobj.cache->visit(&v);
			
			_log(COLLECT__CALL_DUMP, "GetCachableObject reply decoded:");
			if(is_log_enabled(COLLECT__CALL_DUMP))
				cobj.Dump(stdout, "    ", true);
			
			if(is_log_enabled(COLLECT__CALLRSP_SQL)) {	
				//run through the result looking for SQL tables, and dump them.
				SetSQLDumper sql_dumper(stdout, "GetCachableObject");
				cobj.cache->visit(&sql_dumper);
			}
		}
	}

	delete result;
}

void EVECollectDispatcher::Notify_OnLSC(const PyPacket *packet, EVENotificationStream **in_notify) {
	EVENotificationStream *notify = *in_notify;
	*in_notify = NULL;
	
	NotifyOnLSC lsc;
	if(!lsc.Decode(&notify->args)) {
		delete notify;
		codelog(COLLECT__ERROR, "Unable to decode OnLSC update main.");
		return;
	}

	_log(COLLECT__NOTIFY_SUMMARY, "    LSC Method on channel %u: %s", lsc.channelID, lsc.method.c_str());
}
















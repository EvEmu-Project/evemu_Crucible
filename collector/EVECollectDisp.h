#ifndef __EVECOLLECTDISP_H_INCL__
#define __EVECOLLECTDISP_H_INCL__

#include "../common/EVEPktDispatch.h"
#include "../common/PyLookupDump.h"

class DoDestinyAction;

class EVECollectDispatcher
: public EVEPacketDispatcher {
public:
	EVECollectDispatcher();
	virtual ~EVECollectDispatcher();

	PyLookupResolver lookResolver;
	
protected:
	virtual void Handle_AuthenticationReq(const PyPacket *packet, AuthenticationReq **call);
	virtual void Handle_AuthenticationRsp(const PyPacket *packet, AuthenticationRsp **call);
	virtual void Handle_CallReq(const PyPacket *packet, PyCallStream **call);
	virtual void Handle_CallRsp(const PyPacket *packet, PyRepTuple **res);
	virtual void Handle_Notify(const PyPacket *packet, EVENotificationStream **notify);
	virtual void Handle_SessionChange(const PyPacket *packet, SessionChangeNotification **notify);
	virtual void Handle_ErrorResponse(const PyPacket *packet, ErrorResponseBody **body);
	virtual void Handle_Other(PyPacket **packet);
	
    std::map<uint32, std::string> pendingCalls;
    std::map<uint32, std::string> pendingBinds;

	//
	// call/notify dispatcher
	// 
	typedef void (EVECollectDispatcher::* _CallReqDispatch)(const PyPacket *packet, PyCallStream **call);
	typedef void (EVECollectDispatcher::* _CallRspDispatch)(const PyPacket *packet, PyRepTuple **res);
	typedef void (EVECollectDispatcher::* _NotifyDispatch)(const PyPacket *packet, EVENotificationStream **notify);
	std::map<std::string, _CallReqDispatch> m_callReqDisp;
	std::map<std::string, _CallRspDispatch> m_callRspDisp;
	std::map<std::string, _NotifyDispatch>  m_notifyDisp;

#define CALL_REQ_HANDLER(c) virtual void Req_##c(const PyPacket *packet, PyCallStream **res);
#define CALL_RSP_HANDLER(c) virtual void Rsp_##c(const PyPacket *packet, PyRepTuple **res);
#define BOUND_CALL_RSP_HANDLER(c) virtual void Bound_Rsp_##c(const PyPacket *packet, PyRepTuple **res);
#define NOTIFY_HANDLER(n) virtual void Notify_##n(const PyPacket *packet, EVENotificationStream **notify);
	#include "method_types.h"

	//
	// Destiny update dispatcher
	// 
	typedef void (EVECollectDispatcher::* _DestinyDispatch)(const PyPacket *packet, DoDestinyAction *action);
	std::map<std::string, _DestinyDispatch>  m_destinyDisp;
	
#define DESTINY_HANDLER(n) virtual void Destiny_##n(const PyPacket *packet, DoDestinyAction *action);
	#include "destiny_types.h"
	
	void _DumpBalls(const byte *data, uint32 len);
};

#endif




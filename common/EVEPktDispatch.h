#ifndef __EVEDISPATCHERS_H_INCL__
#define __EVEDISPATCHERS_H_INCL__

class PyPacket;
class PyRep;
class PyRepTuple;
class AuthenticationReq;
class AuthenticationRsp;
class PyCallStream;
class EVENotificationStream;
class SessionChangeNotification;
class ErrorResponseBody;

class EVEPacketDispatcher {
public:
	EVEPacketDispatcher();
	virtual ~EVEPacketDispatcher();
	
    virtual void DispatchPacket(PyPacket **packet); //consumes the packet
	
protected:
    /* these methods can choose to consume the ** arguments */
	virtual void Handle_AuthenticationReq(const PyPacket *packet, AuthenticationReq **call);
	virtual void Handle_AuthenticationRsp(const PyPacket *packet, AuthenticationRsp **rsp);
	virtual void Handle_CallReq(const PyPacket *packet, PyCallStream **call);
	virtual void Handle_CallRsp(const PyPacket *packet, PyRepTuple **res);
	virtual void Handle_Notify(const PyPacket *packet, EVENotificationStream **notify);
	virtual void Handle_SessionChange(const PyPacket *packet, SessionChangeNotification **notify);
	virtual void Handle_ErrorResponse(const PyPacket *packet, ErrorResponseBody **body);
	virtual void Handle_Other(PyPacket **packet);
};





#endif



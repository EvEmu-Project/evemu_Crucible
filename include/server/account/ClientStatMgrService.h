
#ifndef CLIENT_STAT_MGR_SERVICE_H
#define CLIENT_STAT_MGR_SERVICE_H

class ClientStatsMgr : public PyService {
public:
	ClientStatsMgr(PyServiceMgr *mgr);

	virtual ~ClientStatsMgr()
	{

	}

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	//overloaded in order to support bound objects:
	//virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);

	//PyCallable_DECL_CALL(MachoBindObject)
	//PyCallable_DECL_CALL(SubmitStats)

	PyResult Handle_SubmitStats(PyCallArgs &call);
};

#endif//CLIENT_STAT_MGR_SERVICE_H
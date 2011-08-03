#ifndef contractProxy_h__
#define contractProxy_h__

#include "PyService.h"

class ContractProxyService : public PyService
{
public:
    ContractProxyService(PyServiceMgr *mgr);
    ~ContractProxyService();
protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetLoginInfo)
	PyCallable_DECL_CALL(GetMyExpiredContractList)
};

#endif // contractProxy_h__

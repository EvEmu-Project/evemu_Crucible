#include "EvemuPCH.h"
#include "../server/account/ClientStatMgrService.h"

PyCallable_Make_InnerDispatcher(ClientStatsMgr)

ClientStatsMgr::ClientStatsMgr(PyServiceMgr *mgr) : PyService(mgr, "clientStatsMgr"), m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ClientStatsMgr, SubmitStats);
}

/** client submitting stats.... unknown content
  */
PyResult ClientStatsMgr::Handle_SubmitStats(PyCallArgs &call)
{
	//call.Dump(CLIENT__ERROR);
	return new PyRepNone;
}



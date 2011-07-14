
#include "EVEServerPCH.h"


PyCallable_Make_InnerDispatcher(MailMgrService)

MailMgrService::MailMgrService(PyServiceMgr *mgr)
: PyService(mgr, "mailMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	//add PyCallable_REG_CALL(service, function_name) here
	PyCallable_REG_CALL(MailMgrService, SyncMail);
}

MailMgrService::~MailMgrService() {
	delete m_dispatch;
}

PyResult MailMgrService::Handle_SyncMail(PyCallArgs &args)
{
	sLog.Debug("MailMgrService","Called SyncMail stub");

	return NULL;
}
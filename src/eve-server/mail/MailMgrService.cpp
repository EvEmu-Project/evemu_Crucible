
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

PyResult MailMgrService::Handle_SyncMail(PyCallArgs &call)
{
	Call_TwoIntegerArgs args;
	if (!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Failed to decode SyncMail args");
		return NULL;
	}

	// referring to mail ids
	int firstId = args.arg1, secondId = args.arg2;

	return NULL;
}
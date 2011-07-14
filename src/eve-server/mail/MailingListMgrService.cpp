
#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(MailingListMgrService)

MailingListMgrService::MailingListMgrService(PyServiceMgr *mgr)
: PyService(mgr, "mailingListMgr"),
  m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	//add PyCallable_REG_CALL(service, function_name) here

}

MailingListMgrService::~MailingListMgrService()
{
	delete m_dispatch;
}
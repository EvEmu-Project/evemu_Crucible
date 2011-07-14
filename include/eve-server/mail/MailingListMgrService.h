
#ifndef MAILING_LIST_MGR_SERVICE_H
#define MAILING_LIST_MGR_SERVICE_H

#include "PyService.h"

class MailingListMgrService : public PyService
{
public:
	MailingListMgrService(PyServiceMgr *mgr);
	virtual ~MailingListMgrService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	//add PyCallable_DECL_CALL(function_name) here
};

#endif /* MAILING_LIST_MGR_SERVICE_H */
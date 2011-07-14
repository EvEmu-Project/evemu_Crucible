
#ifndef MAIL_MGR_SERVICE_H
#define MAIL_MGR_SERVICE_H

#include "PyService.h"

class MailMgrService : public PyService
{
public:
	MailMgrService(PyServiceMgr *mgr);
	virtual ~MailMgrService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	//add PyCallable_DECL_CALL(function_name) here
	PyCallable_DECL_CALL(SyncMail);
};

#endif
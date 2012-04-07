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

    PyCallable_DECL_CALL(GetJoinedLists)
    PyCallable_DECL_CALL(Create)
    PyCallable_DECL_CALL(Join)
    PyCallable_DECL_CALL(Leave)
    PyCallable_DECL_CALL(Delete)
    PyCallable_DECL_CALL(KickMembers)
    PyCallable_DECL_CALL(GetMembers)
    PyCallable_DECL_CALL(SetEntityAccess)
    PyCallable_DECL_CALL(ClearEntityAccess)
    PyCallable_DECL_CALL(SetMembersMuted)
    PyCallable_DECL_CALL(SetMembersOperator)
    PyCallable_DECL_CALL(SetMembersClear)
    PyCallable_DECL_CALL(SetDefaultAccess)
    PyCallable_DECL_CALL(GetSettings)
    PyCallable_DECL_CALL(GetWelcomeMail)
    PyCallable_DECL_CALL(SaveWelcomeMail)
    PyCallable_DECL_CALL(SendWelcomeMail)
    PyCallable_DECL_CALL(ClearWelcomeMail)
};

#endif /* MAILING_LIST_MGR_SERVICE_H */
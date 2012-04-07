#ifndef browserLockdownSvc_h__
#define browserLockdownSvc_h__

#include "PyService.h"

class BrowserLockDownService : public PyService
{
public:
    BrowserLockDownService(PyServiceMgr *mgr);
    ~BrowserLockDownService();
protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetFlaggedSitesHash)
    PyCallable_DECL_CALL(GetFlaggedSitesList)
    PyCallable_DECL_CALL(GetDefaultHomePage)

};
#endif // browserLockdownSvc_h__






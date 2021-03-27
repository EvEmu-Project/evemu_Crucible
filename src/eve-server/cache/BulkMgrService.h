/**
 * @name BulkMgrService.h
 *   This file is for processing bulkdata
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    ozatomic (hacked for static client data)
 * @Author:    Allan (added calls and (hacked) updates for new dgm data)
 * @date:      24 January 2017
 *
 */

#ifndef __BULKMGR_SERVICE_H_INCL__
#define __BULKMGR_SERVICE_H_INCL__

#include "PyService.h"
#include "cache/BulkDB.h"

class BulkMgrService : public PyService
{
public:
    BulkMgrService(PyServiceMgr *mgr);
    virtual ~BulkMgrService();
public:

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(UpdateBulk);
    PyCallable_DECL_CALL(GetAllBulkIDs);
    PyCallable_DECL_CALL(GetVersion);
    PyCallable_DECL_CALL(GetChunk);
    PyCallable_DECL_CALL(GetFullFiles);
    PyCallable_DECL_CALL(GetFullFilesChunk);
    PyCallable_DECL_CALL(GetUnsubmittedChanges);
    PyCallable_DECL_CALL(GetUnsubmittedChunk);

private:

    enum bulkStatus {
        updateBulkStatusOK               = 0,   // client data == server data  - no change
        updateBulkStatusWrongBranch      = 1,   // client != server.  calls GetFullFiles then GetVersion
        updateBulkStatusHashMismatch     = 2,   // client missing files  - compares server (returned) fileIDs with local fileIDs
        updateBulkStatusClientNewer      = 3,   // client != server.  calls GetFullFiles then GetVersion
        updateBulkStatusNeedToUpdate     = 4,   // this one will be complicated.  see notes in cpp
        updateBulkStatusTooManyRevisions = 5    // server has too many updates to bring client files up-to-date.  calls GetFullFiles then GetVersion
    };
};

#endif



 /**
  * @name AllianceBound.h
  *     Alliance Bound code
  *
  * @Author:        Allan
  * @date:          16 January 2018
  */



#ifndef EVE_ALLIANCE_ALLYREGISTRY_BOUND_H_
#define EVE_ALLIANCE_ALLYREGISTRY_BOUND_H_


#include "../eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "alliance/AllianceDB.h"

class AllianceBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(AllianceBound)

    AllianceBound(PyServiceMgr *mgr);
    virtual ~AllianceBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(CreateLabel);
    PyCallable_DECL_CALL(GetLabels);
    PyCallable_DECL_CALL(DeleteLabel);
    PyCallable_DECL_CALL(EditLabel);
    PyCallable_DECL_CALL(AssignLabels);
    PyCallable_DECL_CALL(RemoveLabels);

    PyCallable_DECL_CALL(AddBulletin);
    PyCallable_DECL_CALL(GetBulletins);
    PyCallable_DECL_CALL(DeleteBulletin);

    PyCallable_DECL_CALL(GetMembers);
    PyCallable_DECL_CALL(DeclareExecutorSupport);
    PyCallable_DECL_CALL(DeleteMember);
    PyCallable_DECL_CALL(GetApplications);
    PyCallable_DECL_CALL(UpdateApplication);
    PyCallable_DECL_CALL(AddToVoiceChat);
    PyCallable_DECL_CALL(PayBill);
    PyCallable_DECL_CALL(GetBillBalance);
    PyCallable_DECL_CALL(GetBills);
    PyCallable_DECL_CALL(GetBillsReceivable);
    PyCallable_DECL_CALL(GetAllianceContacts);
    PyCallable_DECL_CALL(AddAllianceContact);
    PyCallable_DECL_CALL(EditAllianceContact);
    PyCallable_DECL_CALL(RemoveAllianceContacts);
    PyCallable_DECL_CALL(EditContactsRelationshipID);
    PyCallable_DECL_CALL(GetAlliance);
    PyCallable_DECL_CALL(UpdateAlliance);

protected:
    Dispatcher *const m_dispatch;

};

#endif  // EVE_ALLIANCE_ALLYREGISTRY_BOUND_H_

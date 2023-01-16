
 /**
  * @name AllianceBound.h
  *     Alliance Bound code
  *
  * @Author:        Allan
  * Updated:        James
  * @date:          16 January 2018
  */



#ifndef EVE_ALLIANCE_ALLYREGISTRY_BOUND_H_
#define EVE_ALLIANCE_ALLYREGISTRY_BOUND_H_


#include "../eve-server.h"
#include "services/BoundService.h"
#include "alliance/AllianceDB.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "Client.h"

class AllianceRegistry;

class AllianceBound : public EVEBoundObject <AllianceBound>
{
public:
    AllianceBound(EVEServiceManager& mgr, AllianceRegistry& parent, AllianceDB& db, uint32 allyID);

    static void FillOAApplicationChange(OnAllianceApplicationChanged& OAAC, const Alliance::ApplicationInfo& Old, const Alliance::ApplicationInfo& New);
    static void FillOAMemberChange(OnAllianceMemberChange& OAMC, const Alliance::ApplicationInfo& Old, const Alliance::ApplicationInfo& New);

    uint32 GetAllianceID () const { return this->m_allyID; }
protected:
    bool CanClientCall(Client* client) override;

    PyResult CreateLabel(PyCallArgs& call, PyString* name, std::optional<PyInt*> color);
    PyResult GetLabels(PyCallArgs& call);
    PyResult DeleteLabel(PyCallArgs& call, PyInt* labelID);
    PyResult EditLabel(PyCallArgs& call, PyInt* labelID, PyString* name, std::optional<PyInt*> color);
    PyResult AssignLabels(PyCallArgs& call, PyList* contactIDs, PyInt* labelMask);
    PyResult RemoveLabels(PyCallArgs& call, PyList* contactIDs, PyInt* labelMask);

    PyResult AddBulletin(PyCallArgs& call, PyWString* title, PyWString* body);
    PyResult GetBulletins(PyCallArgs& call);
    PyResult DeleteBulletin(PyCallArgs& call, PyInt* id);

    PyResult GetMembers(PyCallArgs& call);
    PyResult DeclareExecutorSupport(PyCallArgs& call, PyInt* chosenExecutor);
    PyResult DeleteMember(PyCallArgs& call, PyInt* corporationID);
    PyResult GetApplications(PyCallArgs& call);
    PyResult UpdateApplication(PyCallArgs& call, PyInt* corporationID, PyWString* applicationText, PyInt* state);
    PyResult AddToVoiceChat(PyCallArgs& call, PyString* channelName);
    PyResult PayBill(PyCallArgs& call, PyInt* billID, PyInt* fromAccountKey);
    PyResult GetBillBalance(PyCallArgs& call, PyInt* billID);
    PyResult GetBills(PyCallArgs& call);
    PyResult GetBillsReceivable(PyCallArgs& call);
    PyResult GetAllianceContacts(PyCallArgs& call);
    PyResult AddAllianceContact(PyCallArgs& call, PyInt* contactID, PyInt* relationshipID);
    PyResult EditAllianceContact(PyCallArgs& call, PyInt* contactID, PyInt* relationshipID);
    PyResult RemoveAllianceContacts(PyCallArgs& call, PyList* contactIDs);
    PyResult EditContactsRelationshipID(PyCallArgs& call, PyList* contactIDs, PyInt* relationshipID);
    PyResult GetAlliance(PyCallArgs& call);
    PyResult UpdateAlliance(PyCallArgs& call, PyWString* description, PyWString* url);

    AllianceDB& m_db;
    ObjCacheService* m_cache;
    LSCService* m_lsc;

    uint32 m_allyID;
};

#endif  // EVE_ALLIANCE_ALLYREGISTRY_BOUND_H_

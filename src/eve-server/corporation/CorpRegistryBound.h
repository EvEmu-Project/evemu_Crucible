


#ifndef __EVEMU_CORP_CORPREGISTRY_BOUND_H_
#define __EVEMU_CORP_CORPREGISTRY_BOUND_H_


#include "../eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "corporation/CorporationDB.h"

class CorpRegistryBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(CorpRegistryBound)

    CorpRegistryBound(PyServiceMgr *mgr, CorporationDB& db, uint32 corpID);
    virtual ~CorpRegistryBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetEveOwners);
    PyCallable_DECL_CALL(GetCorporation);
    PyCallable_DECL_CALL(GetCorporations);
    PyCallable_DECL_CALL(GetInfoWindowDataForChar);
    PyCallable_DECL_CALL(GetLockedItemLocations);
    PyCallable_DECL_CALL(AddCorporation);
    PyCallable_DECL_CALL(GetSuggestedTickerNames);
    PyCallable_DECL_CALL(GetOffices);
    PyCallable_DECL_CALL(GetStations);

    PyCallable_DECL_CALL(CreateRecruitmentAd);
    PyCallable_DECL_CALL(UpdateRecruitmentAd);
    PyCallable_DECL_CALL(DeleteRecruitmentAd);
    PyCallable_DECL_CALL(GetRecruiters);
    PyCallable_DECL_CALL(GetRecruitmentAdsForCorporation);
    PyCallable_DECL_CALL(GetMyApplications);
    PyCallable_DECL_CALL(InsertApplication);
    PyCallable_DECL_CALL(GetApplications);
    PyCallable_DECL_CALL(UpdateApplicationOffer);
    PyCallable_DECL_CALL(DeleteApplication);

    PyCallable_DECL_CALL(UpdateDivisionNames);
    PyCallable_DECL_CALL(UpdateCorporation);
    PyCallable_DECL_CALL(UpdateLogo);
    PyCallable_DECL_CALL(SetAccountKey);
    PyCallable_DECL_CALL(GetMember);
    PyCallable_DECL_CALL(GetMembers);

    PyCallable_DECL_CALL(MoveCompanyShares);
    PyCallable_DECL_CALL(MovePrivateShares);
    PyCallable_DECL_CALL(GetSharesByShareholder);
    PyCallable_DECL_CALL(GetShareholders);
    PyCallable_DECL_CALL(PayoutDividend);

    PyCallable_DECL_CALL(CanViewVotes);
    PyCallable_DECL_CALL(InsertVoteCase);
    PyCallable_DECL_CALL(GetVotes);
    PyCallable_DECL_CALL(CanVote);
    PyCallable_DECL_CALL(InsertVote);
    PyCallable_DECL_CALL(GetVoteCasesByCorporation);
    PyCallable_DECL_CALL(GetVoteCaseOptions);
    PyCallable_DECL_CALL(GetSanctionedActionsByCorporation);

    PyCallable_DECL_CALL(GetRoleGroups);
    PyCallable_DECL_CALL(GetRoles);

    PyCallable_DECL_CALL(CreateLabel);
    PyCallable_DECL_CALL(GetLabels);
    PyCallable_DECL_CALL(DeleteLabel);
    PyCallable_DECL_CALL(EditLabel);
    PyCallable_DECL_CALL(AssignLabels);
    PyCallable_DECL_CALL(RemoveLabels);

    PyCallable_DECL_CALL(GetTitles);
    PyCallable_DECL_CALL(UpdateTitle);
    PyCallable_DECL_CALL(UpdateTitles);
    PyCallable_DECL_CALL(DeleteTitle);

    PyCallable_DECL_CALL(AddBulletin);
    PyCallable_DECL_CALL(GetBulletins);
    PyCallable_DECL_CALL(DeleteBulletin);

    PyCallable_DECL_CALL(GetLocationalRoles);
    PyCallable_DECL_CALL(GetRecentKillsAndLosses);
    PyCallable_DECL_CALL(UpdateMember);
    PyCallable_DECL_CALL(ExecuteActions);

    PyCallable_DECL_CALL(GetCorporateContacts);
    PyCallable_DECL_CALL(AddCorporateContact);
    PyCallable_DECL_CALL(EditContactsRelationshipID);
    PyCallable_DECL_CALL(RemoveCorporateContacts);
    PyCallable_DECL_CALL(EditCorporateContact);

    PyCallable_DECL_CALL(CreateAlliance);
    PyCallable_DECL_CALL(ApplyToJoinAlliance);
    PyCallable_DECL_CALL(DeleteAllianceApplication);
    PyCallable_DECL_CALL(GetAllianceApplications);
    PyCallable_DECL_CALL(GetSuggestedAllianceShortNames);

    PyCallable_DECL_CALL(GetMemberTrackingInfo);
    PyCallable_DECL_CALL(GetMembersPaged);
    PyCallable_DECL_CALL(GetMembersByIds);
    PyCallable_DECL_CALL(GetMemberIDsWithMoreThanAvgShares);
    PyCallable_DECL_CALL(GetMemberIDsByQuery);
    PyCallable_DECL_CALL(GetMemberTrackingInfoSimple);

    PyCallable_DECL_CALL(GetRentalDetailsPlayer);
    PyCallable_DECL_CALL(GetRentalDetailsCorp);

    PyCallable_DECL_CALL(UpdateCorporationAbilities);
    PyCallable_DECL_CALL(UpdateStationManagementSettings);
    PyCallable_DECL_CALL(GetNumberOfPotentialCEOs);

    PyCallable_DECL_CALL(CanLeaveCurrentCorporation);
    

protected:
    static void FillOCApplicationChange(OnCorporationApplicationChanged& OCAC, const Corp::ApplicationInfo& Old, const Corp::ApplicationInfo& New);

    bool GetSearchValues(int8 op, PyRep* rep, std::ostringstream& query);
    uint8 GetQueryType(std::string queryType);

    CorporationDB& m_db;

    Dispatcher *const m_dispatch;

    uint32 m_corpID;
};

#endif  // __EVEMU_CORP_CORPREGISTRY_BOUND_H_

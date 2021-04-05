/**
 * @name AllianceDB.h
 *      database methods for alliance data
 *
 * @author Allan
 * @date 24 May 2019
 *
 */


#ifndef EVE_ALLIANCE_ALLIANCEDB_H
#define EVE_ALLIANCE_ALLIANCEDB_H


#include "ServiceDB.h"

#include "corporation/CorpData.h"


class AllianceDB
{
public:

    PyRep* GetEmploymentRecord(uint32 corpID);
    void AddEmployment(uint32 allyID, uint32 corpID);

    PyRep* GetBulletins(uint32 allyID);
    void AddBulletin(uint32 allyID, uint32 ownerID, uint32 cCharID, std::string& title, std::string& body);
    void EditBulletin(uint32 bulletinID, uint32 eCharID, int64 eDataTime, std::string& title, std::string& body);

    PyRep *GetMyApplications(uint32 allyID);
    PyRep *GetApplications(uint32 allyID);

    PyObject *GetAlliance(uint32 allyID);

    PyRep* GetContacts(uint32 allyID);
    void AddContact(uint32 allyID);
    void UpdateContact(uint32 allyID);

    PyRep* GetLabels(uint32 allyID);
    void SetLabel(uint32 allyID, uint32 color, std::string name);
    void EditLabel(uint32 allyID, uint32 labelID, uint32 color, std::string name);
    void DeleteLabel(uint32 allyID, uint32 labelID);

    bool InsertApplication(Corp::ApplicationInfo& aInfo);
    bool UpdateApplication(const Corp::ApplicationInfo& aInfo);
    bool DeleteApplication(const Corp::ApplicationInfo& aInfo);
    bool GetCurrentApplicationInfo(uint32 allyID, uint32 corpID, Corp::ApplicationInfo& aInfo);

};

#endif  // EVE_ALLIANCE_ALLIANCEDB_H
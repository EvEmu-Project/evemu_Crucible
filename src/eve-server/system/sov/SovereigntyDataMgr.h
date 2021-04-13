
 /**
  * @name SovereigntyDataMgr.h
  *   memory object caching system for managing and saving ingame static and dynamic sovereignty data
  *
  * @Author:         James
  * @date:           21Apr13
  *
  */


#ifndef _EVE_SERVER_SOVEREIGNTY_DATAMANAGER_H__
#define _EVE_SERVER_SOVEREIGNTY_DATAMANAGER_H__


#include "../../eve-server.h"

#include "system/sov/SovereigntyDB.h"

class SovereigntyDataMgr
: public Singleton< SovereigntyDataMgr >
{
public:
    SovereigntyDataMgr();
    ~SovereigntyDataMgr();

    int                 Initialize();

    void                Clear();
    void                Close();
    void                GetInfo();
    /*
    void                AddOffice(uint32 stationID, OfficeData& data);
    void                LoadOffices(uint32 stationID, std::map<uint32, OfficeData>& data);
    // this will check for locationID type and retrieve appropriate data
    void                GetStationOfficeIDs(uint32 locationID, std::vector<OfficeData> &data);

    bool                GetStationData(uint32 stationID, StationData& data);

    PyRep*              GetStationItemBits(uint32 stationID);
    PyObject*           GetStationPyData(uint32 stationID);

    uint32              GetOwnerID(uint32 stationID);
    std::string         GetStationName(uint32 stationID);
    uint32              GetStationSystemID(uint32 stationID);

    double              GetDockPosY(uint32 stationID);

    uint32              GetOfficeRentalFee(uint32 stationID);
    uint32              GetOfficeIDForCorp(uint32 stationID, uint32 corpID);*/

protected:
    void                Populate();
    void                LoadSovPyData();


private:
    std::map<uint32, PyObject*>                         m_sovPyData;    // stationID/data
    std::map<uint32, SovereigntyData>                       m_sovData;          // stationID/data
};

//Singleton
#define svDataMgr \
    ( SovereigntyDataMgr::get() )


#endif  // _EVE_SERVER_SOVEREIGNTY_DATAMANAGER_H__


 /**
  * @name StationDataMgr.h
  *   memory object caching system for managing and saving ingame static and dynamic station data
  *
  * @Author:         Allan
  * @date:          12Dec17
  *
  */


#ifndef _EVE_SERVER_STATION_DATAMANAGER_H__
#define _EVE_SERVER_STATION_DATAMANAGER_H__


#include "../eve-server.h"
#include "POD_containers.h"

#include "station/StationDB.h"

class StationDataMgr
: public Singleton< StationDataMgr >
{
public:
    StationDataMgr();
    ~StationDataMgr();

    int                 Initialize();

    void                Clear();
    void                Close();
    void                GetInfo();
    void                AddOffice(uint32 stationID, OfficeData& data);
    void                LoadOffices(uint32 stationID, std::map<uint32, OfficeData>& data);
    // this will check for locationID type and retrieve appropriate data
    void                GetStationOfficeIDs(uint32 locationID, std::vector<OfficeData> &data);

    bool                GetStationData(uint32 stationID, StationData& data);
    uint32              GetStationServiceMask(uint32 stationID) { auto it = m_stationData.find(stationID); return it->second.serviceMask; }

    PyRep*              GetStationItemBits(uint32 stationID);
    PyObject*           GetStationPyData(uint32 stationID);

    uint32              GetOwnerID(uint32 stationID);
    std::string         GetStationName(uint32 stationID);
    uint32              GetStationSystemID(uint32 stationID);

    double              GetDockPosY(uint32 stationID);

    uint32              GetOfficeRentalFee(uint32 stationID);
    uint32              GetOfficeIDForCorp(uint32 stationID, uint32 corpID);

    void                AddOutpost(StationData& data);

protected:
    void                Populate();

    void                LoadStationPyData();


private:
    std::map<int8, int32>                               m_serviceMask;          // operationID/serviceMask
    std::map<uint32, StationData>                       m_stationData;          // stationID/data
    std::map<uint32, PyObject*>                         m_stationPyData;        // stationID/data

    std::multimap<uint32, OfficeData>                   m_stationOfficeData;    // stationID/data

    //  need to add station operation data here...

};

//Singleton
#define stDataMgr \
    ( StationDataMgr::get() )


#endif  // _EVE_SERVER_STATION_DATAMANAGER_H__

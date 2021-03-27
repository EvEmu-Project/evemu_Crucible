
/**
 * @name MapData.h
 *   a group of methods and functions to get map info.
 *     this is mostly used for getting random points in system, system jumps, and misc mission destination info
 * @Author:         Allan
 * @date:   13 November 2018
 */


#ifndef _EVE_MAP_MAPDATA_H_
#define _EVE_MAP_MAPDATA_H_

#include "../eve-server.h"

#include "../../eve-common/EVE_Missions.h"

class Agent;

class MapData
: public Singleton< MapData >
{
public:
    MapData();
    ~MapData();

    int                 Initialize();

    void                Clear();
    void                Close();

    void                GetInfo();

    PyTuple*            GetStationExtraInfo()           { PyIncRef(m_stationExtraInfo); return m_stationExtraInfo; }
    PyObject*           GetPseudoSecurities()           { PyIncRef(m_pseudoSecurities); return m_pseudoSecurities; }

    void GetMissionDestination(Agent* pAgent, uint8 misionType, MissionOffer& offer);

protected:
    void                Populate();

private:

    PyTuple*            m_stationExtraInfo;
    PyObject*           m_pseudoSecurities;

    std::multimap<uint32, uint32>        m_regionJumps;  //fromSys/toSys
    std::multimap<uint32, uint32>        m_constJumps;   //fromSys/toSys
    std::multimap<uint32, uint32>        m_systemJumps;  //fromSys/toSys

};

//Singleton
#define sMapData \
( MapData::get() )


#endif  // _EVE_MAP_MAPDATA_H_

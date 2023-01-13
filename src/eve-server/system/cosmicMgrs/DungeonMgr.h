 /**
  * @name DungeonMgr.cpp
  *     Dungeon management system for EVEmu
  *
  * @Author:        James
  * @date:          13 December 2022
  */
 

#ifndef _EVEMU_SYSTEM_DUNGEONMGR_H
#define _EVEMU_SYSTEM_DUNGEONMGR_H

#include <unordered_map>
#include "EVE_Dungeon.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"

//Shiny multi-index containers
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

// this class is a singleton object to have a common place for all in-memory dungeon data
class DungeonDataMgr
: public Singleton< DungeonDataMgr >
{
public:
    DungeonDataMgr();
    ~DungeonDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void GetRandomDungeon(Dungeon::Dungeon& dungeon, uint8 archetype);
    void UpdateDungeon(uint32 dungeonID);
    void GetDungeons(std::vector<Dungeon::Dungeon>& dunList);
    uint32 GetDungeonID()                               { return ++m_dungeonID; }
    const char* GetDungeonType(int8 typeID);

private:
    void Populate();
    void FillObject(DBResultRow row);
    void CreateDungeon(DBResultRow row);

    //Multi-index container for dungeon data
    typedef boost::multi_index_container<
        Dungeon::Dungeon,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<Dungeon::DungeonsByID>,
                boost::multi_index::member<Dungeon::Dungeon, uint32, &Dungeon::Dungeon::dungeonID>
                >,
            boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<Dungeon::DungeonsByArchetype>,
                boost::multi_index::member<Dungeon::Dungeon, uint8, &Dungeon::Dungeon::archetypeID>
                >>> DunDataContainer;

    DunDataContainer m_dungeons;
    uint32 m_dungeonID;
};

#define sDunDataMgr \
( DungeonDataMgr::get() )

/*  this class is in charge of creating/destroying and maintaining
 * dungeons in a system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */

class AnomalyMgr;
class SpawnMgr;
class EVEServiceManager;

class DungeonMgr {
public:
    DungeonMgr(SystemManager *system, EVEServiceManager& svc);
    ~DungeonMgr();

    bool Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr);
    void Process();
    void Load();

    bool MakeDungeon(CosmicSignature& sig);

    bool Create(uint32 templateID, CosmicSignature& sig);

protected:
    ManagerDB m_db;

    void CreateDeco(uint32 templateID, CosmicSignature& sig);

    /* we do not own any of these */
private:
    bool m_initalized;

    AnomalyMgr* m_anomMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    EVEServiceManager& m_services;

    int8 GetFaction(uint32 factionID);
    int8 GetRandLevel();

    std::map<uint32, std::vector<uint32>> m_dungeonList;  // this holds all items associated with the key 'dungeonID' in this system
};

#endif  // _EVEMU_SYSTEM_DUNGEONMGR_H
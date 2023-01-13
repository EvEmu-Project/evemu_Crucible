
 /**
  * @name DungeonMgr.h
  *     Dungeon management system for EVEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */



#ifndef _EVEMU_SYSTEM_DUNGEONMGR_H
#define _EVEMU_SYSTEM_DUNGEONMGR_H

#include <unordered_map>
#include "EVE_Dungeon.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"


/*
 * struct Dungeon::ActiveData {
 *    uint32 systemID;
 *    uint32 dungeonID;
 *    uint8 dunTemplateID;
 *    int64 dunExpiryTime;
 *    uint8 state;
 *    double x;
 *    double y;
 *    double z;
 * };
 */

// this class is a singleton object to have a common place for all dungeon template data
class DungeonDataMgr
: public Singleton< DungeonDataMgr >
{
public:
    DungeonDataMgr();
    ~DungeonDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void AddDungeon(Dungeon::ActiveData& dungeon);
    void GetDungeons(std::vector<Dungeon::ActiveData>& dunList);

    bool GetTemplate(uint32 templateID, Dungeon::Template& dTemplate);

    uint32 GetDungeonID()                               { return ++m_dungeonID; }

    const char* GetDungeonType(int8 typeID);

protected:
    typedef std::map<uint32, Dungeon::Template> DunTemplateDef;                       //templateID/data
    typedef std::unordered_multimap<uint32, Dungeon::ActiveData> ActiveDungeonDef;    //systemID/data
    typedef std::unordered_multimap<uint32, Dungeon::RoomData> DunRoomsDef;           //roomID/data
    typedef std::unordered_multimap<uint32, Dungeon::EntryData> DunEntryDef;          //entryID/data
    typedef std::unordered_multimap<uint32, Dungeon::GroupData> DunGroupsDef;         //groupID/data

    DunTemplateDef templates;         // templateID/data

public:
    ActiveDungeonDef activeDungeons;  // systemID/data
    DunEntryDef entrys;               // entryID/data
    DunRoomsDef rooms;                // roomID/data
    DunGroupsDef groups;              // groupID/data

private:
    void Populate();

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
class PyServiceMgr;

class DungeonMgr {
public:
    DungeonMgr(SystemManager *system, PyServiceMgr& svc);
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
    PyServiceMgr& m_services;

    int8 GetFaction(uint32 factionID);
    int8 GetRandLevel();

    void AddDecoToVector(uint8 dunType, uint32 templateID, std::vector<uint16>& groupVec);


    std::vector<Dungeon::GroupData> m_anomalyItems;

    std::map<uint32, std::vector<uint32>> m_dungeonList;  // this holds all items associated with the key 'dungeonID' in this system
};

#endif  // _EVEMU_SYSTEM_DUNGEONMGR_H
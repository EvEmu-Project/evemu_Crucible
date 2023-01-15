
 /**
  * @name CivilianMgr.h
  *     Civilian (non-combatant NPC) management system for EVEmu
  *
  * @Author:        Allan
  * @date:          12 Feb 2017
  *
  */



#ifndef EVEMU_SYSTEM_CIVILIANMGR_H_
#define EVEMU_SYSTEM_CIVILIANMGR_H_


#include "ServiceDB.h"
#include "utils/Singleton.h"

/* this class will control all aspects of
 * non-combatant civilians
 */

class EVEServiceManager;

class CivilianMgr
: public Singleton<CivilianMgr>
{
public:
    CivilianMgr();
    ~CivilianMgr()                              { /* do nothing here */ }

    void Initialize();
    void Process();

private:
    ServiceDB* m_db;

    bool m_initalized;

};

//Singleton
#define sCivMgr \
( CivilianMgr::get() )


#endif  // EVEMU_SYSTEM_CIVILIANMGR_H_

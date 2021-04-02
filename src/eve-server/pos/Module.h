
/**
 * @name Module.h
 *   General Class for miscellanous POS Modules.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */


#ifndef EVEMU_POS_MODULE_H_
#define EVEMU_POS_MODULE_H_


#include "DataClasses.h"
#include "pos/Structure.h"


class ModuleSE
: public StructureSE
{
public:
    ModuleSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~ModuleSE()                                 { /* do nothing here */ }

    /* class type pointer querys. */
    virtual ModuleSE*           GetModuleSE()           { return this; }

    /* class type tests. */
    virtual bool                IsModuleSE()            { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();


};


class ReactorSE
: public StructureSE
{
public:
    ReactorSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~ReactorSE();

    /* class type pointer querys. */
    virtual ReactorSE*          GetReactorSE()          { return this; }

    /* class type tests. */
    virtual bool                IsReactorSE()           { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();
    virtual void                InitData();


private:
    ReactorData*                pData; //ReactorData class

};


#endif  // EVEMU_POS_MODULE_H_

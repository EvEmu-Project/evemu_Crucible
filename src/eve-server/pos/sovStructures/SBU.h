/**
 * @name SBU.h
 *   Class for Sovereignty Blockade Units.
 *
 * @Author:         James
 * @date:   8 April 2021
 */


#ifndef EVEMU_POS_SBU_H_
#define EVEMU_POS_SBU_H_

#include "pos/Structure.h"

class SBUSE
: public StructureSE
{
public:
    SBUSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData);
    virtual ~SBUSE()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual SBUSE*              GetSBUSE()            { return this; }
    virtual bool                isGlobal()              { return true; }
    virtual bool                IsPOSSE()               { return false; }
    
    /* class type tests. */
    virtual bool                IsSBUSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();


protected:

private:

};

#endif  // EVEMU_POS_SBU_H_

/**
 * @name TCU.h
 *   Class for Territorial Claim Units.
 *
 * @Author:         James
 * @date:   8 April 2021
 */


#ifndef EVEMU_POS_TCU_H_
#define EVEMU_POS_TCU_H_

#include "pos/Structure.h"

class TCUSE
: public StructureSE
{
public:
    TCUSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData);
    virtual ~TCUSE()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual TCUSE*              GetTCUSE()            { return this; }

    /* class type tests. */
    /* Base */
    //virtual bool                isGlobal()              { return true; }
    virtual bool                IsTCUSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                SetOnline();
    virtual void                SetOffline();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();


protected:

private:

};

#endif  // EVEMU_POS_TCU_H_

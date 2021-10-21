
/**
 * @name JumpBridge.h
 *   Class for POS JumpBridge Modules.
 *
 * @Author:           James
 * @date:   13 October 2021
 */


#ifndef EVEMU_POS_JUMPBRIDGE_H_
#define EVEMU_POS_JUMPBRIDGE_H_


#include "pos/Structure.h"


class JumpBridgeSE
: public StructureSE
{
public:
    JumpBridgeSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~JumpBridgeSE()                                { /* do nothing here */ }

    /* class type pointer querys. */
    virtual JumpBridgeSE*       GetJumpBridgeSE()          { return this; }

    /* class type tests. */
    virtual bool                IsJumpBridgeSE()           { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                SetOnline();
    virtual void                SetOffline();

    /* virtual functions default to base class and overridden as needed */
    virtual void Init();

};

#endif  // EVEMU_POS_JUMPBRIDGE_H_

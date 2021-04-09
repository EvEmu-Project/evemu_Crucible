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
    virtual ~SBUSE();

    /* class type pointer querys. */
    virtual SBUSE*              GetSBUSE()            { return this; }

    /* class type tests. */
    virtual bool                IsSBUSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();
    virtual void                InitData();

    /* basic SBU methods */
    virtual void                SetOnline();
    virtual void                SetOffline();

    virtual void                Online();
    virtual void                Operating();
    virtual void                Reinforced();

    virtual void                Scoop();

    void                        ReinforceSBU();

    void UpdatePassword();
    void SetUseFlags(uint32 itemID, int8 view=0, int8 take=0, int8 use=0);

    uint16 GetSOI()                                     { return m_soi; }

protected:

    SystemEntity* m_pShieldSE;

private:
    uint16 m_soi;   // Sphere Of Influence, 45km max

    std::map<uint32, StructureSE*> m_structs;  // structID/pSSE

};

#endif  // EVEMU_POS_SBU_H_

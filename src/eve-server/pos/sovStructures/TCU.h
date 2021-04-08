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
    virtual ~TCUSE();

    /* class type pointer querys. */
    virtual TCUSE*              GetTCUSE()            { return this; }

    /* class type tests. */
    virtual bool                IsTCUSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Init();
    virtual void                InitData();

    /* basic TCU methods */
    virtual void                SetOnline();
    virtual void                SetOffline();

    virtual void                Online();
    virtual void                Operating();
    virtual void                Reinforced();

    virtual void                Scoop();

    void                        ReinforceTCU();

    /* tower data methods */
    PyRep* GetDeployFlags();
    PyRep* GetUsageFlagList();
    PyRep* GetProcessInfo();

    void UpdatePassword();
    void SetDeployFlags(int8 anchor=0, int8 unanchor=0, int8 online=0, int8 offline=0);
    void SetUseFlags(uint32 itemID, int8 view=0, int8 take=0, int8 use=0);

    /* general tower data methods */
    void GetTCUData(EVEPOS::TCUData& tdata)         { tdata = m_tdata; }

    uint16 GetSOI()                                     { return m_soi; }

protected:
    EVEPOS::TCUData m_tdata;

    SystemEntity* m_pShieldSE;

private:
    uint16 m_soi;   // Sphere Of Influence, 45km max

    std::map<uint32, StructureSE*> m_structs;  // structID/pSSE

};

#endif  // EVEMU_POS_TCU_H_


 /**
  * @name CynoModule.h
  *   Cynosural field generator module class
  * @Author: James
  * @date:   8 October 2021
  */

#ifndef _EVE_SHIP_MODULES_CYNO_MODULE_H_
#define _EVE_SHIP_MODULES_CYNO_MODULE_H_

#include "ship/modules/ActiveModule.h"
#include "system/SystemEntity.h"
#include "Client.h"

class CynoModule: public ActiveModule
{
public:
    CynoModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~CynoModule()                                 { /* do nothing here */ }

    virtual CynoModule*       GetCynoModule()             { return this; }
    virtual bool        IsCynoModule() const        { return true; }

    /* ActiveModule overrides */
    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);
    virtual void DeactivateCycle(bool abort=false);
    virtual uint32      DoCycle();

    // this is a check for those active modules that need it (mining, weapons) and overridden as needed
    virtual bool CanActivate();

private:
    Client* pClient;
    ShipSE* pShipSE;
    SystemEntity* cSE;

    bool m_firstRun;

    float m_shipVelocity;

    EVEItemFlags m_holdFlag;

    void SendOnJumpBeaconChange(bool status=false);
    void CreateCyno();
};

#endif  //_EVE_SHIP_MODULES_CYNO_MODULE_H_

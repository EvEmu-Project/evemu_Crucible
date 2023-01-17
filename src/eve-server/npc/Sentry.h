 /**
  * @name Sentry.h
  *   Sentry class
  * @Author:    Allan
  * @date:      23 April 2017
  */

#ifndef __NPC_SENTRY_H
#define __NPC_SENTRY_H

#include "system/SystemEntity.h"

class EVEServiceManager;
class DestinyManager;
class InventoryItem;
class SentryAI;
class SystemManager;
class ServiceDB;

class Sentry
: public ObjectSystemEntity
{
public:
    Sentry(InventoryItemRef self, EVEServiceManager& services, SystemManager* system, const FactionData& data);
    virtual ~Sentry();

    /* class type pointer querys. */
    virtual Sentry* GetSentrySE()                       { return this; }
    /* class type tests. */
    virtual bool IsSentrySE()                           { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void TargetLost(SystemEntity* who);
    virtual void TargetedAdd(SystemEntity* who);
    virtual void EncodeDestiny(Buffer& into);

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &damage);

    /* specific functions handled here. */
    void SaveSentry();
    void RemoveSentry();
    void SetResists();

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }

    SentryAI* GetAIMgr()                                { return m_AI; }

protected:
    SentryAI* m_AI;

private:
    float m_emDamage = 0;
    float m_expDamage = 0;
    float m_kinDamage = 0;
    float m_therDamage = 0;
    float m_hullDamage = 0;
    float m_armorDamage = 0;
    float m_shieldCharge = 0;
    float m_shieldCapacity = 0;
};

#endif  // __NPC_SENTRY_H

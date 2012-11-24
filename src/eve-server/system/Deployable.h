/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Aknor Jaden
*/

#ifndef __DEPLOYABLE__H__INCL__
#define __DEPLOYABLE__H__INCL__

#include "system/SystemEntity.h"

/**
 * DynamicSystemEntity which represents structure object in space
 */
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;

class DeployableEntity
: public DynamicSystemEntity
{
public:
    DeployableEntity(
        InventoryItemRef structure,
        SystemManager *system,
        PyServiceMgr &services,
        const GPoint &position);

    /*
     * Primary public interface:
     */
    InventoryItemRef GetStructureObject() { return _deployableRef; }
    DestinyManager * GetDestiny() { return m_destiny; }
    SystemManager * GetSystem() { return m_system; }

    /*
     * Public fields:
     */

    inline double x() const { return(GetPosition().x); }
    inline double y() const { return(GetPosition().y); }
    inline double z() const { return(GetPosition().z); }

    //SystemEntity interface:
    virtual EntityClass GetClass() const { return(ecDeployableEntity); }
    virtual bool IsDeployableEntity() const { return true; }
    virtual DeployableEntity *CastToDeployableEntity() { return(this); }
    virtual const DeployableEntity *CastToDeployableEntity() const { return(this); }
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into ) const;
    virtual void TargetAdded(SystemEntity *who) {}
    virtual void TargetLost(SystemEntity *who) {}
    virtual void TargetedAdd(SystemEntity *who) {}
    virtual void TargetedLost(SystemEntity *who) {}
    virtual void TargetsCleared() {}
    virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
    virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}
    virtual uint32 GetCorporationID() const { return(1); }
    virtual uint32 GetAllianceID() const { return(0); }
    virtual void Killed(Damage &fatal_blow);
    virtual SystemManager *System() const { return(m_system); }

    void ForcedSetPosition(const GPoint &pt);

    virtual bool ApplyDamage(Damage &d);
    virtual void MakeDamageState(DoDestinyDamageState &into) const;

    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

protected:
    /*
     * Member functions
     */
    void _ReduceDamage(Damage &d);
    void ApplyDamageModifiers(Damage &d, SystemEntity *target);

    /*
     * Member fields:
     */
    SystemManager *const m_system;    //we do not own this
    PyServiceMgr &m_services;    //we do not own this
    InventoryItemRef _deployableRef;   // We don't own this

    /* Used to calculate the damages on NPCs
     * I don't know why, npc->Set_shieldCharge does not work
     * calling npc->shieldCharge() return the complete shield
     * So we get the values on creation and use then instead.
    */
    double m_shieldCharge;
    double m_armorDamage;
    double m_hullDamage;
};

#endif /* !__DEPLOYABLE__H__INCL__ */



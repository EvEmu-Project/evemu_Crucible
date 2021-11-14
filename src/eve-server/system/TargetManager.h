/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
    Updates:    Allan
*/


#ifndef __TARGETMANAGER_H_INCL__
#define __TARGETMANAGER_H_INCL__

#include "inventory/ItemRef.h"

namespace TargMgr {
    namespace State {
        enum {
            Idle                = 0,
            Locking             = 1,
            Passive             = 2,
            Locked              = 3
        };
    }
    namespace Mode {
        enum {
            None                = 0,
            Add                 = 1,
            Lost                = 2,
            Clear               = 3,
            OtherAdd            = 4,
            OtherLost           = 5,
            LockedBy            = 6
        };
    }
    namespace Msg {
        enum {
            NoMsg               = 0,
            Locked              = 1,
            Docking             = 2,
            Jumping             = 3,
            Cloaked             = 4,
            WarpingOut          = 5,
            StoppedTargeting    = 6,
            Destroyed           = 7,
            ClientReq           = 8,
            InternalCall        = 9,
            Deleted             = 10,
            Shutdown            = 11
        };
    }
}


class MiningLaser;
class ActiveModule;
class SystemEntity;
class PyRep;
class PyTuple;
class PyList;

class TargetManager {
public:
    TargetManager(SystemEntity* self);
    ~TargetManager()                            { /* do nothing here */ }

    /* Common Methods for all objects */
    bool                Process();
    void                Unload();       // called on npcs from sysMgr when unloading system.

    // iterate thru the map of modules targeting this object and call AbortCycle on each.
    void                ClearModules();
    void                TargetsCleared();
    void                ClearFromTargets();
    void                TargetLost(SystemEntity *tSE);
    void                ClearTarget(SystemEntity *tSE);
    void                TargetAdded(SystemEntity *tSE);
    void                TargetedAdd(SystemEntity *tSE);
    void                TargetedLost(SystemEntity *tSE);
    void                ClearTargets(bool notify=true);
    void                ClearAllTargets(bool notify=true);

    /* method to remove target without triggering anything else (target destroyed) */
    void                RemoveTarget(SystemEntity* tSE);

    /*
     *    OnTarget.mode
     *        add - targeting successful
     *        clear - clear all targets
     *        lost - target lost (reason not used)
     *            - Docking
     *            - Cloaked
     *        otheradd - somebody else has targeted you
     *        otherlost - somebody else has stopped targeting you (reason not used)
     *            - WarpingOut
     *            - StoppedTargeting
     *            - Destroyed
     *
     *    OnTargetClear - immediately removes all target info from ship, including pending targets
     *        - this is done automagically when client jump, dock, or warp.  we just clean up our side
     */
    //void                OnTarget(SystemEntity* tSE, uint8 mode=TargMgr::Mode::None, uint8 msg=TargMgr::Msg::NoMsg);
    //void                ClearTargets(uint8 msg=TargMgr::Msg::NoMsg);
    // notify targeters this entity is gone
    //void                ClearFromTargets(bool update=true, uint8 msg=TargMgr::Msg::NoMsg);


    // called by player ships
    bool                StartTargeting(SystemEntity* tSE, ShipItemRef sRef);

    /* NPC AI Methods */
    bool                IsTargetedBy(SystemEntity *pSE);
    SystemEntity*       GetFirstTarget(bool need_locked=false);
    SystemEntity*       GetTarget(uint32 targetID, bool need_locked=true) const;

    // called by npc
    bool                StartTargeting(SystemEntity* who, float lockTime, uint8 maxLockedTargets, double maxTargetLockRange, bool& chase);

    bool                CanAttack()                     { return m_canAttack; }
    bool                HasNoTargets() const            { return m_targets.empty(); }

    /* PC Module Methods (for module deactivation on target removed) */
    void                Destroyed();    // this does NOT remove target from targeters map
    // only called by MiningLaser
    void                Depleted(MiningLaser* pMod);
    // only called by non-lasers
    void                Depleted(InventoryItemRef iRef);
    void                AddTargetModule(ActiveModule* pMod);
    void                RemoveTargetModule(ActiveModule* pMod);

    /* Packet builders: */
    PyList*             GetTargets() const;
    PyList*             GetTargeters() const;

    void                QueueEvent(PyTuple **up) const;    //queue an event to all SEs targeting me.
    void                QueueUpdate(PyTuple **up) const;   //queue an update to all SEs targeting me.

    /* debugging methods */
    void                Dump() const;
    // called by .targlist (player command)
    std::string         TargetList(uint16 &length, uint16 &count);


protected:
    float               TimeToLock(ShipItemRef sRef, SystemEntity* tSE) const;

    static const char*  GetModeName(uint8 mode);
    static const char*  GetStateName(uint8 state);

    //called in reaction to outgoing targeting events in other target managers.
    void                TargetedByLocked(SystemEntity *tSE);
    void                TargetedByLost(SystemEntity *tSE);

    class TargetEntry {
    public:
        TargetEntry()
        : state(TargMgr::State::Idle), timer(0) {}

        void Dump(SystemEntity* pSE) const;

        uint8 state;

        Timer timer;
    };

    class TargetedByEntry {
    public:
        TargetedByEntry()
        : state(TargMgr::State::Idle) {}

        void Dump(SystemEntity* pSE) const;

        uint8 state;
    };

private:
    SystemEntity* mySE;    //we do not own this.

    std::map<uint32, ActiveModule*> m_modules;               // map of modID/Mod* targeting this object
    std::map<SystemEntity*, TargetEntry*> m_targets;         //we own these values, not the keys
    std::map<SystemEntity*, TargetedByEntry*> m_targetedBy;  //we own these values, not the keys

    bool m_canAttack;   // true if npcs can begin attack (to correct attacking before targetlock)
};

#endif

/*{'messageKey': 'TargetByOther', 'dataID': 2040886, 'suppressable': False, 'bodyID': None, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1612}
{'messageKey': 'TargetCantValidate', 'dataID': 17881380, 'suppressable': False, 'bodyID': 258753, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1613}
{'messageKey': 'TargetElectronicsScanStrengthsIncompatible', 'dataID': 17881494, 'suppressable': False, 'bodyID': 258794, 'messageType': 'notify', 'urlAudio': 'wise:/msg_TargetElectronicsScanStrengthsIncompatible_play', 'urlIcon': '', 'titleID': None, 'messageID': 1614}
{'messageKey': 'TargetExploding', 'dataID': 17881630, 'suppressable': False, 'bodyID': 258842, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1615}
{'messageKey': 'TargetExploding2', 'dataID': 38244390, 'suppressable': False, 'bodyID': 264194, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3798}
{'messageKey': 'TargetJammedBy', 'dataID': 17881749, 'suppressable': False, 'bodyID': 258883, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1616}
{'messageKey': 'TargetJammedOtherBy', 'dataID': 17881755, 'suppressable': False, 'bodyID': 258885, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1617}
{'messageKey': 'TargetJammedSuccess', 'dataID': 17881752, 'suppressable': False, 'bodyID': 258884, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1618}
{'messageKey': 'TargetJammingFullSuccess', 'dataID': 17881651, 'suppressable': False, 'bodyID': 258850, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1619}
{'messageKey': 'TargetJammingPartialSuccess', 'dataID': 17881657, 'suppressable': False, 'bodyID': 258852, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1620}
{'messageKey': 'TargetJammingWithoutTargeters', 'dataID': 17881660, 'suppressable': False, 'bodyID': 258853, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1621}
u'TargetJammingWithoutTargetersBody'}(u'As nothing is targeting you at this time, your {[item]moduleID.name} failed to jam anything.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
{'messageKey': 'TargetLocked', 'dataID': 2987151, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': 'wise:/msg_TargetLocked_play', 'urlIcon': '', 'titleID': None, 'messageID': 1622}
{'messageKey': 'TargetLocking', 'dataID': 2987152, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': 'wise:/msg_TargetLocking_play', 'urlIcon': '', 'titleID': None, 'messageID': 1623}
{'messageKey': 'TargetLost', 'dataID': 17881497, 'suppressable': False, 'bodyID': 258795, 'messageType': 'notify', 'urlAudio': 'wise:/msg_TargetLost_play', 'urlIcon': '', 'titleID': None, 'messageID': 1624}
{'messageKey': 'TargetLostNotWithinRange', 'dataID': 17881663, 'suppressable': False, 'bodyID': 258854, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1625}
u'TargetLostNotWithinRangeBody'}(u'The {[item]targetGroupID.name} is too far away, you need to be within {[numeric]desiredRange} meters of it but are actually {[numeric]actualRange} meters away.', None, {u'{[numeric]desiredRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredRange'}, u'{[numeric]actualRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'actualRange'}, u'{[item]targetGroupID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetGroupID'}})
{'messageKey': 'TargetNoLongerPresent', 'dataID': 17881666, 'suppressable': False, 'bodyID': 258855, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1626}
u'TargetNoLongerPresentBody'}(u'{[item]moduleID.name} deactivates as the {[item]targetID.name} it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
{'messageKey': 'TargetNoLongerPresentGeneric', 'dataID': 17875297, 'suppressable': False, 'bodyID': 256459, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3742}
u'TargetNoLongerPresentGenericBody'}(u'{[item]moduleID.name} deactivates as the item it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
{'messageKey': 'TargetNotLocked', 'dataID': 17881675, 'suppressable': False, 'bodyID': 258858, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1627}
{'messageKey': 'TargetNotLocked2', 'dataID': 38244393, 'suppressable': False, 'bodyID': 264195, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3799}
{'messageKey': 'TargetNotShip', 'dataID': 17881383, 'suppressable': False, 'bodyID': 258754, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1628}
{'messageKey': 'TargetNotWithinRange', 'dataID': 17881787, 'suppressable': False, 'bodyID': 258896, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1629}
{'messageKey': 'TargetNotWithinRangeGeneric', 'dataID': 17875391, 'suppressable': False, 'bodyID': 256493, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3744}
{'messageKey': 'TargetObscured', 'dataID': 17881678, 'suppressable': False, 'bodyID': 258859, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1630}
{'messageKey': 'TargetOutOfRangeFar', 'dataID': 17881681, 'suppressable': False, 'bodyID': 258860, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1631}
{'messageKey': 'TargetTooFar', 'dataID': 17881689, 'suppressable': False, 'bodyID': 258863, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1632}
{'messageKey': 'Targeting', 'dataID': 17881919, 'suppressable': False, 'bodyID': 258942, 'messageType': 'notify', 'urlAudio': 'wise:/msg_Targeting_play', 'urlIcon': 'res:/ui/icon/misc/targeting.blue', 'titleID': None, 'messageID': 1633}
{'messageKey': 'TargetingAptitudeAlreadyFullyUtilized', 'dataID': 17881692, 'suppressable': False, 'bodyID': 258864, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1634}
{'messageKey': 'TargetingMissileToSelf', 'dataID': 17881695, 'suppressable': False, 'bodyID': 258865, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1635}
{'messageKey': 'TargetingSystemsAlreadyFullyUtilized', 'dataID': 17881704, 'suppressable': False, 'bodyID': 258868, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1636}
{'messageKey': 'TargetingSystemsBeingJammed', 'dataID': 17881758, 'suppressable': False, 'bodyID': 258886, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1637}
{'messageKey': 'TargetingSystemsNotInstalled', 'dataID': 17881386, 'suppressable': False, 'bodyID': 258755, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1638}

{'FullPath': u'UI/Messages', 'messageID': 258753, 'label': u'TargetCantValidateBody'}(u'Targeting attempt failed as the designated object is no longer present.', None, None)
{'FullPath': u'UI/Messages', 'messageID': 258754, 'label': u'TargetNotShipBody'}(u'The target is not a ship.', None, None)
{'FullPath': u'UI/Messages', 'messageID': 258755, 'label': u'TargetingSystemsNotInstalledBody'}(u'The ship you are piloting does not have targeting systems installed.', None, None)
{'FullPath': u'UI/Messages', 'messageID': 258794, 'label': u'TargetElectronicsScanStrengthsIncompatibleBody'}(u'Unable to modify the scan strengths of the target because one or more is higher than your own preventing the process.', None, None)
{'FullPath': u'UI/Messages', 'messageID': 258795, 'label': u'TargetLostBody'}(u'Target is lost', None, None)
{'FullPath': u'UI/Messages', 'messageID': 258850, 'label': u'TargetJammingFullSuccessBody'}(u'Your {[item]moduleID.name} managed to jam all of the objects targeting you: {jammedNames}.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{jammedNames}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammedNames'}})
{'FullPath': u'UI/Messages', 'messageID': 258853, 'label': u'TargetJammingWithoutTargetersBody'}(u'As nothing is targeting you at this time, your {[item]moduleID.name} failed to jam anything.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
{'FullPath': u'UI/Messages', 'messageID': 258854, 'label': u'TargetLostNotWithinRangeBody'}(u'The {[item]targetGroupID.name} is too far away, you need to be within {[numeric]desiredRange} meters of it but are actually {[numeric]actualRange} meters away.', None, {u'{[numeric]desiredRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredRange'}, u'{[numeric]actualRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'actualRange'}, u'{[item]targetGroupID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetGroupID'}})
{'FullPath': u'UI/Messages', 'messageID': 258855, 'label': u'TargetNoLongerPresentBody'}(u'{[item]moduleID.name} deactivates as the {[item]targetID.name} it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
{'FullPath': u'UI/Messages', 'messageID': 258858, 'label': u'TargetNotLockedBody'}(u'{[item]moduleID.name} deactivates because its target, {[item]targetID}, is not locked.', None, {u'{[item]targetID}': {'conditionalValues': [], 'variableType': 2, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}, u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
{'FullPath': u'UI/Messages', 'messageID': 258859, 'label': u'TargetObscuredBody'}(u'No line of sight to {name}.', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}})
{'FullPath': u'UI/Messages', 'messageID': 258860, 'label': u'TargetOutOfRangeFarBody'}(u'{targetname} is too far away to target.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}})
{'FullPath': u'UI/Messages', 'messageID': 258864, 'label': u'TargetingAptitudeAlreadyFullyUtilizedBody'}(u'You are already managing {[numeric]limit, decimalPlaces=0} targets, as many as you have skill to.', None, {u'{[numeric]limit, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'limit'}})
{'FullPath': u'UI/Messages', 'messageID': 258865, 'label': u'TargetingMissileToSelfBody'}(u'It is not a good idea to shoot a missile at yourself. Code {shipID} {targetID}.', None, {u'{shipID}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipID'}, u'{targetID}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
{'FullPath': u'UI/Messages', 'messageID': 258868, 'label': u'TargetingSystemsAlreadyFullyUtilizedBody'}(u"You are already managing {[numeric]limit, decimalPlaces=0} targets, as many as your ship's electronics are capable of.", None, {u'{[numeric]limit, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'limit'}})
{'FullPath': u'UI/Messages', 'messageID': 258883, 'label': u'TargetJammedByBody'}(u'{jammerShipName} has started trying to target jam you.', None, {u'{jammerShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammerShipName'}})
{'FullPath': u'UI/Messages', 'messageID': 258884, 'label': u'TargetJammedSuccessBody'}(u'You have started trying to target jam {jammedShipName}.', None, {u'{jammedShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammedShipName'}})
{'FullPath': u'UI/Messages', 'messageID': 258885, 'label': u'TargetJammedOtherByBody'}(u'{jammerShipName} has started trying to target jam {jammedShipName}.', None, {u'{jammedShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammedShipName'}, u'{jammerShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammerShipName'}})
{'FullPath': u'UI/Messages', 'messageID': 258886, 'label': u'TargetingSystemsBeingJammedBody'}(u'Unable to target {targetname} as your targeting systems are currently being jammed.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}})
{'FullPath': u'UI/Messages', 'messageID': 258896, 'label': u'TargetNotWithinRangeBody'}(u'The {targetGroupName} is too far away, you need to be within {[numeric]desiredRange, decimalPlaces=0} meters of it but are actually {[numeric]actualDistance, decimalPlaces=0} meters away.', None, {u'{targetGroupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetGroupName'}, u'{[numeric]desiredRange, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'desiredRange'}, u'{[numeric]actualDistance, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'actualDistance'}})
{'FullPath': u'UI/Messages', 'messageID': 258942, 'label': u'TargetingBody'}(u'{name} targeted', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}})
{'FullPath': u'UI/Messages', 'messageID': 264194, 'label': u'TargetExploding2Body'}(u'{[item]moduleTypeID.name} deactivates as {targetName} begins to explode.', None, {u'{[item]moduleTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleTypeID'}, u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
{'FullPath': u'UI/Messages', 'messageID': 264195, 'label': u'TargetNotLocked2Body'}(u'{[item]moduleID.name} deactivates because its target, {targetName}, is not locked.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
{'FullPath': u'UI/Messages', 'messageID': 258842, 'label': u'TargetExplodingBody'}(u'{[item]moduleTypeID.name} deactivates as {[item]targetID.name} begins to explode.', None, {u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}, u'{[item]moduleTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleTypeID'}})

*/

/* {'messageKey': 'DeniedDroneTargetForceField', 'dataID': 17877756, 'suppressable': False, 'bodyID': 257373, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2880}
 * {'messageKey': 'DeniedInvulnerable', 'dataID': 17883409, 'suppressable': False, 'bodyID': 259494, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 779}
 * {'messageKey': 'DeniedStationInvulnerableSovereign', 'dataID': 17883713, 'suppressable': False, 'bodyID': 259600, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 780}
 * {'messageKey': 'DeniedTargetAfterCloak', 'dataID': 17883412, 'suppressable': False, 'bodyID': 259495, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 781}
 * {'messageKey': 'DeniedTargetEvadesSensors', 'dataID': 17883870, 'suppressable': False, 'bodyID': 259657, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 782}
 * {'messageKey': 'DeniedTargetForceField', 'dataID': 17883882, 'suppressable': False, 'bodyID': 259661, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 783}
 * {'messageKey': 'DeniedTargetInvulnerable', 'dataID': 17883415, 'suppressable': False, 'bodyID': 259496, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 784}
 * {'messageKey': 'DeniedTargetOtherFrozen', 'dataID': 17883876, 'suppressable': False, 'bodyID': 259659, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 785}
 * {'messageKey': 'DeniedTargetOtherWarping', 'dataID': 17883809, 'suppressable': False, 'bodyID': 259635, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 786}
 * {'messageKey': 'DeniedTargetReinforcedStructure', 'dataID': 17883888, 'suppressable': False, 'bodyID': 259663, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 787}
 * {'messageKey': 'DeniedTargetSelf', 'dataID': 17883418, 'suppressable': False, 'bodyID': 259497, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 788}
 * {'messageKey': 'DeniedTargetSelfFrozen', 'dataID': 17883873, 'suppressable': False, 'bodyID': 259658, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 789}
 * {'messageKey': 'DeniedTargetSelfWarping', 'dataID': 17883879, 'suppressable': False, 'bodyID': 259660, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 790}
 * {'messageKey': 'DeniedTargetUntargetable', 'dataID': 17880323, 'suppressable': False, 'bodyID': 258348, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2170}
 * {'messageKey': 'DeniedTargetingAttemptFailed', 'dataID': 17883942, 'suppressable': False, 'bodyID': 259683, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 791}
 * {'messageKey': 'DeniedTargetingCloaked', 'dataID': 17883664, 'suppressable': False, 'bodyID': 259583, 'messageType': 'notify', 'urlAudio': 'wise:/msg_DeniedTargetingCloaked_play', 'urlIcon': '', 'titleID': None, 'messageID': 792}
 * {'messageKey': 'DeniedTargetingInsideField', 'dataID': 17883885, 'suppressable': False, 'bodyID': 259662, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 793}
 * {'messageKey': 'DeniedTargetingTargetCloaked', 'dataID': 17883421, 'suppressable': False, 'bodyID': 259498, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 794}
 * {'FullPath': u'UI/Messages', 'messageID': 259494, 'label': u'DeniedInvulnerableBody'}(u'Your ship is realigning its magnetic field, please wait a moment.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259495, 'label': u'DeniedTargetAfterCloakBody'}(u'You cannot perform that action at this time as your systems are still recalibrating after the use of a cloaking device.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259496, 'label': u'DeniedTargetInvulnerableBody'}(u'Target is invulnerable.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259497, 'label': u'DeniedTargetSelfBody'}(u'You cannot target your own ship.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259498, 'label': u'DeniedTargetingTargetCloakedBody'}(u'You failed to target nothing.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257373, 'label': u'DeniedDroneTargetForceFieldBody'}(u'Your drones cannot engage {[item]target.name} since it is within the range of a forcefield you are barred from entering.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259600, 'label': u'DeniedStationInvulnerableSovereignBody'}(u'{[item]module.name} deactivates as the target {targetname} is invulnerable because its owning alliance holds sovereignty in this solar system.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}, u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258348, 'label': u'DeniedTargetUntargetableBody'}(u'You are unable to target {targetName} as it has been made untargetable by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259583, 'label': u'DeniedTargetingCloakedBody'}(u'You cannot target anything while you are cloaked.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259635, 'label': u'DeniedTargetOtherWarpingBody'}(u"Interference from {targetName}'s warp prevents your sensors from locking the target.", None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259657, 'label': u'DeniedTargetEvadesSensorsBody'}(u'You are unable to target the {targetName} as your sensors are unable to lock onto it.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259658, 'label': u'DeniedTargetSelfFrozenBody'}(u'You are unable to target {targetName} because you have been frozen by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259659, 'label': u'DeniedTargetOtherFrozenBody'}(u'You are unable to target {targetName} because they are currently frozen by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259660, 'label': u'DeniedTargetSelfWarpingBody'}(u'Interference from the warp you are doing is preventing your sensors from getting a target lock on {targetName}.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259661, 'label': u'DeniedTargetForceFieldBody'}(u'You failed to target {[item]target.name}, they are within range {[numeric]range.distance} of a {[item]item.name} and with you being outside of it, it is preventing you from holding a lock on them.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]range.distance}': {'conditionalValues': [], 'variableType': 9, 'propertyName': 'distance', 'args': 256, 'kwargs': {}, 'variableName': 'range'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259662, 'label': u'DeniedTargetingInsideFieldBody'}(u'You cannot target the {[item]target.name} while you are inside a force field.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259663, 'label': u'DeniedTargetReinforcedStructureBody'}(u'You failed to target {[item]target.name} as it is locked down in reinforced mode.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259683, 'label': u'DeniedTargetingAttemptFailedBody'}(u'Your attempt to target {[item]target.name} failed.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 *
 */

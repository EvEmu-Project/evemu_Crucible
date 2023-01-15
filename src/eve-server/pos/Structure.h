
/**
 * @name Structure.h
 *   Generic Base Class for POS items and entities.
 *
 * @Author:         Allan
 * @date:   unknown
 */


#ifndef EVEMU_POS_STRUCTURE_H_
#define EVEMU_POS_STRUCTURE_H_

#include "../eve-server.h"


#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryItem.h"
#include "pos/PosMgrDB.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"

// TODO: We may need to create StructureTypeData and StructureType classes just as Ship.h/Ship.cpp
// has in order to load up type data specific to structures.  For now, the generic ItemType class is used.

/**
 * InventoryItem which represents Structure.
 */
class StructureItem
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us

public:
    static StructureItemRef Load( uint32 structureID);
    static StructureItemRef Spawn( ItemData &data);

    virtual void            Delete();
    virtual void            Rename(std::string name);
    virtual void            AddItem(InventoryItemRef iRef);
    virtual void            RemoveItem(InventoryItemRef iRef);

    /** @todo (Allan)  will probably need to rewrite all these, too.
     * Public fields:
     */
    const ItemType &    type() const { return InventoryItem::type(); }

    PyObject *StructureGetInfo();

    void SetMySE(SystemEntity* pSE)                     { mySE = pSE; }
    SystemEntity* GetMySE()                             { return mySE; }


protected:
    StructureItem(uint32 _structureID, const ItemType &_itemType, const ItemData &_data);
    virtual ~StructureItem();

    using InventoryItem::_Load;
    virtual bool _Load();

    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 structureID, const ItemType &type, const ItemData &data)
    {
        if ((type.categoryID() != EVEDB::invCategories::Structure)
        and (type.categoryID() != EVEDB::invCategories::Orbitals)
        and (type.categoryID() != EVEDB::invCategories::SovereigntyStructure)
        and (type.categoryID() != EVEDB::invCategories::StructureUpgrade)
        and (type.groupID() != EVEDB::invGroups::Construction_Platform)) {
            _log(ITEM__ERROR, "Trying to load %s as Structure.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }

        return StructureItemRef( new StructureItem(structureID, type, data ) );
    }

private:
    SystemEntity* mySE;
};


/**
 * ObjectSystemEntity which represents structure object in space
 */
class Client;
class Missile;
class MoonSE;
class TowerSE;
class TCUSE;
class SBUSE;
class IHubSE;
class JumpBridgeSE;
class ArraySE;
class BatterySE;
class WeaponSE;
class ContainerSE;
class StructureSE
: public DynamicSystemEntity
{
public:
    StructureSE(StructureItemRef structure, EVEServiceManager& services, SystemManager* system, const FactionData& data);
    virtual ~StructureSE()  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual StructureSE*        GetPOSSE()              { return this; }
    virtual TowerSE*            GetTowerSE()            { return nullptr; }
    virtual ArraySE*            GetArraySE()            { return nullptr; }
    virtual BatterySE*          GetBatterySE()          { return nullptr; }
    virtual WeaponSE*           GetWeaponSE()           { return nullptr; }
    virtual ReactorSE*          GetReactorSE()          { return nullptr; }
    virtual TCUSE*              GetTCUSE()              { return nullptr; }
    virtual SBUSE*              GetSBUSE()              { return nullptr; }
    virtual IHubSE*             GetIHubSE()             { return nullptr; }
    virtual JumpBridgeSE*       GetJumpBridgeSE()       { return nullptr; }

    /* class type tests. */
    virtual bool                IsPOSSE()               { return true; }
    virtual bool                IsTCUSE()               { return false; }
    virtual bool                IsSBUSE()               { return false; }
    virtual bool                IsIHubSE()              { return false; }
    virtual bool                IsJammerSE()            { return m_jammer; }
    virtual bool                IsCynoGeneratorSE()     { return m_generator; }
    virtual bool                IsMoonMiner()           { return m_miner; }
    virtual bool                IsPlatformSE()          { return false; }
    virtual bool                IsJumpBridgeSE()        { return false; }
    virtual bool                IsTowerSE()             { return false; }
    virtual bool                IsArraySE()             { return false; }
    virtual bool                IsBatterySE()           { return false; }
    virtual bool                IsWeaponSE()            { return false; }
    virtual bool                IsReactorSE()           { return false; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &damage);
    virtual void                Init();
    virtual void                InitData();
    virtual void                Scoop();

    /* virtual functions to be overridden in derived classes */
    virtual void     MissileLaunched(Missile* pMissile) { /* Do nothing here */ }

    /* basic structure processing methods */
    virtual void                SetOnline();
    virtual void                SetOffline();
    virtual void                SetInvulnerable();
    virtual void                SetVulnerable();
    virtual void                Online();
    virtual void                SetOperating();
    virtual void                Operating();

    /* basic tower processing methods */
    virtual void                Reinforced()            { /* do nothing here yet */ }

    /* specific functions handled in this class. */
    // this is for dropping POS where Init() and Add() each need info from the other.
    void                        Drop(SystemBubble* pBubble);
    void                        Anchor();
    void                        Offline();
    void                        PullAnchor();
    void                        SetAnchor(Client* pClient, GPoint& pos);
    void                        Activate(int32 effectID);
    void                        Deactivate(int32 effectID);
    void                        GetEffectState(PyList& into);
    int8                        GetState() const        { return m_data.state; }
    int8                        GetProcState()          { return m_procState; }
    int8                        GetStatus()             { return m_data.status; }
    MoonSE*                     GetMoonSE()             { return m_moonSE; }
    PlanetSE*                   GetPlanetSE()           { return m_planetSE; } //Planets are required for sovereignty structures
    StargateSE*                 GetGateSE()             { return m_gateSE; }

    inline void                 SetPOSState(int8 state) { m_data.state = state; }
    inline void                 SetTimer(uint32 time)   { m_procTimer.SetTimer(time); }

    void                        SetUsageFlags(int8 view=0, int8 take=0, int8 use=0);
    inline int8                 CanUse()                { return m_data.use; }
    inline int8                 CanView()               { return m_data.view; }
    inline int8                 CanTake()               { return m_data.take; }

    // for orbital infrastructure
    void                     SetPlanet(uint32 planetID) { m_anchorPointID = planetID; }
    uint32                      GetPlanetID()           { return m_anchorPointID; }

    // structure update methods....may not use like this
    void                        UpdateTimeStamp()       { m_db.UpdateTimeStamp(m_data.itemID, m_data); }
    void                        UpdateUsageFlags()      { m_db.UpdateUsageFlags(m_data.itemID, m_data); }

    // for targetMgr
    bool                        IsReinforced()          { return (m_data.state == EVEPOS::StructureState::Reinforced); }

    void                        SendSlimUpdate();

protected:
    void                        SendEffectUpdate(int16 effectID, bool active);

    PosMgrDB                    m_db;

    MoonSE*                     m_moonSE;               /* moonSE this structure is orbiting. */
    PlanetSE*                   m_planetSE;             /* planetSE this structure is orbiting. (for sovereignty structures) */
    TowerSE*                    m_towerSE;              /* controlling towerSE for this structure */
    TCUSE*                      m_tcuSE;                /* controlling TCUs */
    SBUSE*                      m_sbuSE;
    IHubSE*                     m_ihubSE;
    JumpBridgeSE*               m_bridgeSE;
    StargateSE*                 m_gateSE;

    EVEPOS::StructureData       m_data;

    int8                        m_procState;            /* internal state data for processing mode changes */

    // this is time shown in structure status (time left until current state completes)
    uint32                      m_delayTime;

    // for orbital infrastructure (customs office and moon miner)
    GVector                     m_rotation;             /* direction to planet (for correct orientation) */
    uint32                      m_anchorPointID;

private:
    uint32 m_duration;              // module duration in ms

    Timer m_procTimer;              // module state timer

    bool m_tcu :1;              // Territorial Claim Unit
    bool m_sbu :1;              // System Blockade Unit
    bool m_ihub :1;             // Infrastructure Hub
    bool m_tower :1;            // Control Tower
    bool m_miner :1;            // Moon Miner
    bool m_bridge :1;           // Jump Bridge
    bool m_jammer :1;           // Cyno Jammer
    bool m_generator :1;        // Cyno Generator
    bool m_loaded :1;
    bool m_module :1;           // any structure requiring a control tower
    bool m_reactor :1;
    bool m_platform :1;         // Outpost construction platform
    bool m_cargo :1;            // cargo container
};

#endif  // EVEMU_POS_STRUCTURE_H_

/* more in system/destinymgr.h
 *
 *    def OnSlimItemUpdated(self, slimItem):
 *        orbitalState = getattr(slimItem, 'orbitalState', None)
 *        orbitalTimestamp = getattr(slimItem, 'orbitalTimestamp', blue.os.GetSimTime())
 *        fxSequencer = sm.GetService('FxSequencer')
 *        if not hasattr(self, 'orbitalState'):
 *            if orbitalState in (entities.STATE_ANCHORING, entities.STATE_ANCHORED):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorDrop', 0, 1, 0)
 *            elif orbitalState in (entities.STATE_IDLE, entities.STATE_OPERATING):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnlined', 0, 1, 0)
 *        else:
 *            if orbitalState == entities.STATE_ANCHORING and self.orbitalState == entities.STATE_UNANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorDrop', 0, 1, 0)
 *            if orbitalState == entities.STATE_ONLINING and self.orbitalState == entities.STATE_ANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnline', 0, 1, 0)
 *            if orbitalState == entities.STATE_IDLE and self.orbitalState == entities.STATE_ONLINING:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnlined', 0, 1, 0)
 *            if orbitalState == entities.STATE_ANCHORED and self.orbitalState in (entities.STATE_OFFLINING, entities.STATE_IDLE, entities.STATE_OPERATING):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOffline', 0, 1, 0)
 *            if orbitalState == entities.STATE_UNANCHORING and self.orbitalState == entities.STATE_ANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorLift', 0, 1, 0)
 *        self.orbitalState = orbitalState
 *        self.orbitalTimestamp = orbitalTimestamp
 */

    /*
    self.variance = sm.GetService('clientDogmaStaticSvc').GetTypeAttribute(self.typeID, const.attributeReinforcementVariance)
    self.reinforceHours = [ ('%.2d:00 - %.2d:00' % ((x - self.variance / 3600) % 24, (x + self.variance / 3600) % 24), x) for x in xrange(0, 24) ]
    reinforceValue = 0-23

    with a variance of 3600, reinforceValue of 0 sets hours of 23:00 to 01:00
    */

    /*{'FullPath': u'UI/Messages', 'messageID': 256800, 'label': u'ClaimMarkerNoAllianceOnlinedBody'}(u'The {structure} can not be put online because the owner is not part of an alliance.', None, {u'{structure}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'structure'}})
     {'FullPath': u'UI/Messages', 'messageID': 256801, 'label': u'ClaimMarkerNoAllianceAnchoredBody'}(u'The {structure} can not be anchored because the owner is not part of an alliance.', None, {u'{structure}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'structure'}})
     {'FullPath': u'UI/Messages', 'messageID': 256802, 'label': u'EffectAlreadyActive2Body'}(u'{modulename} is already active', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}})
     {'FullPath': u'UI/Messages', 'messageID': 256803, 'label': u'CantAnchorInfrasturctureHubWrongAllianceBody'}(u'Sovereignty has not been claimed in this system by your alliance!', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256804, 'label': u'CantAnchorNoInfrastructureHubBody'}(u'This structure can only be anchored in a system that has a infrastructure hub!', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256805, 'label': u'NotAllProbesReturnedSuccessfullyBody'}(u'Not all probes were called back. You possibly do not have room for all of them in your cargo hold.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256806, 'label': u'ConfirmBreakCourierPackageTitle'}(u'Fail contract?', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256807, 'label': u'ConfirmBreakCourierPackageBody'}(u'Breaking this courier package will make it impossible to complete the courier contract. Are you sure you want to do this?', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256808, 'label': u'CantAnchorInfrastructureHubNoPlanetBody'}(u'You can not anchor Infrastructure Hub here. It needs to be close to a planet that has no station near it.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256809, 'label': u'CantAnchorInfrastructureHubNearAStationBody'}(u'You can not anchor Infrastructure Hub near a station.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256810, 'label': u'CantOnlineInfrastructureHubAlreadyInSystemBody'}(u'This Infrastructure Hub cannot be onlined since there is another one operating in the system already.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256811, 'label': u'ChannelTryingToCreateTitle'}(u'Create Channel', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256812, 'label': u'ChannelTryingToCreateBody'}(u'Attempting to create a channel', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256813, 'label': u'ChannelTryingToJoinTitle'}(u'Join Channel', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256814, 'label': u'ChannelTryingToJoinBody'}(u'Attempting to join a channel', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256815, 'label': u'CantOnlineNoInfrastructureHubBody'}(u'This structure can only be onlined in a system that has a infrastructure hub!', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256816, 'label': u'CantAnchorInfrastructureHubAnotherHubPresentBody'}(u'The Infrastructure Hub fails to anchor as there is another hub active in the system.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256817, 'label': u'CanOnlyInstallStructureUpgradeToIHBody'}(u'You can only install structure upgrades to the infrastructure hub.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256837, 'label': u'CantAnchorInfrastructureHubNearStationBody'}(u'You cannot anchor the Infrastructure Hub near a station. Try another planet which does not have a station in orbit around it.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256875, 'label': u'CantAnchorToFarFromSunBody'}(u'The Territorial Claim Unit cannot be anchored this far from the sun, move it closer to less than 300 AU.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256876, 'label': u'CantOnlineAnotherClaimMarkerOnliningBody'}(u'Your corporation can only online one Territorial Claim Unit per system.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256877, 'label': u'CantOnlineInfrastructureDontOwnFlagBody'}(u"You can not online a infrastructure hub because your corporation doesn't own the Territorial Claim Unit.", None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256878, 'label': u'CantOnlineSovereigntyAlreadyClaimedBody'}(u'The Territorial Claim Unit cannot online because sovereignty has already been claimed in this system.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256879, 'label': u'CantAnchorDisruptorIHProtectingBody'}(u'There is a infrastructure hub that is preventing you from anchoring the Sovereignty Blockade Unit.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256880, 'label': u'CantAnchorDisruptorOutpostProtectingBody'}(u'The sovereignty holder holds the outpost, you can not anchor Sovereignty Blockade Unit while this is true.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256881, 'label': u'CantAnchoredDisruptorNotClaimedBody'}(u'The Sovereignty Blockade Unit cannot be anchored because sovereignty has not been claimed in this system.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256882, 'label': u'CantOnlineDisruptorOutpostProtectingBody'}(u'The sovereignty holder holds an outpost in the solar system. You cannot online the Sovereignty Blockade Unit while this is true.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 256892, 'label': u'CantAnchorDefenseBunkerNotHereBody'}(u'Defense bunkers must be anchored withing 50 - 100 km of a stargate, station, control tower, or infrastructure hub.', None, None)
     {'FullPath': u'UI/Messages', 'messageID': 257374, 'label': u'StructureNotControllableUntilOnlineBody'}(u'You cannot assume control of the {item} until it has been put online.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}})

     {'FullPath': u'UI/Messages', 'messageID': 260202, 'label': u'CannotPlaceInLowShieldTowerBody'}(u'You cannot put {[item]item.name} into the {[item]type.name} while it is reinforced or has a shield level lower than {[numeric]level}%.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}, u'{[numeric]level}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'level'}})
     {'FullPath': u'UI/Messages', 'messageID': 260203, 'label': u'CannotPlaceInOfflineStructureBody'}(u"You cannot put {[item]item.name} into the {[item]type.name} while it is offline and it's control tower is reinforced or could become reinforced due to the current shield level.", None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})
     {'FullPath': u'UI/Messages', 'messageID': 260204, 'label': u'CannotPlaceInOperatingStructureBody'}(u'You cannot put {[item]item.name} in the {[item]type.name} while it is active.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})
     {'FullPath': u'UI/Messages', 'messageID': 260205, 'label': u'CannotPlaceInUnanchoredStructureBody'}(u'You cannot put {[item]item.name} into the {[item]type.name} while it is unanchored.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})
     {'FullPath': u'UI/Messages', 'messageID': 260206, 'label': u'CantPerformTowerShieldFluctuationsBody'}(u'You cannot do that at this time as the {[item]tower.name} is either reinforced or the shield level is below 50%.', None, {u'{[item]tower.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'tower'}})
     {'FullPath': u'UI/Messages', 'messageID': 260207, 'label': u'CantOnlineTowerLacksResourcesBody'}(u'The {[item]tower.name} cannot be brought online because it requires the following resource(s) to power it: {resource}', None, {u'{[item]tower.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'tower'}, u'{resource}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'resource'}})
     {'FullPath': u'UI/Messages', 'messageID': 260208, 'label': u'CannotPlacePilotedShipInStructureBody'}(u'This {[item]container.name} cannot accept the {[item]type.name} as it still has a pilot inside it.', None, {u'{[item]container.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'container'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})
     {'FullPath': u'UI/Messages', 'messageID': 260209, 'label': u'CannotPlaceTypeInStructureBody'}(u'This {[item]container.name} can only contain one or more specific types of which the {[item]type.name} you gave was not one.', None, {u'{[item]container.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'container'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})


     */

    /*{'messageKey': 'CannotPlaceInLowShieldTower', 'dataID': 17885348, 'suppressable': False, 'bodyID': 260202, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 306}
     * {'messageKey': 'CannotPlaceInOfflineStructure', 'dataID': 17885351, 'suppressable': False, 'bodyID': 260203, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 307}
     * {'messageKey': 'CannotPlaceInOperatingStructure', 'dataID': 17885354, 'suppressable': False, 'bodyID': 260204, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 308}
     * {'messageKey': 'CannotPlaceInUnanchoredStructure', 'dataID': 17885357, 'suppressable': False, 'bodyID': 260205, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 310}
     * {'messageKey': 'CannotPlaceItemInsideItself', 'dataID': 17877493, 'suppressable': False, 'bodyID': 257272, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257271, 'messageID': 2947}
     * {'messageKey': 'CannotPlacePilotedShipInStructure', 'dataID': 17885366, 'suppressable': False, 'bodyID': 260208, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 311}
     * {'messageKey': 'CannotRemoveFromLowShieldTower', 'dataID': 17885380, 'suppressable': False, 'bodyID': 260213, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 315}
     * {'messageKey': 'CannotRemoveFromOfflineStructure', 'dataID': 17885383, 'suppressable': False, 'bodyID': 260214, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 316}
     * {'messageKey': 'CannotRemoveFromThatLocation', 'dataID': 17885013, 'suppressable': False, 'bodyID': 260085, 'messageType': 'notify', 'urlAudio': 'wise:/msg_CannotRemoveFromThatLocation_play', 'urlIcon': '', 'titleID': None, 'messageID': 318}
     * {'messageKey': 'CannotRemoveFromUnanchoredStructure', 'dataID': 17885386, 'suppressable': False, 'bodyID': 260215, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 319}
     */

    //CantStructureIncapacitated
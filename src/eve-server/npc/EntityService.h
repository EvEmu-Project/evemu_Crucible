
 /**
  * @name EntityService.h
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */


#ifndef __EVEMU_NPC_ENTITY_H
#define __EVEMU_NPC_ENTITY_H

#include "../eve-server.h"

#include "services/BoundService.h"
#include "PyServiceCD.h"

class SystemManager;

class EntityService : public BindableService <EntityService>
{
public:
    EntityService(EVEServiceManager& mgr);

protected:
    //overloaded in order to support bound objects:
    BoundDispatcher* BindObject(Client* client, PyRep* bindParameters);
};

class EntityBound : public EVEBoundObject <EntityBound>
{
public:
    EntityBound(EVEServiceManager& mgr, SystemManager* systemMgr, uint32 systemID, PyRep* bindData);

protected:
    bool CanClientCall(Client* client) override;

    PyResult CmdEngage(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdRelinquishControl(PyCallArgs& call, PyList* IDs);
    PyResult CmdDelegateControl(PyCallArgs& call, PyList* droneIDs, PyInt* controllerID);
    PyResult CmdAssist(PyCallArgs& call, PyInt* assistID, PyList* droneIDs);
    PyResult CmdGuard(PyCallArgs& call, PyInt* guardID, PyList* droneIDs);
    PyResult CmdMine(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdMineRepeatedly(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdUnanchor(PyCallArgs& call, PyList* droneIDs, PyInt* targetID);
    PyResult CmdReturnHome(PyCallArgs& call, PyList* droneIDs);
    PyResult CmdReturnBay(PyCallArgs& call, PyList* droneIDs);
    PyResult CmdAbandonDrone(PyCallArgs& call, PyList* droneIDs);
    PyResult CmdReconnectToDrones(PyCallArgs& call, PyList* droneCandidates);

    PyCallable_DECL_CALL(CmdEngage);
    PyCallable_DECL_CALL(CmdRelinquishControl);
    PyCallable_DECL_CALL(CmdDelegateControl);
    PyCallable_DECL_CALL(CmdAssist);
    PyCallable_DECL_CALL(CmdGuard);
    PyCallable_DECL_CALL(CmdMine);
    PyCallable_DECL_CALL(CmdMineRepeatedly);
    PyCallable_DECL_CALL(CmdUnanchor);
    PyCallable_DECL_CALL(CmdReturnHome);
    PyCallable_DECL_CALL(CmdReturnBay);
    PyCallable_DECL_CALL(CmdAbandonDrone);
    PyCallable_DECL_CALL(CmdReconnectToDrones);

protected:
    SystemManager* m_sysMgr;

    uint32 m_systemID;
};

#endif  // __EVEMU_NPC_ENTITY_H

/*
 * {'messageKey': 'MiningDronesDeactivatedAsteroidEmpty', 'dataID': 17883322, 'suppressable': False, 'bodyID': 259462, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedAsteroidEmpty_play', 'urlIcon': '', 'titleID': None, 'messageID': 1168}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldFull', 'dataID': 17883265, 'suppressable': False, 'bodyID': 259442, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedCargoHoldFull_play', 'urlIcon': '', 'titleID': None, 'messageID': 1169}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldNowFull', 'dataID': 17883243, 'suppressable': False, 'bodyID': 259434, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1170}
 * {'messageKey': 'MiningDronesDeactivatedOutOfRange', 'dataID': 17883208, 'suppressable': False, 'bodyID': 259422, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1171}
 */

/* {'FullPath': u'UI/Messages', 'messageID': 258206, 'label': u'EntityDelegateInsideFieldBody'}(u'The drone cannot be delegated while you are inside an active force field.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257373, 'label': u'DeniedDroneTargetForceFieldBody'}(u'Your drones cannot engage {[item]target.name} since it is within the range of a forcefield you are barred from entering.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259588, 'label': u'DroneCommandRequiresShipPilotedFleetMemberBody'}(u'To give this command to a drone requires that the active target be piloted and piloted by a fleet member.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259592, 'label': u'DroneCommandEngageRequiresNoAmbiguityBody'}(u'You are commanding a group of drones to engage a target, but some of the drones offer assistance and some commit aggression. This is not possible as the conflicting signals cause interference.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259593, 'label': u'DroneCommandRequiresActiveTargetBody'}(u'To give this command to a drone requires that you have an active target. Target something and try again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259601, 'label': u'EntityTargetCharNotPresentBody'}(u'The drone cannot be commanded with respect to {[character]targetChar.name} because they are not present in this solar system.', None, {u'{[character]targetChar.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259606, 'label': u'EntityHasSkillPrerequisitesBody'}(u'You do not have the required {[numeric]skillCount -> "skill", "skills"} to do that. To command that drone requires having learned the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259607, 'label': u'EntityTargetMustBeFleetMemberBody'}(u'Drones can only accept that command if {targetOwner} is a member of your fleet, which they are not.', None, {u'{targetOwner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetOwner'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259637, 'label': u'EntityCurrentlyMiningBody'}(u'{targetTypeName} is currently mining an asteroid and cannot take commands until it has finished. It will return home and await further orders when it is finished.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259638, 'label': u'EntityToolRequirementsBody'}(u'You cannot do that. The drone you use on the {tGroup} needs to have the following in its skill requirements: {skills}.', None, {u'{tGroup}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'tGroup'}, u'{skills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'skills'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259636, 'label': u'EntityActionSecurityLevelRestrictionsBody'}(u'You cannot do that. That drone command requires you to be in a solar system with a security level less than {[numeric]needed, decimalPlaces=1}.', None, {u'{[numeric]needed, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'needed'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259502, 'label': u'DroneCommandRequiresShipButNotCapsuleBody'}(u'To give this command to a drone requires that the active target be a ship, but not a capsule.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259503, 'label': u'DroneTargetJammedBody'}(u'The drone is target jammed and cannot be commanded to do that.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259504, 'label': u'DronesNotLoadableInSpaceBody'}(u'You cannot load to or unload from the drone bay while in space.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 257802, 'label': u'DronesDroppedBecauseOfBandwidthModificationBody'}(u'The drone control bandwidth of your ship has been modified causing you to lose the ability to control some drones.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259504, 'label': u'DronesNotLoadableInSpaceBody'}(u'You cannot load to or unload from the drone bay while in space.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259694, 'label': u'EntityIncapacitatedCommandBody'}(u'{targetTypeName} is incapacitated due to damage or abandonment and will not respond to the command you are giving it (try scooping it).', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259695, 'label': u'EntityNotYoursToCommandBody'}(u'{targetTypeName} does not respond to your commands.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259698, 'label': u'EntityTargetMustBeTargetedBody'}(u'{targetTypeName} requires the target be locked onto by you, which it is not.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259699, 'label': u'EntityTargetAlreadyHasControlBody'}(u'Control of the {item} cannot be delegated to {whom} because they already have control of it.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258349, 'label': u'EntityTargetCharInCapsuleBody'}(u'The drone cannot be commanded with respect to {targetChar} because the pilot is in a capsule.', None, {u'{targetChar}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259601, 'label': u'EntityTargetCharNotPresentBody'}(u'The drone cannot be commanded with respect to {[character]targetChar.name} because they are not present in this solar system.', None, {u'{[character]targetChar.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259698, 'label': u'EntityTargetMustBeTargetedBody'}(u'{targetTypeName} requires the target be locked onto by you, which it is not.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259699, 'label': u'EntityTargetAlreadyHasControlBody'}(u'Control of the {item} cannot be delegated to {whom} because they already have control of it.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259668, 'label': u'EntityTargetTooDistantBody'}(u'The drones fail to execute your commands as the target {targetTypeName} is not within your {distance} m drone command range.', None, {u'{distance}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258393, 'label': u'EntityTargetWarpDisruptedBody'}(u'Control of the {[item]item.name} cannot be delegated to someone who the drones cannot warp to.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259705, 'label': u'EntityUnknownCommandBody'}(u'{targetTypeName} does not recognize the command you are trying to give it.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259706, 'label': u'EntityNotPresentBody'}(u'{targetTypeName} cannot be commanded as it is not actually present.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 *
 */

/*{'FullPath': u'UI/Messages', 'messageID': 259652, 'label': u'EntityBrokenCommandBody'}(u'{targetTypeName} seems to be defective and does not respond to the command you are giving it.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259653, 'label': u'EntityDistantCommandBody'}(u'{targetTypeName} is too far away and will not respond to the command you are giving it (must be within {distance} meters from it).', None, {u'{distance}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259668, 'label': u'EntityTargetTooDistantBody'}(u'The drones fail to execute your commands as the target {targetTypeName} is not within your {distance} m drone command range.', None, {u'{distance}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259694, 'label': u'EntityIncapacitatedCommandBody'}(u'{targetTypeName} is incapacitated due to damage or abandonment and will not respond to the command you are giving it (try scooping it).', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259695, 'label': u'EntityNotYoursToCommandBody'}(u'{targetTypeName} does not respond to your commands.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259698, 'label': u'EntityTargetMustBeTargetedBody'}(u'{targetTypeName} requires the target be locked onto by you, which it is not.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259699, 'label': u'EntityTargetAlreadyHasControlBody'}(u'Control of the {item} cannot be delegated to {whom} because they already have control of it.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259702, 'label': u'EntityNoTargetDroneManagementAbilitiesBody'}(u'Control of the {item} cannot be delegated to {whom} because they do not have the skill to control any drones.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259703, 'label': u'EntityNoTargetDroneManagementAbilitiesLeftBody'}(u'Control of the {item} cannot be delegated to {whom} because they only have the skill to control {[numeric]limit} drones and they are already controlling that many.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]limit}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259704, 'label': u'EntityTargetNotPresentBody'}(u'{targetTypeName} cannot be commanded to work on a target that is no longer present.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259705, 'label': u'EntityUnknownCommandBody'}(u'{targetTypeName} does not recognize the command you are trying to give it.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259706, 'label': u'EntityNotPresentBody'}(u'{targetTypeName} cannot be commanded as it is not actually present.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259711, 'label': u'EntityInvalidTargetBody'}(u'{targetTypeName} can only perform that action on an item of group  {desiredTarget}.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}, u'{desiredTarget}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredTarget'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258393, 'label': u'EntityTargetWarpDisruptedBody'}(u'Control of the {[item]item.name} cannot be delegated to someone who the drones cannot warp to.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258349, 'label': u'EntityTargetCharInCapsuleBody'}(u'The drone cannot be commanded with respect to {targetChar} because the pilot is in a capsule.', None, {u'{targetChar}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259601, 'label': u'EntityTargetCharNotPresentBody'}(u'The drone cannot be commanded with respect to {[character]targetChar.name} because they are not present in this solar system.', None, {u'{[character]targetChar.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259606, 'label': u'EntityHasSkillPrerequisitesBody'}(u'You do not have the required {[numeric]skillCount -> "skill", "skills"} to do that. To command that drone requires having learned the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259607, 'label': u'EntityTargetMustBeFleetMemberBody'}(u'Drones can only accept that command if {targetOwner} is a member of your fleet, which they are not.', None, {u'{targetOwner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetOwner'}})
 */

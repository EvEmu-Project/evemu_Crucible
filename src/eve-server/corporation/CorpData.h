
 /**
  * @name CorpData.h
  *     Corporation Data containers for EVEmu
  *
  * @Author:        Allan
  * @date:          25 December 2017
  */

#ifndef EVEMU_SRC_CORP_DATA_H_
#define EVEMU_SRC_CORP_DATA_H_


#include "../eve-server.h"

    /*
    "corpRole"      - corporation management-type roles (manager, officer, trader)  NOT access-type roles
    "rolesAtAll"    - access roles everywhere.  is joined with other access roles
    "rolesAtBase"   - access roles at base defined for this char. overrides hq if hq and base are same location for char
    "rolesAtHQ"     - access roles at corp HQ.
    "rolesAtOther"  - access roles for non-station containers with corp hangars
    */


namespace Corp {

    /* POD structure for corp members query data */
    struct QueryMembers {
        bool blockRoles;
        uint32 characterID;
        int64 startDateTime;
        int64 titleMask;
        int64 rolesAtAll;
        int64 rolesAtHQ;
        int64 rolesAtBase;
        int64 rolesAtOther;
        int64 grantableRoles;
        int64 grantableRolesAtHQ;
        int64 grantableRolesAtBase;
        int64 grantableRolesAtOther;
    };

    /* POD structure for corp vote data */
    struct VoteCaseOptions {
        int8 optionID;
        int16 parameter1;      // typeID
        int16 voteCaseID;
        int32 parameter;       // itemID
        int32 votesFor;
        int32 votesMade;
        int32 votesProxied;
        int32 parameter2;      // locationID
        std::string optionText;
    };

    /* POD structure for corp app data */
    struct ApplicationInfo {
        bool valid;
        uint32 appID;
        uint32 corpID;
        uint32 charID;
        uint32 status;
        uint32 deleted;
        uint32 lastCID;
        int64 appTime;
        int64 role;
        int64 grantRole;
        std::string appText;
    };

    struct MedalData {
        int8 part;
        int16 color;
        std::string graphic;
    };

}

#endif  // EVEMU_SRC_CORP_DATA_H_


/*{'FullPath': u'UI/Messages', 'messageID': 259245, 'label': u'PermissionDeniedCorpDoesntAllowBody'}(u'You cannot do that on behalf of the {corp} corporation because they do not allow it.', None, {u'{corp}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corp'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259246, 'label': u'PermissionDeniedNeedEquipRoleBody'}(u'You cannot do that on behalf of the {corp} corporation because they have not given you the equipment config role.', None, {u'{corp}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corp'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259247, 'label': u'PermissionDeniedNeedStarbaseRoleBody'}(u'You cannot do that on behalf of the {corp} corporation because they have not given you the starbase config role.', None, {u'{corp}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corp'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259248, 'label': u'PermissionDeniedNotInCorpBody'}(u'You cannot do that on behalf of the {corp} corporation because you are not a member.', None, {u'{corp}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'corp'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259249, 'label': u'PermissionDeniedNotOwnerBody'}(u'You are unable to do that to the {item} as it is owned by {owner} and not by you.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}})
 */
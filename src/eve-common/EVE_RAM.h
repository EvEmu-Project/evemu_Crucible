
/*
 * EVE_RAM.h
 *  enums for R.A.M.
 *
 */


#ifndef EVE_RAM_ENUMS_H
#define EVE_RAM_ENUMS_H

/*
BASE_INVENTION_ME = -4.0
BASE_INVENTION_PE = -4.0
*/

/*  for indy */
namespace EvERam {
    namespace Activity {
        //from table 'ramActivities'
        enum  {
            None = 0,
            Manufacturing = 1,
            ResearchTech = 2,
            ResearchTime = 3,
            ResearchMaterial = 4,
            Copying = 5,
            Duplicating = 6,
            ReverseEngineering = 7,
            Invention = 8
        };
    }

    namespace Status {
        //from table 'ramCompletedStatuses'
        enum  {
            InProgress = 0,
            Delivered = 1,
            Abort = 2,
            GMAbort = 3,
            Unanchor = 4,
            Destruction = 5
        };
    }

    namespace RestrictionMask {
        //restrictionMask from table 'ramAssemblyLines'
        enum  {
            None = 0,
            BySecurity = 1,
            ByStanding = 2,
            ByCorp = 4,
            ByAlliance = 8
        };
    }

    struct LineRestrictions {
        uint8 activityID;
        uint8 rMask;
        uint32 ownerID;
        float minCharSec;
        float maxCharSec;
        float minCorpSec;
        float maxCorpSec;
        float minStanding;
    };

    /* POD structure for blueprint ram requirements */
    struct RamRequirements {
        bool extra;
        uint8 activityID;
        uint16 requiredTypeID;
        uint32 quantity;
        float damagePerJob;
    };

    /* POD structure for blueprint item materials  */
    struct RamMaterials {
        uint16 materialTypeID;
        uint32 quantity;
    };

    /* POD structure for blueprint required materials  */
    struct RequiredItem {
        bool extra :1;
        bool isSkill :1;
        uint16 typeID;
        uint32 quantity;
        float damagePerJob;
    };

    /* POD structure for indy job data  */
    struct JobProperties {
        int8 activity;
        int8 status;
        int16 jobRuns;
        int16 licensedRuns;
        uint32 itemID;
        uint32 ownerID;
        uint32 eventID;
        int64 endTime;
        EVEItemFlags outputFlag;
    };

    /* POD structure for blueprint data */
    struct bpData {
        bool copy :1;
        int8 mLevel;       // material level
        int8 pLevel;       // productivity level
        int16 runs;
    };

    /* POD structure for blueprint type data */
    struct bpTypeData {
        int8 catID;
        int8 techLevel;
        uint16 wasteFactor;
        uint16 productTypeID;
        uint16 parentBlueprintTypeID;
        uint32 productionTime;
        uint32 researchProductivityTime;
        uint32 researchMaterialTime;
        uint32 researchCopyTime;
        uint32 researchTechTime;
        uint32 productivityModifier;
        uint32 materialModifier;
        uint32 maxProductionLimit;
        float chanceOfRE;
    };

}

#endif  //EVE_RAM_ENUMS_H

/*{'FullPath': u'UI/Messages', 'messageID': 258445, 'label': u'RamInstalledItemMustBeInShipTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258446, 'label': u'RamInstalledItemMustBeInShipBody'}(u'The item you are attempting to install must be located in your ship. It may not be in a container in your ship.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258447, 'label': u'RamInstalledItemInStructureNotInContainerTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258448, 'label': u'RamInstalledItemInStructureNotInContainerBody'}(u'The item you are attempting to install must be either in a station or on the floor of the structure. It may not be in a container.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258449, 'label': u'RamInstalledItemInStructureUnknownLocationTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258450, 'label': u'RamInstalledItemInStructureUnknownLocationBody'}(u'The item you are attempting to install must be either in a station or on the floor of the structure. It may not be in a container.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258451, 'label': u'RamRemoteInstalledItemNotInStationTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258452, 'label': u'RamRemoteInstalledItemNotInStationBody'}(u'The remote item you are trying to install must be on a hangar floor or in your corporate hangar. It may not be in a container at either of these locations.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258453, 'label': u'RamRemoteInstalledItemImpoundedTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258454, 'label': u'RamRemoteInstalledItemImpoundedBody'}(u'The remote item you are trying to install is an impounded item. You must unimpound the item before attempting to use it.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258455, 'label': u'RamRemoteInstalledItemNotInOfficeTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258456, 'label': u'RamRemoteInstalledItemNotInOfficeBody'}(u'The remote item you are trying to install must be located on your corporate hangar floor. It may not be located in some container on that floor.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258457, 'label': u'RamRemoteInstalledItemInStationNotHangarTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258458, 'label': u'RamRemoteInstalledItemInStationNotHangarBody'}(u'The remote item you are trying to install must be located on your hangar floor. It may not be located in some container on that floor.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258459, 'label': u'RamInstalledItemBadLocationTitle'}(u'Item Warning', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258460, 'label': u'RamInstalledItemBadLocationBody'}(u'The item you are trying to install is not in an acceptable location. For example, the item may not be in a container.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258461, 'label': u'RamPleasePickAnInstalltionTitle'}(u'Select Installation?', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258462, 'label': u'RamPleasePickAnInstalltionBody'}(u'You have not selected an installation. Please select an installation and then try again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258463, 'label': u'RamPleasePickAnItemToInstallTitle'}(u'Nothing to Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258464, 'label': u'RamPleasePickAnItemToInstallBody'}(u'You have not selected an item to install. Please select an item, such as a blueprint to install and try again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258465, 'label': u'RamAssemblyLineHasNoActivityTitle'}(u'Bad Assembly Line', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258466, 'label': u'RamAssemblyLineHasNoActivityBody'}(u'The selected assembly line does not support any activities. Please select a different one and try again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258467, 'label': u'RamCorpInstalledItemNotInCargoTitle'}(u'Item Warning', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258468, 'label': u'RamCorpInstalledItemNotInCargoBody'}(u'The item you are trying to install appears to be in your cargo. Please place the item on your hangar floor or corporate hangar floor and try again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258469, 'label': u'RamNotYourItemToInstallTitle'}(u'Item Warning', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258470, 'label': u'RamNotYourItemToInstallBody'}(u'It would appear that the item you are trying to install does not belong to you or your corporation. Please pick a different item to install and try again.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258472, 'label': u'RamStructureNotIsSolarsystemTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258473, 'label': u'RamStructureNotIsSolarsystemBody'}(u'The structure you are trying to use in is not in a solar system. Thus it cannot be online.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258474, 'label': u'RamInstalledItemMustBeInInstallationTitle'}(u'Failed To Install', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258475, 'label': u'RamInstalledItemMustBeInInstallationBody'}(u'The item you are trying to install must be in the same location as the installation you are trying to use.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258476, 'label': u'RamCompletionMustBeInShipTitle'}(u'Failed to Deliver', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258477, 'label': u'RamCompletionMustBeInShipBody'}(u'You cannot deliver this job unless you are the pilot of the ship that the job is installed in.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258481, 'label': u'RamCannotInstallForCorpByRoleFactoryManagerTitle'}(u'Access Denied', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258482, 'label': u'RamCannotInstallForCorpByRoleFactoryManagerBody'}(u'You cannot install this job for your corporation because you do not possess the role "Factory Manager"', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258483, 'label': u'RamNoShipTitle'}(u"You don't have a ship", None, None)
 */
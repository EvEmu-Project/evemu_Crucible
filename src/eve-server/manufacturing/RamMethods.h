
 /**
  * @name RamMethods.h
  *   methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18
  */


#ifndef EVE_MANUF_RAM_METHODS_H
#define EVE_MANUF_RAM_METHODS_H


#include "manufacturing/FactoryDB.h"

class Client;

class RamMethods
: public Singleton< RamMethods >
{
public:
    // verify output, runs, and type data
    void ActivityCheck(Client* const pClient, const Call_InstallJob& args, BlueprintRef bpRef);
    // verify job count for char
    void JobsCheck(Character* pChar, const Call_InstallJob& args);
    // verify range for stations and structure data for POS
    void InstallationCheck(Client* const pClient, int32 lineLocationID);
    // verify roles and restrictions
    void LinePermissionCheck(Client* const pClient, const Call_InstallJob& args);
    void ItemOwnerCheck(Client*const pClient, const Call_InstallJob& args, BlueprintRef bpRef);
    void ItemLocationCheck(Client* const pClient, const Call_InstallJob& args, InventoryItemRef installedItem);

    void HangarRolesCheck(Client* const pClient, int16 flagID);
    // only used for station location role checks
    void LocationRolesCheck(Client*const pClient, const CorpPathElement& data);

    void ProductionTimeCheck(uint32 productionTime);
    void MaterialSkillsCheck(Client* const pClient, uint32 runs, const PathElement& bomLocation, const Rsp_InstallJob& rsp, const std::vector< EvERam::RequiredItem >& reqItems);

    void VerifyCompleteJob(const Call_CompleteJob& args, EvERam::JobProperties& data, Client*const pClient);

    bool Calculate(const Call_InstallJob& args, BlueprintRef bpRef, Character* pChar, Rsp_InstallJob& into);
    void EncodeBillOfMaterials(const std::vector< EvERam::RequiredItem >& reqItems, float materialMultiplier, float charMaterialMultiplier, uint32 runs, BillOfMaterials& into);
    void EncodeMissingMaterials(const std::vector< EvERam::RequiredItem >& reqItems, const PathElement& bomLocation, Client*const pClient, float materialMultiplier, float charMaterialMultiplier, int32 runs, std::map< int32, PyRep* >& into);

    void GetBOMItems(const PathElement &bomLocation, std::vector<InventoryItemRef> &into);
    void GetBOMItemsMap(const PathElement &bomLocation, std::map<uint16, InventoryItemRef> &into);

    void GetAdjustedRamRequiredMaterials();
    const char* GetActivityName(int8 activityID);
};

//Singleton
#define sRamMthd \
( RamMethods::get() )


#endif  // EVE_MANUF_RAM_METHODS_H

/* {'messageKey': 'RamAccessDeniedCorpSecStatusTooHigh', 'dataID': 17882212, 'suppressable': False, 'bodyID': 259049, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259048, 'messageID': 1444}
 * {'messageKey': 'RamAccessDeniedCorpSecStatusTooLow', 'dataID': 17882217, 'suppressable': False, 'bodyID': 259051, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259050, 'messageID': 1445}
 * {'messageKey': 'RamAccessDeniedCorpStandingTooLow', 'dataID': 17882222, 'suppressable': False, 'bodyID': 259053, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259052, 'messageID': 1446}
 * {'messageKey': 'RamAccessDeniedNotPilot', 'dataID': 17882227, 'suppressable': False, 'bodyID': 259055, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259054, 'messageID': 1447}
 * {'messageKey': 'RamAccessDeniedSecStatusTooHigh', 'dataID': 17882232, 'suppressable': False, 'bodyID': 259057, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259056, 'messageID': 1448}
 * {'messageKey': 'RamAccessDeniedSecStatusTooLow', 'dataID': 17882237, 'suppressable': False, 'bodyID': 259059, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259058, 'messageID': 1449}
 * {'messageKey': 'RamAccessDeniedStandingTooLow', 'dataID': 17882242, 'suppressable': False, 'bodyID': 259061, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259060, 'messageID': 1450}
 * {'messageKey': 'RamAccessDeniedToBOMHangar', 'dataID': 17882247, 'suppressable': False, 'bodyID': 259063, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259062, 'messageID': 1451}
 * {'messageKey': 'RamAccessDeniedWrongAlliance', 'dataID': 17882252, 'suppressable': False, 'bodyID': 259065, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259064, 'messageID': 1452}
 * {'messageKey': 'RamAccessDeniedWrongCorp', 'dataID': 17882257, 'suppressable': False, 'bodyID': 259067, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259066, 'messageID': 1453}
 * {'messageKey': 'RamActivityInvalid', 'dataID': 17882262, 'suppressable': False, 'bodyID': 259069, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259068, 'messageID': 1454}
 * {'messageKey': 'RamActivityRequiresABlueprint', 'dataID': 17882267, 'suppressable': False, 'bodyID': 259071, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259070, 'messageID': 1455}
 * {'messageKey': 'RamAssemblyLineHasNoActivity', 'dataID': 17880629, 'suppressable': False, 'bodyID': 258466, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258465, 'messageID': 1812}
 * {'messageKey': 'RamBadEndProductForActivity', 'dataID': 17882272, 'suppressable': False, 'bodyID': 259073, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259072, 'messageID': 1456}
 * {'messageKey': 'RamBlueprintAlreadyInstalled', 'dataID': 17878839, 'suppressable': False, 'bodyID': 257778, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257777, 'messageID': 2329}
 * {'messageKey': 'RamCancelJobConfirm', 'dataID': 17880982, 'suppressable': False, 'bodyID': 258602, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 258601, 'messageID': 1857}
 * {'messageKey': 'RamCannotContactFacility', 'dataID': 17878765, 'suppressable': False, 'bodyID': 257749, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257748, 'messageID': 2301}
 * {'messageKey': 'RamCannotCopyABlueprintCopy', 'dataID': 17882277, 'suppressable': False, 'bodyID': 259075, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259074, 'messageID': 1457}
 * {'messageKey': 'RamCannotCopyInPlayerLab', 'dataID': 17878788, 'suppressable': False, 'bodyID': 257758, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 257757, 'messageID': 2314}
 * {'messageKey': 'RamCannotCopyZeroRuns', 'dataID': 17879223, 'suppressable': False, 'bodyID': 257928, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257927, 'messageID': 2496}
 * {'messageKey': 'RamCannotGuaranteeStartTime', 'dataID': 17882282, 'suppressable': False, 'bodyID': 259077, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259076, 'messageID': 1458}
 * {'messageKey': 'RamCannotInstallForCorpByRole', 'dataID': 17882287, 'suppressable': False, 'bodyID': 259079, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259078, 'messageID': 1459}
 * {'messageKey': 'RamCannotInstallForCorpByRoleFactoryManager', 'dataID': 17880671, 'suppressable': False, 'bodyID': 258482, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258481, 'messageID': 1832}
 * {'messageKey': 'RamCannotInstallItemForAnother', 'dataID': 17882292, 'suppressable': False, 'bodyID': 259081, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259080, 'messageID': 1460}
 * {'messageKey': 'RamCannotInstallItemForAnotherCorp', 'dataID': 17882297, 'suppressable': False, 'bodyID': 259083, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259082, 'messageID': 1461}
 * {'messageKey': 'RamCannotInstallWithoutRentFactorySlot', 'dataID': 17880157, 'suppressable': False, 'bodyID': 258285, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258284, 'messageID': 2057}
 * {'messageKey': 'RamCannotInstallWithoutRentResearchSlot', 'dataID': 17879696, 'suppressable': False, 'bodyID': 258105, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258104, 'messageID': 2058}
 * {'messageKey': 'RamCannotInventABlueprintOriginal', 'dataID': 17880061, 'suppressable': False, 'bodyID': 258247, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258246, 'messageID': 2054}
 * {'messageKey': 'RamCannotInventZeroRuns', 'dataID': 17879098, 'suppressable': False, 'bodyID': 257879, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257878, 'messageID': 2497}
 * {'messageKey': 'RamCannotResearchABlueprintCopy', 'dataID': 17882302, 'suppressable': False, 'bodyID': 259085, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259084, 'messageID': 1462}
 * {'messageKey': 'RamCompletionAccessDenied', 'dataID': 17882307, 'suppressable': False, 'bodyID': 259087, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259086, 'messageID': 1463}
 * {'messageKey': 'RamCompletionAccessDeniedByCorpRole', 'dataID': 17882312, 'suppressable': False, 'bodyID': 259089, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259088, 'messageID': 1464}
 * {'messageKey': 'RamCompletionInProduction', 'dataID': 17882317, 'suppressable': False, 'bodyID': 259091, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259090, 'messageID': 1465}
 * {'messageKey': 'RamCompletionJobCompleted', 'dataID': 17882322, 'suppressable': False, 'bodyID': 259093, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259092, 'messageID': 1466}
 * {'messageKey': 'RamCompletionMustBeInShip', 'dataID': 17880657, 'suppressable': False, 'bodyID': 258477, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258476, 'messageID': 1826}
 * {'messageKey': 'RamCompletionNoSuchJob', 'dataID': 17882327, 'suppressable': False, 'bodyID': 259095, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259094, 'messageID': 1467}
 * {'messageKey': 'RamConfigAssemblyLinesAccessDenied', 'dataID': 17882332, 'suppressable': False, 'bodyID': 259097, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259096, 'messageID': 1468}
 * {'messageKey': 'RamConfigAssemblyLinesInsuficientAccess', 'dataID': 17882337, 'suppressable': False, 'bodyID': 259099, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259098, 'messageID': 1469}
 * {'messageKey': 'RamCorpBOMItemNoSuchOffice', 'dataID': 17881000, 'suppressable': False, 'bodyID': 258609, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258608, 'messageID': 1814}
 * {'messageKey': 'RamCorpInstalledItemNotInCargo', 'dataID': 17880634, 'suppressable': False, 'bodyID': 258468, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258467, 'messageID': 1813}
 * {'messageKey': 'RamCorpInstalledItemWrongLocation', 'dataID': 17881073, 'suppressable': False, 'bodyID': 258638, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258637, 'messageID': 1821}
 * {'messageKey': 'RamInstallJob_InstalledItemChanged', 'dataID': 17880977, 'suppressable': False, 'bodyID': 258600, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 258599, 'messageID': 1800}
 * {'messageKey': 'RamInstallationBlueprintLocationError', 'dataID': 17879897, 'suppressable': False, 'bodyID': 258185, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258184, 'messageID': 2179}
 * {'messageKey': 'RamInstallationHasNoDefaultContent', 'dataID': 17882342, 'suppressable': False, 'bodyID': 259101, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259100, 'messageID': 1470}
 * {'messageKey': 'RamInstallationIDUnexpected', 'dataID': 17882347, 'suppressable': False, 'bodyID': 259103, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259102, 'messageID': 1471}
 * {'messageKey': 'RamInstalledItemBadLocation', 'dataID': 17880614, 'suppressable': False, 'bodyID': 258460, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258459, 'messageID': 1809}
 * {'messageKey': 'RamInstalledItemBadLocationStructure', 'dataID': 17881083, 'suppressable': False, 'bodyID': 258642, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258641, 'messageID': 1824}
 * {'messageKey': 'RamInstalledItemInStructureNotInContainer', 'dataID': 17880584, 'suppressable': False, 'bodyID': 258448, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258447, 'messageID': 1803}
 * {'messageKey': 'RamInstalledItemInStructureUnknownLocation', 'dataID': 17880589, 'suppressable': False, 'bodyID': 258450, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258449, 'messageID': 1804}
 * {'messageKey': 'RamInstalledItemMustBeInInstallation', 'dataID': 17880652, 'suppressable': False, 'bodyID': 258475, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258474, 'messageID': 1825}
 * {'messageKey': 'RamInstalledItemMustBeInShip', 'dataID': 17880579, 'suppressable': False, 'bodyID': 258446, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258445, 'messageID': 1802}
 * {'messageKey': 'RamInstalledItemWrongLocation', 'dataID': 17881058, 'suppressable': False, 'bodyID': 258632, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258631, 'messageID': 1820}
 * {'messageKey': 'RamInvalidRegion', 'dataID': 17880930, 'suppressable': False, 'bodyID': 258582, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258581, 'messageID': 1799}
 * {'messageKey': 'RamInventionJobFailed', 'dataID': 17880435, 'suppressable': False, 'bodyID': 258390, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258389, 'messageID': 2052}
 * {'messageKey': 'RamInventionJobSucceeded', 'dataID': 17880545, 'suppressable': False, 'bodyID': 258433, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258432, 'messageID': 2053}
 * {'messageKey': 'RamInventionNoOutput', 'dataID': 17879685, 'suppressable': False, 'bodyID': 258101, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258100, 'messageID': 2048}
 * {'messageKey': 'RamInventionSourceMetaLevelTooHigh', 'dataID': 17880440, 'suppressable': False, 'bodyID': 258392, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258391, 'messageID': 2055}
 * {'messageKey': 'RamInventionWrongDecryptor', 'dataID': 17880472, 'suppressable': False, 'bodyID': 258404, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258403, 'messageID': 2163}
 * {'messageKey': 'RamIsNotAnInstallation', 'dataID': 17882352, 'suppressable': False, 'bodyID': 259105, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259104, 'messageID': 1472}
 * {'messageKey': 'RamItemInTrash', 'dataID': 17878844, 'suppressable': False, 'bodyID': 257780, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257779, 'messageID': 2330}
 * {'messageKey': 'RamLineInUseConfirm', 'dataID': 17881096, 'suppressable': False, 'bodyID': 258647, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 258646, 'messageID': 1852}
 * {'messageKey': 'RamMustSelectInstallation', 'dataID': 17880053, 'suppressable': False, 'bodyID': 258244, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258243, 'messageID': 2051}
 * {'messageKey': 'RamNeedMoreForJob', 'dataID': 17882571, 'suppressable': False, 'bodyID': 259186, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259185, 'messageID': 1473}
 * {'messageKey': 'RamNeedSkillForJob', 'dataID': 17882473, 'suppressable': False, 'bodyID': 259149, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259148, 'messageID': 1474}
 * {'messageKey': 'RamNoCorpAccount', 'dataID': 17878663, 'suppressable': False, 'bodyID': 257711, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257710, 'messageID': 2251}
 * {'messageKey': 'RamNoKnownOutputType', 'dataID': 17882357, 'suppressable': False, 'bodyID': 259107, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259106, 'messageID': 1475}
 * {'messageKey': 'RamNoShip', 'dataID': 17880676, 'suppressable': False, 'bodyID': 258484, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258483, 'messageID': 1833}
 * {'messageKey': 'RamNoSuchInstallation', 'dataID': 17876809, 'suppressable': False, 'bodyID': 257019, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257018, 'messageID': 3249}
 * {'messageKey': 'RamNotYourItemToInstall', 'dataID': 17880639, 'suppressable': False, 'bodyID': 258470, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258469, 'messageID': 1815}
 * {'messageKey': 'RamPleasePickAnInstalltion', 'dataID': 17880619, 'suppressable': False, 'bodyID': 258462, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258461, 'messageID': 1810}
 * {'messageKey': 'RamPleasePickAnItemToInstall', 'dataID': 17880624, 'suppressable': False, 'bodyID': 258464, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258463, 'messageID': 1811}
 * {'messageKey': 'RamProductionTimeExceedsLimits', 'dataID': 17881030, 'suppressable': False, 'bodyID': 258621, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258620, 'messageID': 1816}
 * {'messageKey': 'RamRangeLimitation', 'dataID': 17882547, 'suppressable': False, 'bodyID': 259177, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259176, 'messageID': 1476}
 * {'messageKey': 'RamRangeLimitationJumps', 'dataID': 17881010, 'suppressable': False, 'bodyID': 258613, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258612, 'messageID': 1985}
 * {'messageKey': 'RamRangeLimitationJumpsNoSkill', 'dataID': 17881253, 'suppressable': False, 'bodyID': 258708, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258707, 'messageID': 1986}
 * {'messageKey': 'RamRangeLimitationRegion', 'dataID': 17880905, 'suppressable': False, 'bodyID': 258572, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258571, 'messageID': 1987}
 * {'messageKey': 'RamRemoteInstalledItemImpounded', 'dataID': 17880599, 'suppressable': False, 'bodyID': 258454, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258453, 'messageID': 1806}
 * {'messageKey': 'RamRemoteInstalledItemInStationNotHangar', 'dataID': 17880609, 'suppressable': False, 'bodyID': 258458, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258457, 'messageID': 1808}
 * {'messageKey': 'RamRemoteInstalledItemNotInOffice', 'dataID': 17880604, 'suppressable': False, 'bodyID': 258456, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258455, 'messageID': 1807}
 * {'messageKey': 'RamRemoteInstalledItemNotInStation', 'dataID': 17880594, 'suppressable': False, 'bodyID': 258452, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258451, 'messageID': 1805}
 * {'messageKey': 'RamRevEngNoDecryptors', 'dataID': 17877425, 'suppressable': False, 'bodyID': 257246, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257245, 'messageID': 2907}
 * {'messageKey': 'RamStationIsNotConstructed', 'dataID': 17882362, 'suppressable': False, 'bodyID': 259109, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259108, 'messageID': 1477}
 * {'messageKey': 'RamStructureNotInSpace', 'dataID': 17881078, 'suppressable': False, 'bodyID': 258640, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258639, 'messageID': 1823}
 * {'messageKey': 'RamStructureNotIsSolarsystem', 'dataID': 17880647, 'suppressable': False, 'bodyID': 258473, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258472, 'messageID': 1822}
 * {'messageKey': 'RamTooManyProductionRuns', 'dataID': 17882367, 'suppressable': False, 'bodyID': 259111, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259110, 'messageID': 1478}
 *
 * {'messageKey': 'ReverseEngineeringFailureWithItem', 'dataID': 17878445, 'suppressable': False, 'bodyID': 257628, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257627, 'messageID': 2742}
 * {'messageKey': 'ReverseEngineeringSuccessWithBlueprint', 'dataID': 17878440, 'suppressable': False, 'bodyID': 257626, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257625, 'messageID': 2741}
 * {'messageKey': 'ReverseEngineeringTaskFailed', 'dataID': 17878161, 'suppressable': False, 'bodyID': 257522, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257521, 'messageID': 2740}
 *
 */
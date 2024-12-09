
#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "Profiler.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "effects/EffectsProcessor.h"
#include "inventory/Inventory.h"
#include "npc/Drone.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"
#include "station/Station.h"
#include "system/DestinyManager.h"
#include "system/BubbleManager.h"
#include "system/SolarSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

/*
 * ShipItem
 */
ShipItem::ShipItem(uint32 shipID, const ItemType &type, const ItemData &data)
: InventoryItem(shipID, type, data),
m_pilot(nullptr),
m_targetRef(InventoryItemRef(nullptr)),
m_ModuleManager(new ModuleManager(this)),
m_loaded(false),
m_isActive(false),
m_isPopped(false),
m_isDocking(false),
m_isUndocking(false)
{
    m_onlineModuleVec.clear();
    pInventory = new Inventory(InventoryItemRef(this));

    _log(ITEM__TRACE, "Created ShipItem for %s(%u).", name(), itemID());
}

ShipItem::~ShipItem()
{
    SafeDelete(pInventory);
    SafeDelete(m_ModuleManager);
}

ShipItemRef ShipItem::Load( uint32 shipID)
{
    return InventoryItem::Load<ShipItem>( shipID);
}

ShipItemRef ShipItem::Spawn( ItemData &data) {
    uint32 shipID(InventoryItem::CreateItemID(data));
    if (shipID == 0)
        return ShipItemRef(nullptr);

    return ShipItem::Load( shipID);
}

uint32 ShipItem::CreateItemID( ItemData &data) {
    return InventoryItem::CreateItemID(data);
}

bool ShipItem::_Load()
{
    if (m_loaded and (m_ModuleManager != nullptr))
        return true;

    Client* pClient = sItemFactory.GetUsingClient();
    // test for character creation (which throws errors on following *load() calls and not really needed)
    if ((pClient != nullptr) and pClient->IsCharCreation())
        return true;
    // load attributes
    if (!InventoryItem::_Load())
        return false;
    // load contents
    if (!pInventory->LoadContents())
        return false;

    SetAttribute(AttrVolume, GetPackagedVolume(), false);

    return (m_loaded = true);
}

void ShipItem::LogOut()
{
    SetAttribute(AttrOnline, EvilZero, false);
    SaveShip();

    pInventory->Unload();

    // remove ship item from factory master list here, as *something* changes ship position when saving from factory.
    sItemFactory.RemoveItem(m_itemID);

    // remove ship item from its' container's inventory list also.
    Inventory* pInv(nullptr);
    if (sDataMgr.IsStation(locationID())) {
        pInv = sItemFactory.GetStationRef(locationID())->GetMyInventory();
    } else {
        pInv = sItemFactory.GetSolarSystemRef(locationID())->GetMyInventory();
    }

    if (pInv != nullptr)
        pInv->RemoveItem(ShipItemRef(this));
}

void ShipItem::SetPlayer(Client* pClient) {
    if (m_pilot == pClient)
        return;
    if (pClient == nullptr) {
        SetAttribute(AttrOnline, EvilZero, false);
        if (m_pilot != nullptr)
            pInventory->RemoveItem(m_pilot->GetChar());
        // remove ship effects and char skill effects for char leaving ship.
        ProcessEffects(false);
        // should we check for cargo and damage after char leaves ship?  maybe later
        m_onlineModuleVec.clear();
        m_pilot = nullptr;
        m_isActive = false;
        return;
    }

    m_pilot = pClient;
    if (pClient->IsCharCreation())
        return;

    Init();

    // proc effects when changing ships, or on login.
    ProcessEffects(true, sDataMgr.IsSolarSystem(locationID()));

    // this hits on login and when boarding ship in space.  will not hit on Undock() (location is still station at this point of execution)
    if (sDataMgr.IsSolarSystem(locationID())) {
        SetFlag(flagNone);
        /*  not sure if we're gonna keep this in here....
        if (pClient->IsLogin()) {
            if (sConfig.debug.IsTestServer) {
                // Heal Ship completely on test server
                Heal();
            } else {
                // live server will Recharge shields and cap if session change isnt active
                if (!m_pilot->IsSessionChange()) {
                    SetShipShield(1.0);
                    SetShipCapacitorLevel(1.0);
                }
            }
        } */
    }
}

void ShipItem::Init()
{
    // pods have 57 attribs and 0 effects
    if (groupID() == EVEDB::invGroups::Capsule) {
        InitPod();
        return;
    }

    InitAttribs();

    m_isActive = true;

    m_ModuleManager->Initialize();

    // load linked weapons (if available)
    LoadWeaponGroups();

    if (sConfig.server.CargoMassAdditive)
        UpdateMass();
}

void ShipItem::InitPod() {
    SetAttribute(AttrOnline,                            EvilOne, false);
    SetAttribute(AttrVolume,                            GetPackagedVolume(), false);
    SetAttribute(AttrCpuLoad,                           EvilZero, false);
    SetAttribute(AttrPowerLoad,                         EvilZero, false);

    // Check for existence of attributes.  if not loaded then set them to default values:
    if (!HasAttribute(AttrDamage))                      SetAttribute(AttrDamage, EvilZero, false);
    if (!HasAttribute(AttrArmorDamage))                 SetAttribute(AttrArmorDamage, EvilZero, false);
    // shield and cap are part of persistance, and loaded on attrib map initalization.  check for and set to full if no saved value found
    if (!HasAttribute(AttrShieldCharge))                SetAttribute(AttrShieldCharge,  GetAttribute(AttrShieldCapacity), false);

    m_ModuleManager->Initialize();

    // pod will always be full when activated
    if (m_pilot != nullptr)
        if (m_pilot->IsInSpace())
            Heal();
}

void ShipItem::InitAttribs()
{
    // Create default dynamic attributes in the AttributeMap
    SetAttribute(AttrOnline,                            EvilOne, false);
    SetAttribute(AttrVolume,                            GetPackagedVolume(), false);
    SetAttribute(AttrMass,                              type().mass());
    SetAttribute(AttrCpuLoad,                           EvilZero, false);
    SetAttribute(AttrPowerLoad,                         EvilZero, false);
    // rig shit
    SetAttribute(AttrUpgradeLoad,                       EvilZero, false);

    // Check for existence of attributes.  if not loaded then set them to default values:
    if (!HasAttribute(AttrDamage))                      SetAttribute(AttrDamage, EvilZero, false);
    if (!HasAttribute(AttrArmorDamage))                 SetAttribute(AttrArmorDamage, EvilZero, false);
    // shield and cap are part of persistance, and loaded on attrib map initalization.  check for and set to full if no saved value found
    if (!HasAttribute(AttrShieldCharge))                SetAttribute(AttrShieldCharge,  GetAttribute(AttrShieldCapacity), false);
    if (!HasAttribute(AttrCapacitorCharge))             SetAttribute(AttrCapacitorCharge,  GetAttribute(AttrCapacitorCapacity), false);
    if (!HasAttribute(AttrMaximumRangeCap))             SetAttribute(AttrMaximumRangeCap, ((float)BUBBLE_RADIUS_METERS), false);
    // Warp Scramble Status of the ship (most ships have zero warp scramble status, but some (t2 indy) already have it defined):
    if (!HasAttribute(AttrWarpScrambleStatus))          SetAttribute(AttrWarpScrambleStatus, EvilZero, false);
    if (!HasAttribute(AttrWarpSpeedMultiplier))         SetAttribute(AttrWarpSpeedMultiplier, EvilOne, false);
    if (!HasAttribute(AttrArmorMaxDamageResonance))     SetAttribute(AttrArmorMaxDamageResonance, EvilOne, false);
    if (!HasAttribute(AttrShieldMaxDamageResonance))    SetAttribute(AttrShieldMaxDamageResonance, EvilOne, false);
    // hull res is stored in item type as AttrHull*Resonance for 6 ships.  set accordingly
    if (!HasAttribute(AttrEmDamageResonance))           SetAttribute(AttrEmDamageResonance,  GetAttribute(AttrHullEmDamageResonance), false);
    if (!HasAttribute(AttrKineticDamageResonance))      SetAttribute(AttrKineticDamageResonance,  GetAttribute(AttrHullKineticDamageResonance), false);
    if (!HasAttribute(AttrThermalDamageResonance))      SetAttribute(AttrThermalDamageResonance,  GetAttribute(AttrHullThermalDamageResonance), false);
    if (!HasAttribute(AttrExplosiveDamageResonance))    SetAttribute(AttrExplosiveDamageResonance,  GetAttribute(AttrHullExplosiveDamageResonance), false);

    // ship bonuses....are these set in ship fx?
    /*
    AttrMaxRangeBonus = 351,
    AttrDroneRangeBonus = 459,
    AttrExplosionRangeBonus = 558,
    AttrMaxRangeBonus2 = 769,

    */

    // cap ships have bonuses to various systems....these may be set in ship fx
    /*
    AttrSurveyScannerRangeBonus = 1234,
    AttrCargoScannerRangeBonus = 1235,
    AttrCommandBonusEffective = 1236,
    AttrCommandBonusAdd = 1237,
    AttrCommandBonusEffectiveAdd = 1238,
    AttrShipBonusORECapital1 = 1239,
    AttrShipBonusORECapital2 = 1240,
    AttrShipBonusORECapital3 = 1243,
    AttrShipBonusORECapital4 = 1244,

    AttrEliteBonusHeavyInterdictors1 = 1246,
    AttrEliteBonusHeavyInterdictors2 = 1247,
    AttrEliteBonusElectronicAttackShip1 = 1249,
    AttrEliteBonusElectronicAttackShip2 = 1250,
    AttrEliteBonusViolators1 = 1265,
    AttrEliteBonusViolators2 = 1266,
    AttrEliteBonusJumpFreighter1 = 1311,
    AttrEliteBonusJumpFreighter2 = 1312,
    AttrMaxTargetRangeBonusBonus = 1313,
    AttrScanResolutionBonusBonus = 1314,
    AttrMaxRangeBonusBonus = 1315,
    AttrTrackingSpeedBonusBonus = 1316,
    AttrEliteBonusViolatorsRole1 = 1268,
    AttrEliteBonusViolatorsRole2 = 1269,
    AttrWarpScrambleRangeBonus = 1327,
    AttrDroneBandwidthMultiplier = 1328,
    AttrDroneBandwidthBonusAdd = 1329,
    AttrShipOrcaTractorBeamRangeBonus1 = 1355,
    AttrShipOrcaCargoBonusOrca1 = 1356,
    AttrShipOrcaTractorBeamVelocityBonus2 = 1357,
    AttrShipOrcaForemanBonus = 1358,
    AttrShipOrcaSurveyScannerBonus = 1359,
    AttrShipBonusHPExtender1 = 1360,
    AttrEliteIndustrialCovertCloakBonus = 1361,

    AttrShipBonusOreIndustrial1 = 1669,
    AttrShipBonusOreIndustrial2 = 1670,

    AttrtitanAmarrBonus5 = 1592,
    AttrtitanGallenteBonus5 = 1593,
    AttrtitanMinmatarBonus5 = 1594,
    AttrtitanCaldariBonus5 = 1596,

    */

    // t3 ships
    /*
    AttrSubsystemBonusAmarrEngineering = 1431,
    AttrSubsystemBonusAmarrElectronic = 1432,
    AttrSubsystemBonusAmarrDefensive = 1433,
    AttrSubsystemBonusAmarrOffensive = 1434,
    AttrSubsystemBonusAmarrPropulsion = 1435,
    AttrSubsystemBonusGallenteEngineering = 1436,
    AttrSubsystemBonusGallenteElectronic = 1437,
    AttrSubsystemBonusGallenteDefensive = 1438,
    AttrSubsystemBonusGallenteOffensive = 1439,
    AttrSubsystemBonusGallentePropulsion = 1440,
    AttrSubsystemBonusCaldariEngineering = 1441,
    AttrSubsystemBonusCaldariElectronic = 1442,
    AttrSubsystemBonusCaldariDefensive = 1443,
    AttrSubsystemBonusCaldariOffensive = 1444,
    AttrSubsystemBonusCaldariPropulsion = 1445,
    AttrSubsystemBonusMinmatarEngineering = 1446,
    AttrSubsystemBonusMinmatarElectronic = 1447,
    AttrSubsystemBonusMinmatarDefensive = 1448,
    AttrSubsystemBonusMinmatarOffensive = 1449,
    AttrSubsystemBonusMinmatarPropulsion = 1450,

    AttrShipBonusStrategicCruiserAmarr = 1503,
    AttrShipBonusStrategicCruiserCaldari = 1504,
    AttrShipBonusStrategicCruiserGallente = 1505,
    AttrShipBonusStrategicCruiserMinmatar = 1506,

    AttrSubsystemBonusAmarrDefensive2 = 1507,
    AttrSubsystemBonusAmarrElectronic2 = 1508,
    AttrSubsystemBonusAmarrEngineering2 = 1509,
    AttrSubsystemBonusCaldariOffensive2 = 1510,
    AttrSubsystemBonusAmarrOffensive2 = 1511,
    AttrSubsystemBonusAmarrPropulsion2 = 1512,
    AttrSubsystemBonusCaldariPropulsion2 = 1513,
    AttrSubsystemBonusCaldariElectronic2 = 1514,
    AttrSubsystemBonusCaldariEngineering2 = 1515,
    AttrSubsystemBonusCaldariDefensive2 = 1516,
    AttrSubsystemBonusGallenteDefensive2 = 1517,
    AttrSubsystemBonusGallenteElectronic2 = 1518,
    AttrSubsystemBonusGallenteEngineering2 = 1519,
    AttrSubsystemBonusGallentePropulsion2 = 1520,
    AttrSubsystemBonusGallenteOffensive2 = 1521,
    AttrSubsystemBonusMinmatarOffensive2 = 1522,
    AttrSubsystemBonusMinmatarPropulsion2 = 1523,
    AttrSubsystemBonusMinmatarElectronic2 = 1524,
    AttrSubsystemBonusMinmatarEngineering2 = 1525,
    AttrSubsystemBonusMinmatarDefensive2 = 1526,

    AttrSubsystemBonusAmarrOffensive3 = 1531,
    AttrSubsystemBonusGallenteOffensive3 = 1532,
    AttrSubsystemBonusCaldariOffensive3 = 1533,
    AttrSubsystemBonusMinmatarOffensive3 = 1534,
    */
}

void ShipItem::Delete() {
    pInventory->DeleteContents();
    InventoryItem::Delete();
}

/** @todo this will need more work to correctly check hold capacity for offline/unloaded ships */
float ShipItem::GetRemainingVolumeByFlag(EVEItemFlags flag) const {
    // updated to use inventory  -allan 26Jul16  -fixed 22Nov18  -updated to new call 08Aug20
    return pInventory->GetRemainingCapacity(flag);
}

void ShipItem::ProcessModules() {
    if (m_pilot->IsDocked())
        return;

    m_ModuleManager->Process();
}

void ShipItem::Eject()
{
    if (m_ModuleManager != nullptr) {
        m_ModuleManager->AbortCycle();
        m_ModuleManager->CharacterLeavingShip();
    }
    SaveShip();
    // linked weapons will be persistant here.
    //UnlinkAllWeapons();
}

void ShipItem::Dock() {
    m_isDocking = true;

    m_ModuleManager->DeactivateAllModules();
    m_onlineModuleVec.clear();
    // remove ship effects and char skill effects for docking.
    ProcessEffects();
}

void ShipItem::Undock() {
    m_isUndocking = true;

    //HorribleFittingProblems

    ResetEffects();
    //ProcessEffects(true, true);

    // horrible hack to set charge qty in fit window
    // on the off-chance player opens the fit window
    m_ModuleManager->UpdateChargeQty();

    // Recharge shields and cap if session change isnt active (undocking too fast)
    if (!m_pilot->IsSessionChange()) {
        SetShipShield(1.0);
        SetShipCapacitorLevel(1.0);
    }
}

void ShipItem::UpdateMass()
{
    std::map< uint32, InventoryItemRef > invMap;
    pInventory->GetInventoryMap(invMap);
    uint32 mass(GetAttribute(AttrMass).get_uint32());
    for (auto cur : invMap)
        mass += cur.second->type().mass() * cur.second->quantity();

    // may have to adjust this for player login
    SetAttribute(AttrMass, mass, HasPilot());
}

void ShipItem::Warp() {
    m_ModuleManager->ShipWarping();
}

void ShipItem::Jump() {
    m_ModuleManager->ShipJumping();
}
/* {'messageKey': 'JumpAlreadyHaveStationClone', 'dataID': 17882749, 'suppressable': False, 'bodyID': 259250, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1009}
 * {'messageKey': 'JumpAttemptTooSoon', 'dataID': 17883129, 'suppressable': False, 'bodyID': 259393, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1010}
 * {'messageKey': 'JumpBeaconInWormholeSpace', 'dataID': 17877290, 'suppressable': False, 'bodyID': 257194, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2829}
 * {'messageKey': 'JumpCantAcceptOwnInstall', 'dataID': 17883135, 'suppressable': False, 'bodyID': 259395, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1011}
 * {'messageKey': 'JumpCantDestroyCloneInOthersShip', 'dataID': 17882752, 'suppressable': False, 'bodyID': 259251, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1012}
 * {'messageKey': 'JumpCantDestroyCloneNoRight', 'dataID': 17882755, 'suppressable': False, 'bodyID': 259252, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1013}
 * {'messageKey': 'JumpCantUseCFG', 'dataID': 17882760, 'suppressable': False, 'bodyID': 259254, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259253, 'messageID': 1015}
 * {'messageKey': 'JumpCharStoringMaxClones', 'dataID': 17883140, 'suppressable': False, 'bodyID': 259397, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259396, 'messageID': 1016}
 * {'messageKey': 'JumpCharStoringMaxClonesNone', 'dataID': 17879907, 'suppressable': False, 'bodyID': 258189, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258188, 'messageID': 2182}
 * {'messageKey': 'JumpCheckIntoShip', 'dataID': 17879917, 'suppressable': False, 'bodyID': 258193, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 258192, 'messageID': 2187}
 * {'messageKey': 'JumpCheckWillLoseExistingClone', 'dataID': 17882765, 'suppressable': False, 'bodyID': 259256, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 259255, 'messageID': 1017}
 * {'messageKey': 'JumpCheckWillLoseExistingCloneAndImplants', 'dataID': 17882770, 'suppressable': False, 'bodyID': 259258, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 259257, 'messageID': 1018}
 * {'messageKey': 'JumpCloneInstallationOffered', 'dataID': 17883248, 'suppressable': False, 'bodyID': 259436, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259435, 'messageID': 1020}
 * {'messageKey': 'JumpCloneNotFound', 'dataID': 17882773, 'suppressable': False, 'bodyID': 259259, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1021}
 * {'messageKey': 'JumpCriminalConfirm', 'dataID': 17877821, 'suppressable': False, 'bodyID': 257396, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 257395, 'messageID': 2544}
 * {'messageKey': 'JumpDestinationBlocked', 'dataID': 17875972, 'suppressable': False, 'bodyID': 256704, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256703, 'messageID': 3497}
 * {'messageKey': 'JumpDestinationInvalid', 'dataID': 17882778, 'suppressable': False, 'bodyID': 259261, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259260, 'messageID': 1022}
 * {'messageKey': 'JumpDestinationIsWormholeSpace', 'dataID': 17877287, 'suppressable': False, 'bodyID': 257193, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2828}
 * {'messageKey': 'JumpDestinationNotWilling', 'dataID': 17883025, 'suppressable': False, 'bodyID': 259352, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1023}
 * {'messageKey': 'JumpDriveActive', 'dataID': 2039122, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1024}
 * {'messageKey': 'JumpHarmonicsDoNotMatch', 'dataID': 17878894, 'suppressable': False, 'bodyID': 257799, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2383}
 * {'messageKey': 'JumpInstalleeNotPresent', 'dataID': 17882781, 'suppressable': False, 'bodyID': 259262, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1025}
 * {'messageKey': 'JumpInstalleeTooDistant', 'dataID': 17883170, 'suppressable': False, 'bodyID': 259408, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1026}
 * {'messageKey': 'JumpInstallerNotPresent', 'dataID': 17882784, 'suppressable': False, 'bodyID': 259263, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1027}
 * {'messageKey': 'JumpMustBeInPod', 'dataID': 17878190, 'suppressable': False, 'bodyID': 257533, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257532, 'messageID': 2744}
 * {'messageKey': 'JumpNoBridge', 'dataID': 17882789, 'suppressable': False, 'bodyID': 259265, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259264, 'messageID': 1028}
 * {'messageKey': 'JumpNoCloneAtDestination', 'dataID': 17882792, 'suppressable': False, 'bodyID': 259266, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1029}
 * {'messageKey': 'JumpNoDrive', 'dataID': 17882797, 'suppressable': False, 'bodyID': 259268, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259267, 'messageID': 1030}
 * {'messageKey': 'JumpNoPortal', 'dataID': 17882802, 'suppressable': False, 'bodyID': 259270, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259269, 'messageID': 1031}
 * {'messageKey': 'JumpNoShipCloneForAbsentChar', 'dataID': 17883173, 'suppressable': False, 'bodyID': 259409, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1032}
 * {'messageKey': 'JumpNoShipCloneForBusyChar', 'dataID': 17883176, 'suppressable': False, 'bodyID': 259410, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1033}
 * {'messageKey': 'JumpNoShipCloneWhenDoingShipOne', 'dataID': 17883179, 'suppressable': False, 'bodyID': 259411, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1034}
 * {'messageKey': 'JumpNoStationCloneWhenDoingShipOne', 'dataID': 17882805, 'suppressable': False, 'bodyID': 259271, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1035}
 * {'messageKey': 'JumpNotEnoughCap', 'dataID': 17883189, 'suppressable': False, 'bodyID': 259415, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259414, 'messageID': 1036}
 * {'messageKey': 'JumpNotEnoughCharge', 'dataID': 17883194, 'suppressable': False, 'bodyID': 259417, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259416, 'messageID': 1037}
 * {'messageKey': 'JumpNotEnoughCharge2', 'dataID': 17883199, 'suppressable': False, 'bodyID': 259419, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259418, 'messageID': 1038}
 * {'messageKey': 'JumpNotEnoughCharge3', 'dataID': 17879346, 'suppressable': False, 'bodyID': 257973, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257972, 'messageID': 2342}
 * {'messageKey': 'JumpOtherAlreadyHasShipClone', 'dataID': 17883205, 'suppressable': False, 'bodyID': 259421, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1039}
 * {'messageKey': 'JumpShipCloneInstallAcceptTooLate', 'dataID': 17882808, 'suppressable': False, 'bodyID': 259272, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1040}
 * {'messageKey': 'JumpShipCloneInstallAcceptWTF', 'dataID': 17882811, 'suppressable': False, 'bodyID': 259273, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1041}
 * {'messageKey': 'JumpShipCloneInstallCancelTooLate', 'dataID': 17882814, 'suppressable': False, 'bodyID': 259274, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1042}
 * {'messageKey': 'JumpShipCloneInstallCancelWTF', 'dataID': 17882817, 'suppressable': False, 'bodyID': 259275, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1043}
 * {'messageKey': 'JumpShipLacksActiveCloneVatBay', 'dataID': 17877847, 'suppressable': False, 'bodyID': 257406, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2557}
 * {'messageKey': 'JumpShipLacksCloneSupport', 'dataID': 17882820, 'suppressable': False, 'bodyID': 259276, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1044}
 * {'messageKey': 'JumpShipNotOwnedByCharacter', 'dataID': 17882823, 'suppressable': False, 'bodyID': 259277, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1045}
 * {'messageKey': 'JumpShipStoringMaxClones', 'dataID': 17883211, 'suppressable': False, 'bodyID': 259423, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1046}
 * {'messageKey': 'JumpSkillInTraining', 'dataID': 17880967, 'suppressable': False, 'bodyID': 258596, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1817}
 * {'messageKey': 'JumpSuperweaponActive', 'dataID': 17879575, 'suppressable': False, 'bodyID': 258058, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258057, 'messageID': 2306}
 * {'messageKey': 'JumpTooFar', 'dataID': 17883219, 'suppressable': False, 'bodyID': 259426, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259425, 'messageID': 1047}
 */
void ShipItem::Heal()
{
    // Repair Ship, Modules and Recharge Capacitor:
    SetShipShield(1.0);
    SetShipCapacitorLevel(1.0);
    SetShipArmor(1.0);
    SetShipHull(1.0);
    m_ModuleManager->RepairModules();
}

void ShipItem::AddItem(InventoryItemRef iRef)
{
    if (iRef.get() == nullptr)
        return;

    InventoryItem::AddItem(iRef);

    // add item mass to ship's mass if set in options (additive...loaded ship should be heavy)
    if (sConfig.server.CargoMassAdditive) {
        uint32 mass = GetAttribute(AttrMass).get_uint32();
        uint32 addition = iRef->type().mass() * iRef->quantity();
        SetAttribute(AttrMass, mass + addition, HasPilot());
    }

    /** @todo update destiny mass after adding item */
}

// cannot throw
uint32 ShipItem::AddItemByFlag(EVEItemFlags flag, InventoryItemRef iRef, Client* pClient/*nullptr*/)
{
    if (iRef.get() == nullptr)
        return 0;

    if (flag == flagNone) {
        // make error.  nothing at this point should be "autoFit"
        codelog(SHIP__ERROR, "ShipItem::AddItem() - flag = flagNone.");
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
        flag = flagCargoHold;   //default to cargo (cause this is a ship)
    }
    // CantFitModuleToThatShip
    // u'CantFitModuleToThatShipBody'}(u"You can't fit {item} to {ship}", None, {u'{ship}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'ship'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}})

    if (IsModuleSlot(flag)) {
        if (iRef->categoryID() == EVEDB::invCategories::Charge) {
            iRef->ChangeSingleton(false, false);
            m_ModuleManager->LoadCharge(iRef, flag);
            iRef = m_ModuleManager->GetLoadedChargeOnModule(flag);
            if (iRef.get() == nullptr)
                return 0;
            return iRef->itemID();
        } else if (iRef->categoryID() == EVEDB::invCategories::Module) {
            ModuleItemRef mRef = ModuleItemRef::StaticCast(iRef);
            mRef->ChangeSingleton(true, false);
            // rigs are classed in the module category.  check here and call appropriate method as needed.
            if (IsRigSlot(flag)) {
                if (!m_ModuleManager->InstallRig(mRef, flag))
                    return 0;
            } else if (!m_ModuleManager->AddModule(mRef, flag))
                return 0;
        } else if (iRef->categoryID() == EVEDB::invCategories::Subsystem) {
            ModuleItemRef mRef = ModuleItemRef::StaticCast(iRef);
            //mRef->SetOnline(true);  // is this needed here?
            mRef->ChangeSingleton(true, false);
            if (!m_ModuleManager->InstallSubSystem(mRef, flag))
                return 0;
        }
        //m_ModuleManager->UpdateModules(flag);
    }

    iRef->Move(m_itemID, flag, true);
    return iRef->itemID();
}

void ShipItem::RemoveItem(InventoryItemRef iRef)
{
    if (iRef.get() == nullptr)
        return;

    InventoryItem::RemoveItem(iRef);

    if (m_pilot == nullptr)
        return;

    // check to see if item is currently in a module slot.
    if (IsModuleSlot(iRef->flag())) {
        if (IsRigSlot(iRef->flag())) {
            m_ModuleManager->UninstallRig(iRef->itemID());
        } else if (iRef->categoryID() == EVEDB::invCategories::Charge) {
            m_ModuleManager->UnloadModule(iRef->flag());
        } else {
            m_ModuleManager->UnfitModule(iRef->itemID());
        }
        //m_ModuleManager->UpdateModules(iRef->flag());
    }

    // remove item mass to ship's mass if set in options (additive...loaded ship should be heavy)
    if (sConfig.server.CargoMassAdditive) {
        uint32 mass = GetAttribute(AttrMass).get_uint32();
        uint32 addition = iRef->type().mass() * iRef->quantity();
        SetAttribute(AttrMass, mass - addition, HasPilot());
    }
}

void ShipItem::AddModuleToOnlineVec(uint32 modID)
{
    m_onlineModuleVec.push_back(modID);
    _log(MODULE__INFO, "Added ModuleID %u to Online List", modID);
}

void ShipItem::GetModuleItemVec( std::vector< InventoryItemRef >& iRefVec ) {
    std::map<uint32, InventoryItemRef> invMap;
    pInventory->GetInventoryMap( invMap );
    for (auto cur : invMap )
        if (IsModuleSlot(cur.second->flag()))
            iRefVec.push_back(cur.second);
}

/* Begin new Module Manager Interface */
void ShipItem::GetModuleRefVec(std::vector< InventoryItemRef >& iRefVec)
{
    m_ModuleManager->GetModuleListOfRefsAsc(iRefVec);
}

InventoryItemRef ShipItem::GetModuleRef(EVEItemFlags flag)
{
    GenericModule* pMod(m_ModuleManager->GetModule(flag));
    if (pMod != nullptr)
        return pMod->GetSelf();

    return InventoryItemRef(nullptr);
}

InventoryItemRef ShipItem::GetModuleRef(uint32 modID)
{
    GenericModule* pMod(m_ModuleManager->GetModule(modID));
    if (pMod != nullptr)
        return pMod->GetSelf();

    return InventoryItemRef(nullptr);
}

// only called from dogma call.  can throw
void ShipItem::LoadCharge(InventoryItemRef cRef, EVEItemFlags flag)
{
    if (cRef.get() == nullptr)
        throw UserError ("CantFindChargeToAdd");

    if (!IsFittingSlot(flag))
        throw CustomError ("Destination is not weapon.");

    if (IsFittingSlot(cRef->flag())) {
        _log(MODULE__TRACE, "ShipItem::LoadCharge - Trying to load %s from %s to %s.", \
                    cRef->name(), sDataMgr.GetFlagName(cRef->flag()), sDataMgr.GetFlagName(flag));
        throw UserError ("CantMoveChargesBetweenModules");
    }

    GenericModule* pMod(m_ModuleManager->GetModule(flag));
    if (pMod == nullptr)
        throw UserError ("ModuleNoLongerPresentForCharges");

    if (pMod->IsActive()) {
        throw CustomError ("You cannot load active modules.");
        /*
        std::map<std::string, PyRep *> args;
        args["chargeType"] = new PyInt(iRef->typeID());
        throw PyException( MakeUserError("LoadingChargeSlotAlready", args));  */
        /*LoadingChargeSlotAlreadyBody'}(u'You cannot load the {[item]chargeType.name} because the module is already involved in another loading operation.'
         * , None, {u'{[item]chargeType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'chargeType'}})
         */
    }
    if (pMod->GetModuleState() == Module::State::Loading) {
        throw UserError ("LoadingChargeSlotAlready")
                .AddFormatValue ("chargeType", new PyInt (cRef->typeID ()));
        //throw PyException( MakeUserError("LoadingChargeAlready", args));
        /*LoadingChargeAlreadyBody'}(u'Some or all of {[item]chargeType.name} is already being loaded into a module.
         * If you wish to load what remains, you will have to wait until this is finished.',
         * None, {u'{[item]chargeType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'chargeType'}})
         */
    }

    VerifyHoldType(flag, cRef, m_pilot);
        /*  this doesnt work right....comment for now.
        std::map<std::string, PyRep *> args;
        args["charge"] = new PyInt(iRef->itemID());
        throw UserError ("ChargeLoadingFailedWithRefund");
        */
        /* ChargeLoadingFailedWithRefundBody'}(u'Your {[item]charge.name} failed to load and was returned to your cargo.',
         * None, {u'{[item]charge.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'charge'}})
         */
    m_ModuleManager->LoadCharge(cRef, flag);
}
/*
 * ChargeRequiresLauncher
 * DragDropCharge ??

 * IncompatibleChargeGroup1  {'FullPath': u'UI/Messages', 'messageID': 259621, 'label': u'IncompatibleChargeGroup1Body'}(u'You cannot load {ammoGroup} into a {[item]moduleName.name}, this module needs to be loaded with {group1}.', None, {u'{ammoGroup}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'ammoGroup'}, u'{[item]moduleName.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}, u'{group1}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'group1'}})
NotEnoughChargeSpace  {'FullPath': u'UI/Messages', 'messageID': 259162, 'label': u'NotEnoughChargeSpaceBody'}(u'There is not enough space to add those charges. There is only {[numeric]capacity, decimalPlaces=2} {[numeric]capacity -> "unit", "units"} of space, but the charges require {[numeric]volume, decimalPlaces=2} {[numeric]volume -> "unit", "units"}.', None, {u'{[numeric]capacity, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'capacity'}, u'{[numeric]capacity -> "unit", "units"}': {'conditionalValues': [u'unit', u'units'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'capacity'}, u'{[numeric]volume, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'volume'}, u'{[numeric]volume -> "unit", "units"}': {'conditionalValues': [u'unit', u'units'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'volume'}})
{'FullPath': u'UI/Messages', 'messageID': 257216, 'label': u'NoSuitableModulesBody'}(u'These charges cannot be fitted to any of the currently fitted modules', None, None)

 */

// only called from dogma call.  can throw
void ShipItem::LoadChargesToBank(EVEItemFlags flag, std::vector< int32 >& chargeIDs)
{
    int8 pos = 0;
    InventoryItemRef cRef(nullptr);
    std::vector<GenericModule*> modVec;
    m_ModuleManager->GetModulesInBank(flag, modVec);
    for (auto cur : modVec) {
        if (pos + 1 > chargeIDs.size())
            return;
        cRef = sItemFactory.GetItemRef(chargeIDs[pos]);
        if (cRef.get() == nullptr) {
            ++pos;
            continue;
        }
        if (IsCargoHoldFlag(cRef->flag()) or IsHangarFlag(cRef->flag())) {
            LoadCharge(cRef, cur->flag());
        } else {
            ++pos;
        }
    }
}

// only called from dogma call.  can throw
void ShipItem::LoadLinkedWeapons(GenericModule* pMod, std::vector<int32>& chargeIDs)
{
    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod);
    if (itr == m_linkedWeapons.end())
        return;

    int8 pos = 0;
    InventoryItemRef cRef(sItemFactory.GetItemRef(chargeIDs[pos]));
    if (cRef.get() == nullptr)
        throw UserError ("CantFindChargeToAdd");

    int8 size(chargeIDs.size());
    //load charge in master
    VerifyHoldType(pMod->flag(), cRef, m_pilot);
    m_ModuleManager->LoadCharge(cRef, pMod->flag());
    // loop thru slaves and load charge(s)
    std::list<GenericModule*>::iterator itr2 = itr->second.begin();
    while ((itr2 != itr->second.end()) and (pos <= size)) {
        cRef = sItemFactory.GetItemRef(chargeIDs[pos]);
        if (cRef.get() == nullptr){
            ++pos;
        } else if (IsCargoHoldFlag(cRef->flag()) or IsHangarFlag(cRef->flag())) {
            VerifyHoldType((*itr2)->flag(), cRef, m_pilot);
            m_ModuleManager->LoadCharge(cRef, (*itr2)->flag());
            ++itr2;
        } else {
            ++pos;
        }
    }
}

//{'FullPath': u'UI/Messages', 'messageID': 257270, 'label': u'NotEnoughCargoSpaceToUnloadBankBody'}(u'There is not enough cargo space left to unload the charges in the weapon bank. Try freeing up some space and try again.', None, None)

// only called from client call.  can throw
void ShipItem::RemoveCharge(EVEItemFlags fromFlag)
{
    if (IsFittingSlot(fromFlag)) {
        GenericModule* pMod(m_ModuleManager->GetModule(fromFlag));
        if (pMod == nullptr)
            throw CustomError ("Module was not found at %s.", sDataMgr.GetFlagName(fromFlag));

        if (pMod->IsActive())
            throw UserError ("CannotAccessChargeWhileInUse");

        if (!pMod->IsLoaded())
            throw CustomError ("Your %s is not loaded.", pMod->GetSelf()->name());

        m_ModuleManager->UnloadCharge(pMod);
    }
}

// only called from inv bound call.  can throw
void ShipItem::TryModuleLimitChecks(EVEItemFlags flag, InventoryItemRef iRef)
{
    if (m_ModuleManager->IsSlotOccupied(flag))
        throw UserError ("SlotAlreadyOccupied");

    m_ModuleManager->CheckSlotFitLimited(flag);
    m_ModuleManager->CheckGroupFitLimited(flag, iRef);

    if (IsHiSlot(flag)) {
        // check available turret/launcher hardpoints
        if (iRef->type().HasEffect(EVEEffectID::turretFitted)) {
            if (GetAttribute(AttrTurretSlotsLeft) < 1) {
                throw UserError ("NotEnoughTurretSlots")
                        .AddTypeName ("moduleName", iRef->typeID ());
                /*u'NotEnoughTurretSlotsBody'}(u"You cannot fit the {moduleName} because your ship doesn't have any turret slots left for fitting, possibly because you have already filled your ship with turrets or that the ship simply can not be fitted with turrets.\r\n<br>
                 * <br>Turret slots represent how many weapons of a certain type can be fitted on a ship. The current design is over a hundred years old, and is modular enough to allow for a great leeway in the fitting of various weaponry.", None,
                 * {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
                 */
            }
        } else if (iRef->type().HasEffect(EVEEffectID::launcherFitted)) {
            if (GetAttribute(AttrLauncherSlotsLeft) < 1) {
                throw UserError ("NotEnoughLauncherSlots")
                        .AddTypeName ("moduleName", iRef->typeID ());
                /*NotEnoughLauncherSlotsBody'}(u"You cannot fit the {moduleName} because your ship doesn't have any launcher slots left for fitting, possibly because you have already filled your ship with launchers or that the ship simply can not be fitted with launchers.<br>
                 * <br>Launcher slots represent how many weapons of a certain type can be fitted on a ship. The current design is over a hundred years old, and is modular enough to allow for a great leeway in the fitting of various weaponry.", None,
                 * {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
                 */
            }
        }
    } else if (IsRigSlot(flag)) {
        if (GetAttribute(AttrRigSize) != iRef->GetAttribute(AttrRigSize)) {
            throw UserError ("CannotFitRigWrongSize")
                    .AddFormatValue ("rigSize", new PyString (sDataMgr.GetRigSizeName (iRef->GetAttribute (AttrRigSize).get_uint32())))
                    .AddTypeName ("item", iRef->typeID ())
                    .AddFormatValue ("shipRigSize", new PyString (sDataMgr.GetRigSizeName (GetAttribute (AttrRigSize).get_uint32())));
            /* CannotFitRigWrongSizeBody'}(u'{item} does not fit in this slot.
             * The slot takes size {shipRigSize} rigs, but the item is size {rigSize}.', None,
             * {u'{rigSize}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rigSize'},
             * u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'},
             * u'{shipRigSize}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipRigSize'}})
             * check avalible rig slots and ship upgrade capy
             */
        }
        if (GetAttribute(AttrUpgradeSlotsLeft) < 1) {
            throw UserError ("NotEnoughUpgradeSlots")
                    .AddFormatValue ("moduleType", new PyInt (iRef->typeID ()));
            /*NotEnoughUpgradeSlotsBody'}(u"You cannot fit the {[item]moduleType.name} because your ship doesn't have any upgrade slots left for fitting, possibly because you have already filled your ship with upgrades or that the ship simply can not be fitted with upgrades.", None,
             * {u'{[item]moduleType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleType'}})
             */
        }

        if ((GetAttribute(AttrUpgradeLoad) + iRef->GetAttribute(AttrUpgradeCost)) > GetAttribute(AttrUpgradeCapacity)) {
            throw UserError ("NotEnoughUpgradeCapacity")
                    .AddTypeName ("moduleName", iRef->typeID ());
            /*NotEnoughUpgradeCapacityBody'}(u'You cannot fit the {moduleName} because your ship cannot handle it. Your ship can only fit so many upgrades as each interferes with its calibration, and past a certain point your ship is rendered unusable.', None,
             * {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
             */
        }
    }
}

EVEItemFlags ShipItem::FindAvailableModuleSlot(InventoryItemRef iRef) {
    uint16 slotFound(flagIllegal);
    if (iRef->type().HasEffect(EVEEffectID::loPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::loPower);
    } else if (iRef->type().HasEffect(EVEEffectID::medPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::medPower);
    } else if (iRef->type().HasEffect(EVEEffectID::hiPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::hiPower);
    } else if (iRef->type().HasEffect(EVEEffectID::subSystem)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::subSystem);
    } else if (iRef->type().HasEffect(EVEEffectID::rigSlot)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::rigSlot);
    } else {
        codelog(SHIP__ERROR, "ShipItem::FindAvailableModuleSlot() - iRef %s has no bank effect.", iRef->name());
    }

    return (EVEItemFlags)slotFound;
}


// only called from inv bound call.  can throw
void ShipItem::MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2) {
    // this will never hit.  client checks before call.
    if (!m_ModuleManager->VerifySlotExchange(slot1, slot2))
        throw CustomError ("Those locations are not compatible.");

    // test for active module(s) before moving
    GenericModule* pMod(GetModule(slot1));
    if (pMod->IsActive())
        throw CustomError ("Your %s is currently active.  You must wait for the cycle to complete before it can be removed.", pMod->GetSelf()->name());

    pMod = GetModule(slot2);
    if (pMod != nullptr)
        if (pMod->IsActive())
            throw CustomError ("Your %s is currently active.  You must wait for the cycle to complete before it can be removed.", pMod->GetSelf()->name());

    // slot1 is occupied, as this is where module is from.
    InventoryItemRef modItemRef1(GetModuleRef(slot1));
    if (modItemRef1.get() == nullptr) {
        _log(MODULE__TRACE, "ShipItem::MoveModuleSlot - modItemRef1 is null.");
        throw CustomError ("The module to move was not found.");
    }
    InventoryItemRef chargeItemRef1(m_ModuleManager->GetLoadedChargeOnModule(slot1));
    RemoveItem(modItemRef1);

    if (m_ModuleManager->IsSlotOccupied(slot2)) {
        // dropped-on slot is occupied.  proceed with moving the module currently in this slot.
        InventoryItemRef modItemRef2 = GetModuleRef(slot2);
        InventoryItemRef chargeItemRef2 = m_ModuleManager->GetLoadedChargeOnModule(slot2);
        RemoveItem(modItemRef2);
        AddItemByFlag(slot1, modItemRef2);
        if (modItemRef2->GetAttribute(AttrOnline).get_bool())
            m_ModuleManager->Online(slot1);
        if (chargeItemRef2.get() != nullptr)
            m_ModuleManager->LoadCharge(chargeItemRef2, slot1);
    }

    AddItemByFlag(slot2, modItemRef1);
    if (modItemRef1->GetAttribute(AttrOnline).get_bool())
        m_ModuleManager->Online(slot2);

    if (chargeItemRef1.get() != nullptr)
        m_ModuleManager->LoadCharge(chargeItemRef1, slot2);
}

void ShipItem::UpdateModules()
{
    /* this is only called when ship is in space
     * this will call Online() on all modules, which will apply passive and online effects.
     */
    m_ModuleManager->UpdateModules(m_onlineModuleVec);
    m_onlineModuleVec.clear();
    //FailedToOnlineModulesOnUndock
}

void ShipItem::UpdateModules(EVEItemFlags flag)
{
    // List of callees to put this function into context as to what it should be doing:
    // ShipSE::AddItem()
    // Client::MoveItem()               - something has been moved into or out of the ship, recheck all modules for... some reason
    m_ModuleManager->UpdateModules(flag);
}

//  Updated fractional ship defense settings.  -allan 1Feb15
void ShipItem::SetShipCapacitorLevel(float fraction)
{
    if (fraction > 1.0f) fraction = 1.0f;
    if (fraction < 0.0f) fraction = 0.0f;

    EvilNumber newCapacitorCharge(GetAttribute(AttrCapacitorCapacity) * fraction);
    if ((newCapacitorCharge + 0.5f) > GetAttribute(AttrCapacitorCapacity).get_float())
        newCapacitorCharge = GetAttribute(AttrCapacitorCapacity);
    if ((newCapacitorCharge - 0.5f) < 0)
        newCapacitorCharge = 0;

    _log(SHIP__MESSAGE, "SetShipCapacitorLevel() to %.1f%%.  new value is %.1f", fraction, newCapacitorCharge.get_float());
    SetAttribute(AttrCapacitorCharge, newCapacitorCharge);
}

void ShipItem::SetShipShield(float fraction)
{
    if (fraction > 1.0f) fraction = 1.0f;
    if (fraction < 0.0f) fraction = 0.0f;

    EvilNumber newShieldCharge(GetAttribute(AttrShieldCapacity) * fraction);
    if ((newShieldCharge + 0.2f) > GetAttribute(AttrShieldCapacity).get_float())
        newShieldCharge = GetAttribute(AttrShieldCapacity);
    if ((newShieldCharge - 0.2f) < 0)
        newShieldCharge = 0;

    _log(SHIP__MESSAGE, "SetShipShield() to %.1f%%.  new value is %.1f", fraction, newShieldCharge.get_float());
    SetAttribute(AttrShieldCharge, newShieldCharge);
}

void ShipItem::SetShipArmor(float fraction)
{
    fraction = 1 - fraction;

    if (fraction > 1.0f) fraction = 1.0f;
    if (fraction < 0.0f) fraction = 0.0f;

    EvilNumber newArmorDamage(GetAttribute(AttrArmorHP) * fraction);
    if ((newArmorDamage + 0.2f) > GetAttribute(AttrArmorHP).get_float())
        newArmorDamage = GetAttribute(AttrArmorHP);
    if ((newArmorDamage - 0.2f) < 0)
        newArmorDamage = 0;

    _log(SHIP__MESSAGE, "SetShipArmor() to %.1f%%.  new value is %.1f", fraction, newArmorDamage.get_float());
    SetAttribute(AttrArmorDamage, newArmorDamage);
}

void ShipItem::SetShipHull(float fraction)
{
    fraction = 1 - fraction;

    if (fraction > 1.0f) fraction = 1.0f;
    if (fraction < 0.0f) fraction = 0.0f;

    EvilNumber newHullDamage(GetAttribute(AttrHP) * fraction);
    if ((newHullDamage + 0.2f) > GetAttribute(AttrHP).get_float())
        newHullDamage = GetAttribute(AttrHP);
    if ((newHullDamage - 0.2f) < 0)
        newHullDamage = 0;

    _log(SHIP__MESSAGE, "SetShipHull() to %.1f%%.  new value is %.1f", fraction, newHullDamage.get_float());
    SetAttribute(AttrDamage, newHullDamage);
}

// not used
void ShipItem::RepairShip(float fraction)
{
    if (fraction > 1)
        fraction = 1;

    if (fraction == 1) {
         SetAttribute(AttrDamage, EvilZero);
         SetAttribute(AttrArmorDamage, EvilZero);
        return;
    }

    uint32 cHull(GetAttribute(AttrDamage).get_uint32());
    uint32 cArmor(GetAttribute(AttrArmorDamage).get_uint32());
    uint32 damage(cHull + cArmor);
    EvilNumber amount(damage * fraction);
    // this will repair hull first, then armor
    if (amount > cHull) {
        amount -= cHull;
         SetAttribute(AttrDamage, EvilZero);
        if (amount >= cArmor) {
             SetAttribute(AttrArmorDamage, EvilZero);
        } else {
            amount = cArmor - amount;
             SetAttribute(AttrArmorDamage, amount);
        }
    } else {
         SetAttribute(AttrDamage, amount);
    }
}

// not used
void ShipItem::RepairModules(std::vector<InventoryItemRef>& itemRefVec, float fraction)
{
    /** @todo  this isnt right....needs update */
    EvilNumber amount(EvilZero), damage(EvilZero);
    for (auto cur : itemRefVec) {
        damage = cur->GetAttribute(AttrDamage);
        if (damage < 0.01)
            continue;
        amount = cur->GetAttribute(AttrDamage);
        if ((amount / cur->GetAttribute(AttrHP)) > fraction) {
            amount = cur->GetAttribute(AttrHP) *  fraction;
        } else {
            amount = 1;
        }
        m_ModuleManager->RepairModule(cur->itemID(), amount);
    }
}

void ShipItem::Online(uint32 modID)
{
    if (sDataMgr.IsSolarSystem(locationID())) {
        ; // check for avalible cap, and drain accordingly (this can throw)
        /*
        float Charge = GetAttribute(AttrCapacitorCharge).get_float();
        float Capacity = GetAttribute(AttrCapacitorCapacity).get_float();
        float newCharge = 0;
        SetAttribute(AttrCapacitorCharge, newCharge);
        _log(SHIP__MESSAGE, "ShipItem::Online(): %s(%u) - New Cap Charge: %f", GetPilot()->GetName(), itemID(), newCharge);
        */
    }
    /* {'messageKey': 'EffectAlreadyActive2', 'dataID': 17876243, 'suppressable': False, 'bodyID': 256802, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3252}
     * {'messageKey': 'EffectCantHaveNegativeDuration', 'dataID': 17883801, 'suppressable': False, 'bodyID': 259632, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 847}
     * {'messageKey': 'EffectCrowdedOut', 'dataID': 17883719, 'suppressable': False, 'bodyID': 259602, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 848}
     * {'messageKey': 'EffectDeactivationCloaking', 'dataID': 17883455, 'suppressable': False, 'bodyID': 259510, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 849}
     * {'messageKey': 'EffectDeactivationWarping', 'dataID': 17883458, 'suppressable': False, 'bodyID': 259511, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 850}
     * {'messageKey': 'EffectNotActivatible', 'dataID': 17880378, 'suppressable': False, 'bodyID': 258369, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2228}
     * {'messageKey': 'EffectNotDeactivatible', 'dataID': 17883847, 'suppressable': False, 'bodyID': 259649, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 851}
     * {'messageKey': 'EffectStillActivating', 'dataID': 17883850, 'suppressable': False, 'bodyID': 259650, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 852}
     * {'messageKey': 'EffectStillActive', 'dataID': 17883853, 'suppressable': False, 'bodyID': 259651, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 853}
     */
    /*{'FullPath': u'UI/Messages', 'messageID': 256802, 'label': u'EffectAlreadyActive2Body'}(u'{modulename} is already active', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}})
     * {'FullPath': u'UI/Messages', 'messageID': 258369, 'label': u'EffectNotActivatibleBody'}(u'You do not have the ability to engage the {moduleName} in that action.', None, {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259510, 'label': u'EffectDeactivationCloakingBody'}(u'As certain activated effects interfere with the warping process, these are automatically being deactivated before the warp proceeds.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259511, 'label': u'EffectDeactivationWarpingBody'}(u'As certain activated effects interfere with the warping process, these are automatically being deactivated before the warp proceeds.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259602, 'label': u'EffectCrowdedOutBody'}(u'The {[item]module.name} cannot be activated as it requires access to resources and/or skills which {[numeric]count} modules of the same purpose are already using up.', None, {u'{[numeric]count}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'count'}, u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259632, 'label': u'EffectCantHaveNegativeDurationBody'}(u'{modulename} has somehow come to have a negative duration of {duration} and cannot operate. If you know how, obtain a breakdown of its modifiers and report it, otherwise submit a detailed bug report.', None, {u'{modulename}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'modulename'}, u'{duration}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'duration'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259649, 'label': u'EffectNotDeactivatibleBody'}(u'The {moduleName} cannot be interrupted in that action.', None, {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259650, 'label': u'EffectStillActivatingBody'}(u'The {moduleName} cannot be manually deactivated while it is still being activated.', None, {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259651, 'label': u'EffectStillActiveBody'}(u'The {[item]moduleTypeID.name} cannot be manually deactivated in the middle of an operation, it will deactivate without repeating in {[timeinterval]timeLeft.writtenForm, from=second, to=second} (its activation duration is {[timeinterval]duration.writtenForm, from=second, to=second}).', None, {u'{[timeinterval]timeLeft.writtenForm, from=second, to=second}': {'conditionalValues': [], 'variableType': 8, 'propertyName': 'writtenForm', 'args': 0, 'kwargs': {'to': 'second', 'from': 'second'}, 'variableName': 'timeLeft'}, u'{[timeinterval]duration.writtenForm, from=second, to=second}': {'conditionalValues': [], 'variableType': 8, 'propertyName': 'writtenForm', 'args': 0, 'kwargs': {'to': 'second', 'from': 'second'}, 'variableName': 'duration'}, u'{[item]moduleTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleTypeID'}})
     */
	m_ModuleManager->Online(modID);
}

void ShipItem::Offline(uint32 modID)
{
    m_ModuleManager->Offline(modID);
}

void ShipItem::Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat)
{
    if (IsValidTarget(targetID)) {
        m_targetRef = sItemFactory.GetItemRef(targetID);
    } else {
        m_targetRef = InventoryItemRef(nullptr);
    }

    m_ModuleManager->Activate( itemID, sFxDataMgr.GetEffectID(effectName), targetID, repeat);
}

void ShipItem::OnlineAll()
{
    m_ModuleManager->OnlineAll();
}

void ShipItem::OfflineAll()
{
    m_ModuleManager->OfflineAll();
}

void ShipItem::SaveShip()
{
    SaveItem();                         // Save ship info
    pAttributeMap->SaveShipState();     // save ship damage
    m_ModuleManager->SaveModules();     // Save item info for modules fitted to this ship
}

void ShipItem::RemoveRig(InventoryItemRef iRef) {
    if (iRef.get() == nullptr)
        return;
    m_ModuleManager->UninstallRig(iRef->itemID());
    iRef->Move(itemID(), flagCargoHold, true);
}

void ShipItem::ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge)
{
    _log(MODULE__ERROR, "ReplaceCharges() called by %s(%u).  It still needs to be written.", name(), itemID());
}


/*  heat-related shit......
 * see also file:///home/allan/Desktop/backups/GoonSwarm_wiki/www.eveinfo.net/wiki/ind~2149.htm - "Thermodynamics"
 *
    AttrHeatGenerationMultiplier = 1224,
    AttrHeatHi = 1175,
    AttrHeatMed = 1176,
    AttrHeatLow = 1177,

    AttrHeatDissipationRateHi = 1179,
    AttrHeatDissipationRateMed = 1196,
    AttrHeatDissipationRateLow = 1198,

    AttrHeatCapacityHi = 1178,
    AttrHeatCapacityMed = 1199,
    AttrHeatCapacityLow = 1200,

    AttrHeatAttenuationHi = 1259,
    AttrHeatAttenuationMed = 1261,
    AttrHeatAttenuationLow = 1262,

    AttrHeatAbsorbtionRateModifier = 1180,      // active modules only

    created ship attribs (rifter)
    1179    heatDissipationRateHi       NULL    0.01
    1196    heatDissipationRateMed      NULL    0.01
    1198    heatDissipationRateLow      NULL    0.01
    1178    heatCapacityHi              NULL    100
    1199    heatCapacityMed             NULL    100
    1200    heatCapacityLow             NULL    100
    1224    heatGenerationMultiplier    NULL    1
    1259    heatAttenuationHi           NULL    0.63
    1261    heatAttenuationMed          NULL    0.5
    1262    heatAttenuationLow          NULL    0.5
    created module attribs (200mm ac)
    1180    heatAbsorbtionRateModifier  NULL    0.01
    1211    heatDamage                  NULL    1
    (1mn ab I)
    1180    heatAbsorbtionRateModifier  NULL    0.04
    1211    heatDamage                  NULL    9.6
    (sebo I)
    1180    heatAbsorbtionRateModifier  NULL    0.01
    1211    heatDamage                  NULL    3.4

    // these are calculated from fx shit (not saved)    {cannot find where these are used..}
    AttrHeatAbsorbtionRateHi = 1182,        eID 10326
    AttrHeatAbsorbtionRateMed = 1183,       eID 10327
    AttrHeatAbsorbtionRateLow = 1184,       eID 10328

*/

void ShipItem::ProcessHeat()
{
    double start = GetTimeUSeconds();
    float heat(0.0f);
    // heat loop
    for (uint16 i = AttrHeatHi; i < AttrHeatLow + 1; ++i) {
        heat = GetAttribute(i).get_float();
        // the ordering here is important
        //heat -= log(-(heat + 1));
        if (heat > 1.0f)
            heat -= DissipateHeat(i, heat);
        heat += GenerateHeat(i);
        if (heat > 1.0f) {  // not concerned about anything < 1
            if (heat > 100)
                heat = 100.0f;
            SetAttribute(i, heat);
        } //else
          //  DeleteAttribute(i);
        heat = 0.0f;
    }
    _log(SHIP__HEAT, "ShipItem::ProcessHeat() Executed in %.3f us.", GetTimeUSeconds() - start);

    /*  proc times with 3 banks of 8 modules each (from 0 - 3 modules actived)
13:05:44 [ShipHeat] ShipItem::ProcessHeat() Executed in 867.000 us.
13:05:50 [ShipHeat] ShipItem::ProcessHeat() Executed in 1205.000 us.
13:05:56 [ShipHeat] ShipItem::ProcessHeat() Executed in 1051.500 us.
13:06:02 [ShipHeat] ShipItem::ProcessHeat() Executed in 1665.750 us.
13:06:08 [ShipHeat] ShipItem::ProcessHeat() Executed in 1108.500 us.
13:06:14 [ShipHeat] ShipItem::ProcessHeat() Executed in 1090.250 us.
13:06:20 [ShipHeat] ShipItem::ProcessHeat() Executed in 1011.250 us.
13:06:26 [ShipHeat] ShipItem::ProcessHeat() Executed in 938.500 us.
     */
}

float ShipItem::GenerateHeat(uint16 attrID)
{
    /**  @note  still not sure how live builds heat, but here's how im gonna do it.
     * during normal op, modules make heat that builds slowly
     *  normal module operation will not build excessive heat on ship...it was designed for it.
     * the difference here is pilots will now be able to SEE the heat buildup (dont recall if it showed on HUD w/o OL)
     * overloaded modules will build excessive heat, using a diff heat generation method,
     * and will begin to damage it's rack from overheating (heat > rack heat capy).
     */

    /*  heat buildup
     * H = 3(e^t)
     * t = (sum of active module's heat damage / 10) + 1
     *   t must be > 1.0 to avoid negatives.  if no modules active, t=1.0 and log(1.0)=0
     *   this may look funny, but is rather accurate generation of module heat from normal op.
     */

    float t(1.0f);
    std::string rack = "";
    //std::vector< GenericModule* > modVec;
    switch(attrID) {
        case AttrHeatHi: {
            rack = "Hi";
            //m_ModuleManager->GetActiveModules(EVEEffectID::hiPower, modVec);
            m_ModuleManager->GetActiveModulesHeat(EVEEffectID::hiPower, t);
        } break;
        case AttrHeatMed: {
            rack = "Mid";
            m_ModuleManager->GetActiveModulesHeat(EVEEffectID::medPower, t);
        } break;
        case AttrHeatLow: {
            rack = "Low";
            m_ModuleManager->GetActiveModulesHeat(EVEEffectID::loPower, t);
        } break;
        default: {
            _log(SHIP__HEAT, "GenerateHeat() - %s invalid rack sent (%u)", name(), attrID);
            return 0;
        } break;
    }

    if (t < 1.2)
        return 0;

    //log(t) *3;   //0.28 when t=1.1,  1.2 when t=1.5,  4.1 when t=3.9 (highest i found), 6.8 when t=5.55
    float heat = log(t) *3 * GetAttribute(AttrHeatGenerationMultiplier).get_float();

    _log(SHIP__HEAT, "%s generated %.2f heat points from the %s rack this tic.  t = %.3f", name(), heat, rack.c_str(), t);
    return heat;
}

float ShipItem::DissipateHeat(uint16 attrID, float heat)
{
    //H = ln^t
    float t(1.0f + heat), newHeat(0.0f);
    std::string rack = "";
    switch(attrID) {
        case AttrHeatHi: {
            rack = "Hi";
        } break;
        case AttrHeatMed: {
            rack = "Mid";
        } break;
        case AttrHeatLow: {
            rack = "Low";
        } break;
        default: {
            _log(SHIP__HEAT, "DissipateHeat() - %s invalid rack sent (%u)", name(), attrID);
            return 0.0f;
        } break;
    }

    newHeat = log(t);    //0.18 when t=1.2, 3.1 when t=21.9, 3.9 when t=51.9, 4.6 when t=99.9

    if (newHeat < 0)
        newHeat = 0.0f;

    _log(SHIP__HEAT, "%s dissipated %.2f heat points from the %s rack this tic.  was %.1f, is %.1f, t = %.3f", \
            name(), newHeat, rack.c_str(), heat, (heat - newHeat), t);

    return newHeat;
}

void ShipItem::DamageGroup(GenericModule* pMod)
{
    // not used yet
}

/*
 * AttrHeatDamage = 1211,
 * AttrHeatDamageBonus = 1213,          // module attrib (float x/100 = %)
 * AttrHeatGenerationMultiplier = 1224, // ship attrib
 * AttrThermodynamicsHeatDamage = 1229, // skill attrib
 * AttrHeatDamageMultiplier = 1485,     // system beacon effect
*/

/*
Slt  Att   Damage chance multiplier at distance from overheated module (in %)
            1       2       3       4       5       6       7
1   0.00    0.10
2   0.25    0.25
3   0.50    0.50    0.25
4   0.63    0.63    0.40    0.25
5   0.71    0.71    0.50    0.36    0.25
6   0.76    0.76    0.58    0.44    0.33    0.25
7   0.79    0.79    0.62    0.49    0.39    0.31    0.24
8   0.82    0.82    0.67    0.55    0.45    0.37    0.30    0.25
*/
//Cycles to burnout = total module HP/ (hp heat damage per cycle - (hp heat damage per cycle*thermodynamics level*5/100))
void ShipItem::HeatDamageCheck(GenericModule* pMod)
{
    if (pMod->IsLinked())       // linked slaves will contribute to heat calculation, but not individually.
        if (!pMod->IsMaster())  // heat is calculated by master and multiplied by #linked modules
            return;

    // check ship's current bank heat to determine chance for pMod to take heat damage.
    //  damage to other modules based on table above.
    float curHeat(0.0f), damChance(0.0f);
    if (pMod->isHighPower()) {
        curHeat = GetAttribute(AttrHeatHi).get_float();
        damChance = GetAttribute(AttrHeatAttenuationHi).get_float();
    } else if (pMod->isMediumPower()) {
        curHeat = GetAttribute(AttrHeatMed).get_float();
        damChance = GetAttribute(AttrHeatAttenuationMed).get_float();
    } else if (pMod->isLowPower()) {
        curHeat = GetAttribute(AttrHeatLow).get_float();
        damChance = GetAttribute(AttrHeatAttenuationLow).get_float();
    }

    std::vector<uint32> modVec;
    // if this module is grouped, all modules will take same damage.
    if (pMod->IsLinked()) {
        // not used yet
    } else {
        // module not linked.  continue with default heat damage calc's
        // determine position and get adjacent modules
        uint8 flag = pMod->flag();

        // determine modules to damage and add to list
        uint32 moduleID(0);

        //modVec.push_back(moduleID);
    }

    for (auto cur : modVec)
        DamageModule(cur);
}

void ShipItem::StripFitting()
{
    UnlinkAllWeapons();

    EVEItemFlags flag = flagCargoHold;
    if (sDataMgr.IsStation(locationID()))
        flag = flagHangar;

    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);
    for (auto cur : moduleList) {
        m_ModuleManager->UnfitModule(cur->itemID());
        cur->Move(locationID(), flag, true);
    }
}

void ShipItem::EmptyCargo()
{
    std::map<uint32, InventoryItemRef> invMap;
    pInventory->GetInventoryMap( invMap );
    for (auto cur : invMap )
        cur.second->Move(locationID(), flagHangar, true);
}

void ShipItem::CargoFull() {
    // loop thru modules and deactivate any type of miner
    m_ModuleManager->CargoFull();
    // tell drones cargo is full.  this will command mining types to return and orbit
    // drones->return();
}

void ShipItem::GetLinkedWeaponMods( EVEItemFlags flag, std::vector<GenericModule*> &modules ) {
    GenericModule* pMod = m_ModuleManager->GetModule(flag);
    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod);
    if (itr != m_linkedWeapons.end()) {
        modules.push_back(pMod);
        for (auto cur : itr->second)
            modules.push_back(cur);
    } else {
        // this module isnt master... loop thru all links and see if we can find it
        for (auto cur : m_linkedWeapons) {
            std::list<GenericModule*>::iterator itr2 = cur.second.begin(), end = cur.second.end();
            while (itr2 != end) {
                if ((*itr2) == pMod) {
                    GetLinkedWeaponMods(cur.first->flag(), modules);
                    return;
                }
                ++itr2;
            }
        }
    }
}

// only called from dogma call.  can throw
void ShipItem::LinkWeapon(uint32 masterID, uint32 slaveID)
{
    if (masterID == slaveID)
        return;
    GenericModule* pMod1 = m_ModuleManager->GetModule(masterID);
    GenericModule* pMod2 = m_ModuleManager->GetModule(slaveID);
    LinkWeapon(pMod1, pMod2);

    SaveWeaponGroups();
}

void ShipItem::LinkWeapon(GenericModule* pMaster, GenericModule* pSlave)
{
    if ((pMaster == nullptr) or (pSlave == nullptr))
        return; // make error here?
    if (pMaster == pSlave)
        return; // make error here?
    if ((pMaster->IsLoaded()) or (pSlave->IsLoaded()))
        throw UserError ("CantLinkAmmoInWeapon");
    if ((pMaster->IsActive()) or (pSlave->IsActive()))
        throw UserError ("CantLinkModuleActive");
    if ((pMaster->IsDamaged()) or (pSlave->IsDamaged()))
        throw UserError ("CantLinkModuleDamaged");
    if ((pMaster->IsLoading()) or (pSlave->IsLoading()))
        throw UserError ("CantLinkModuleLoading");
    if ((!pMaster->isOnline()) or (!pSlave->isOnline()))
        throw UserError ("CantLinkModuleNotOnline");

    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMaster);
    if (itr == m_linkedWeapons.end()) {
        std::list<GenericModule*> slaves;
        slaves.push_back(pSlave);
        m_linkedWeapons[pMaster] = slaves;
        pMaster->SetLinked(true);
        pMaster->SetLinkMaster(true);
    } else {
        itr->second.push_back(pSlave);
    }
    pSlave->SetLinked(true);
}

void ShipItem::MergeModuleGroups(uint32 masterID, uint32 slaveID)
{
    _log(MODULE__ERROR, "MergeModuleGroups() called by %s(%u).  It still needs to be written.", name(), itemID());
}

void ShipItem::PeelAndLink(uint32 masterID, uint32 slaveID)
{
    _log(MODULE__ERROR, "PeelAndLink() called by %s(%u).  It still needs to be written.", name(), itemID());
}

void ShipItem::LinkAllWeapons()
{
    std::list< GenericModule* > weaponList;
    m_ModuleManager->GetWeapons(weaponList);

    // remove current links
    for (auto cur : weaponList) {
        if (sConfig.server.UnloadOnLinkAll)
            m_ModuleManager->UnloadCharge(cur);
        cur->SetLinked(false);
        cur->SetLinkMaster(false);
    }
    m_linkedWeapons.clear();

    LinkWeaponLoop(weaponList);

    // remove empty masters from map
    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.begin();
    while (itr != m_linkedWeapons.end()) {
        if (itr->second.empty()) {
            if (is_log_enabled(MODULE__INFO))
                _log(MODULE__INFO, "ShipItem::LinkAllWeapons() - %s(%s) has empty link list.  Removing.", \
                    itr->first->GetSelf()->name(), sDataMgr.GetFlagName(itr->first->flag()));
            itr = m_linkedWeapons.erase(itr);
        } else {
            ++itr;
        }
    }

    SaveWeaponGroups();
}

void ShipItem::LinkWeaponLoop(std::list<GenericModule*>& weaponList)
{
    double start = GetTimeUSeconds();
    GenericModule* master(nullptr);
    std::list< GenericModule*>::iterator itr = weaponList.begin();
    while (itr != weaponList.end()) {
        if ((*itr)->IsLoaded() or (*itr)->IsLoading()) {
            if (is_log_enabled(MODULE__INFO))
                _log(MODULE__INFO, "ShipItem::LinkWeaponLoop() - %s(%s-%u) IsLoaded.  Skipping.", \
                    (*itr)->GetSelf()->name(), sDataMgr.GetFlagName((*itr)->flag()), (*itr)->itemID());
            m_pilot->SendErrorMsg("You cannot group the %s while loaded with %s", (*itr)->GetSelf()->name(), (*itr)->GetLoadedChargeRef()->name());
            itr = weaponList.erase(itr);
        } else if (master == nullptr) {
            // lets check if this module will match a master already in list before making new master...
            bool match(false);
            for (auto item : m_linkedWeapons)
                if (item.first->typeID() == (*itr)->typeID()) {
                    //master = item.first;
                    if (is_log_enabled(MODULE__INFO))
                        _log(MODULE__INFO, "ShipItem::LinkWeaponLoop() -(null master) %s(%s-%u) matches list master %s(%s-%u).  Adding.", \
                            (*itr)->GetSelf()->name(), sDataMgr.GetFlagName((*itr)->flag()), (*itr)->itemID(), \
                            item.first->GetSelf()->name(), sDataMgr.GetFlagName(item.first->flag()), item.first->itemID());
                    LinkWeapon(item.first, (*itr));
                    itr = weaponList.erase(itr);
                    match = true;
                    break;
                }
            if (match)
                continue;
            // didnt match, or list empty.  make new master.
            master = (*itr);
            if (is_log_enabled(MODULE__INFO))
                _log(MODULE__INFO, "ShipItem::LinkWeaponLoop() - Setting %s(%s-%u) to master.",\
                    (*itr)->GetSelf()->name(), sDataMgr.GetFlagName((*itr)->flag()), (*itr)->itemID());
            // set blank list for this master.
            std::list<GenericModule*> slaves;
            m_linkedWeapons[master] = slaves;
            itr = weaponList.erase(itr);
        } else {
            if (master->typeID() == (*itr)->typeID()) {    // this item can be slave
                if (is_log_enabled(MODULE__INFO))
                    _log(MODULE__INFO, "ShipItem::LinkWeaponLoop() - %s(%s-%u) matches master %s(%s-%u).  Adding.", \
                        (*itr)->GetSelf()->name(), sDataMgr.GetFlagName((*itr)->flag()), (*itr)->itemID(), \
                        master->GetSelf()->name(), sDataMgr.GetFlagName(master->flag()), master->itemID());
                LinkWeapon(master, (*itr));
                itr = weaponList.erase(itr);
            } else {
                for (auto item : m_linkedWeapons)
                    if (item.first->typeID() == (*itr)->typeID()) {
                        //master = item.first;
                        if (is_log_enabled(MODULE__INFO))
                            _log(MODULE__INFO, "ShipItem::LinkWeaponLoop() - %s(%s-%u) matches list master %s(%s-%u).  Adding.", \
                                (*itr)->GetSelf()->name(), sDataMgr.GetFlagName((*itr)->flag()), (*itr)->itemID(), \
                                item.first->GetSelf()->name(), sDataMgr.GetFlagName(item.first->flag()), item.first->itemID());
                        LinkWeapon(item.first, (*itr));
                        itr = weaponList.erase(itr);
                        break;
                    }
                master = nullptr;
            }
        }
    }
    if (is_log_enabled(MODULE__INFO))
        _log(MODULE__INFO, "ShipItem::LinkWeaponLoop() - Completed loop in %.3fus.", GetTimeUSeconds() - start);
}

/*
 * {'messageKey': 'CantUngroupModuleActive', 'dataID': 17874986, 'suppressable': False, 'bodyID': 256339, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3540}
 *    (256339, `You cannot ungroup active modules, please deactivate them first.`)
 * {'messageKey': 'CantUngroupModuleLoading', 'dataID': 17874989, 'suppressable': False, 'bodyID': 256340, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3541}
 *    (256340, `You can't ungroup weapons while they are being loaded with charges.`)
 * {'messageKey': 'CantUnlinkModuleActive', 'dataID': 17878129, 'suppressable': False, 'bodyID': 257511, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2679}
 *    (257511, `You cannot ungroup active modules, please deactivate them first.`)
 */
uint32 ShipItem::UnlinkWeapon(uint32 moduleID)
{
    GenericModule* pMod1(m_ModuleManager->GetModule(moduleID));
    if (pMod1 == nullptr)
        return 0; // make error here?

    if (pMod1->IsActive()) {
        m_pilot->SendNotifyMsg("You cannot ungroup active modules, please deactivate them first.");
        return 0;
    }
    if (pMod1->IsLoading()) {
        m_pilot->SendNotifyMsg("You cannot ungroup weapons while they are being loaded with charges.");
        return 0;
    }
    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod1);
    if (itr == m_linkedWeapons.end())
        return 0;

    // weird shit here, but this works...

    // get first linked moduleID
    uint32 slaveID(itr->second.front()->itemID());
    // delete group
    UnlinkGroup(moduleID);
    LinkWeapon(moduleID, slaveID);
    // make packet for master and first slave (slaveID)
    PyList* slaves = new PyList();
        slaves->AddItem(new PyInt(slaveID));
    PyDict* result = new PyDict();
        result->SetItem(new PyInt(moduleID), slaves);
    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyString("OnWeaponBanksChanged"));
        tuple->SetItem(1, new PyInt(m_itemID));
        tuple->SetItem(2, result);      //GetLinkedWeapons()
    // send 'new' group data to client
    m_pilot->QueueDestinyEvent(&tuple);
    // send immediately (otherwise it will wait until after this call returns, which negates this hack
    //m_pilot->FlushQueue();

    // return slaveID, which client will use to delete it's map and continue processing
    return slaveID;
}

// only called from dogma call.  can throw
void ShipItem::UnlinkWeapon(uint32 masterID, uint32 slaveID)
{
    if (masterID == slaveID)
        return;
    GenericModule* pMod1(m_ModuleManager->GetModule(masterID));
    GenericModule* pMod2(m_ModuleManager->GetModule(slaveID));
    if ((pMod1 == nullptr) or (pMod2 == nullptr))
        return; // make error here?

    // if master is loading or active, then whole group is
    if (pMod1->IsActive())
        throw UserError ("CantUngroupModuleActive");
    if (pMod1->IsLoading())
        throw UserError ("CantUngroupModuleLoading");

    pMod2->SetLinked(false);

    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod1);
    if (itr != m_linkedWeapons.end()) {
        std::list<GenericModule*>::iterator itr2 = itr->second.begin();
        while (itr2 != itr->second.end()) {
            if ((*itr2) == pMod2) {
                itr2 = itr->second.erase(itr2);
                if (itr->second.empty()) {
                    pMod1->SetLinked(false);
                    pMod1->SetLinkMaster(false);
                    m_linkedWeapons.erase(itr);
                }
                return;
            }
            ++itr2;
        }
    }
}

// only called from dogma call.  can throw
void ShipItem::UnlinkGroup(uint32 memberID, bool update/*false*/)
{
    GenericModule* pMod1(m_ModuleManager->GetModule(memberID));
    if (pMod1 == nullptr)
        return; // make error here?

    // if master is loading or active, then whole group is
    if (pMod1->IsActive())
        throw UserError ("CantUngroupModuleActive");
    if (pMod1->IsLoading())
        throw UserError ("CantUngroupModuleLoading");

    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod1);
    if (itr != m_linkedWeapons.end()) {
        pMod1->SetLinked(false);
        pMod1->SetLinkMaster(false);
        std::list<GenericModule*>::iterator itr2 = itr->second.begin();
        while (itr2 != itr->second.end()) {
            (*itr2)->SetLinked(false);
            itr2 = itr->second.erase(itr2);
            if (itr->second.empty()) {
                m_linkedWeapons.erase(itr);
                SaveWeaponGroups();
                if (update) {
                    PyTuple* tuple = new PyTuple(3);
                        tuple->SetItem(0, new PyString("OnWeaponGroupDestroyed"));
                        tuple->SetItem(1, new PyInt(m_itemID));
                        tuple->SetItem(2, new PyInt(memberID));
                    m_pilot->QueueDestinyEvent(&tuple);
                }
                return;
            }
        }
    } else {
        // this module isnt master... loop thru all links to see if we can find it
        for (auto cur : m_linkedWeapons) {
            std::list<GenericModule*>::iterator itr2 = cur.second.begin();
            while (itr2 != cur.second.end()) {
                if ((*itr2) == pMod1) {
                    UnlinkGroup(cur.first->itemID(), update);
                    return;
                }
                ++itr2;
            }
        }
    }
}

// called from here and dogma call.  can throw
void ShipItem::UnlinkAllWeapons()
{
    std::list< GenericModule* > weaponList;
    m_ModuleManager->GetWeapons(weaponList);
    for (auto cur : weaponList) {
        if (cur->IsActive())
            throw UserError ("CantUngroupModuleActive");
        if (cur->IsLoading())
            throw UserError ("CantUngroupModuleLoading");

        cur->SetLinked(false);
        cur->SetLinkMaster(false);
    }
    m_linkedWeapons.clear();
    ShipDB::ClearWeaponGroups(m_itemID);
}

uint8 ShipItem::GetLinkedCount(GenericModule* pMod)
{
    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod);
    if (itr == m_linkedWeapons.end())
        return 1;
    return itr->second.size() + 1;
}

uint8 ShipItem::GetLoadedLinkedCount(GenericModule* pMod)
{
    uint8 count(1);
    std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod);
    if (itr != m_linkedWeapons.end()) {
        std::list<GenericModule*>::iterator itr2 = itr->second.begin(), end = itr->second.end();
        while (itr2 != end) {
            if ((*itr2)->IsLoaded())
                ++count;
            ++itr2;
        }
    }
    return count;
}

PyRep* ShipItem::GetLinkedWeapons()
{
    if (m_linkedWeapons.empty())
        return PyStatic.NewNone();

    PyDict* result = new PyDict();
    for (auto cur : m_linkedWeapons) {
        PyList* slaves = new PyList();
        for (auto slave : cur.second)
            slaves->AddItem(new PyInt(slave->itemID()));
        result->SetItem(new PyInt(cur.first->itemID()), slaves);
    }

    if (is_log_enabled(MODULE__MESSAGE)) {
        _log(MODULE__MESSAGE, "GetLinkedWeapons()");
        result->Dump(MODULE__MESSAGE, "    ");
    }
    return result;
}

void ShipItem::OfflineGroup(GenericModule* pMod)
{
    if (pMod->IsMaster()) {
        std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMod);
        if (itr != m_linkedWeapons.end())
            for (auto cur : itr->second)
                cur->Offline();
    } else {
        // this module isnt master... loop thru all links to see if we can find it
        for (auto cur : m_linkedWeapons) {
            std::list<GenericModule*>::iterator itr = cur.second.begin();
            while (itr != cur.second.end()) {
                if ((*itr) == pMod) {
                    OfflineGroup(cur.first);
                    return;
                }
                ++itr;
            }
        }
    }
}

void ShipItem::SaveWeaponGroups()
{
    std::multimap< uint32, uint32 > data;
    data.clear();
    if (!m_linkedWeapons.empty())
        for (auto cur : m_linkedWeapons)
            for (auto slave : cur.second)
                data.emplace(cur.first->itemID(), slave->itemID());

    ShipDB::SaveWeaponGroups(m_itemID, data);
}

void ShipItem::LoadWeaponGroups()
{
    // NOTE:  there probably isnt a pilot at this point, so no sending of errors on load.
    if (m_ModuleManager == nullptr) {
        if (m_pilot != nullptr)
            m_pilot->SendErrorMsg("There was an error loading your weapon groups.  Ref: ServerError 06071");
        return;
    }
    DBQueryResult* res = new DBQueryResult();
    ShipDB::LoadWeaponGroups(m_itemID, *res);

    bool error(false);
    GenericModule* pMaster(nullptr);
    GenericModule* pSlave(nullptr);
    DBResultRow row;
    // this is gonna be slow as shit...i dont like this, but best way to do it.
    while (res->GetRow(row)) {
        //SELECT masterID, slaveID FROM shipWeaponGroups
        pMaster = m_ModuleManager->GetModule(row.GetInt(0));
        pSlave = m_ModuleManager->GetModule(row.GetInt(1));
        if ((pMaster == nullptr) or (pSlave == nullptr)) {
            error = true;
            continue;
        }
        //LinkWeapon(pMaster, pSlave);  <<<--- this will throw.  DO NOT use here.
        std::map<GenericModule*, std::list<GenericModule*>>::iterator itr = m_linkedWeapons.find(pMaster);
        if (itr == m_linkedWeapons.end()) {
            std::list<GenericModule*> slaves;
            slaves.push_back(pSlave);
            m_linkedWeapons[pMaster] = slaves;
        } else {
            itr->second.push_back(pSlave);
        }
        pMaster->SetLinked(true);
        pMaster->SetLinkMaster(true);
        pSlave->SetLinked(true);
    }

    if (error) {
        UnlinkAllWeapons();
        if (m_pilot != nullptr)
            m_pilot->SendErrorMsg("There was an error loading a weapon group master.  Ref: ServerError 06123");
    }

    SafeDelete(res);
}
/* End new Module Manager Interface */

// new effects system.  wip
void ShipItem::ProcessEffects(bool add/*false*/, bool update/*false*/)
{
    double startTime(GetTimeMSeconds());
    _log(EFFECTS__TRACE, "ShipItem::ProcessEffects()");
    /*
    Effects processing order...
        Skills     //char effect
        Implants   //char effect
        Boosters   //char effect
        Ship       //ship effect
        Subsystem  //module effect
        Rigs       //module effect
        Low        //module effect
        Mid        //module effect
        Hi         //module effect
    */
    if (add) {
        m_pilot->GetChar()->ProcessEffects(this);
        ProcessShipEffects(update);
    } else {
        ClearModifiers();
        pAttributeMap->SaveShipState();      // save ship damage as it's removed on next call
        ResetAttributes();
        ClearModuleModifiers();
        m_pilot->GetChar()->ResetModifiers();
        std::vector< InventoryItemRef > modVec;
        m_ModuleManager->GetModuleListOfRefsAsc(modVec);
        for (auto cur : modVec)
            cur->ResetAttributes();
        std::map<EVEItemFlags, InventoryItemRef> charges;
        m_ModuleManager->GetLoadedCharges(charges);
        for (auto cur : charges)
            cur.second->ResetAttributes();

        // do we remove fx here?  nah, we've reset everything at this point.
    }

    _log(EFFECTS__DEBUG, "ShipItem::ProcessEffects() - effects processed and applied in %.3fms", (GetTimeMSeconds() - startTime));
}

void ShipItem::ProcessShipEffects(bool update/*false*/)
{
    _log(EFFECTS__TRACE, "ShipItem::ProcessShipEffects()");
    for (auto it : type().m_stateFxMap) {
        fxData data = fxData();
        data.action = FX::Action::Invalid;
        data.srcRef = static_cast<InventoryItemRef>(this);
        sFxProc.ParseExpression(this, sFxDataMgr.GetExpression(it.second.preExpression), data);
    }

    if (m_isUndocking) {
        // online modules sent from client (these are onlined in fit window while docked)
        m_ModuleManager->UpdateModules(m_onlineModuleVec);
        m_onlineModuleVec.clear();
        //FailedToOnlineModulesOnUndock
    } else {
        // this will set module to last saved online state in the case of BoardShip() and Login()
        m_ModuleManager->LoadOnline();
    }

    // apply processed effects
    sFxProc.ApplyEffects(this, m_pilot->GetChar().get(), this, update);
    //ClearModifiers();
}

void ShipItem::ClearModuleModifiers()
{
    _log(EFFECTS__TRACE, "ShipItem::ClearModuleModifiers()");
    //m_ModuleManager->OfflineAll();
    std::vector< InventoryItemRef > modVec;
    m_ModuleManager->GetModuleListOfRefsAsc(modVec);
    for (auto cur : modVec)
        cur->ClearModifiers();
    std::map<EVEItemFlags, InventoryItemRef> charges;
    m_ModuleManager->GetLoadedCharges(charges);
    for (auto cur : charges)
        cur.second->ClearModifiers();
}

void ShipItem::ResetEffects() {
    _log(EFFECTS__TRACE, "ShipItem::ResetEffects()");
    double start = GetTimeMSeconds();

    m_ModuleManager->OfflineAll();

    // reset attributes on char, ship, all modules and charges
    pAttributeMap->SaveShipState();      // save ship damage as it's removed on next call
    ResetAttributes();
    m_pilot->GetChar()->ResetModifiers();
    std::vector< InventoryItemRef > modVec;
    m_ModuleManager->GetModuleListOfRefsAsc(modVec);
    for (auto cur : modVec)
        cur->ResetAttributes();
    std::map<EVEItemFlags, InventoryItemRef> charges;
    m_ModuleManager->GetLoadedCharges(charges);
    for (auto cur : charges)
        cur.second->ResetAttributes();

    ProcessEffects(true, true/*sDataMgr.IsSolarSystem(locationID())*/);
    _log(EFFECTS__DEBUG, "ShipItem::ResetEffects() - Effects reset in %.3fms", (GetTimeMSeconds() - start));
}

void ShipItem::PrepForUndock() {
    //  "Offline" modules (see notes in GM::Online and GM::Offline
    m_ModuleManager->OfflineAll();
    // clear fx maps before undock
    ProcessEffects();
}

std::string ShipItem::GetShipDNA()
{
    /* ship dna is shorthand notation to describe a ship and it's fittings purely thru the use of typeIDs and quantities
     *
     * the format is as follows:
     * <shipTypeID>:
     *    <subsystemID>:
     *      <moduleType-Highslot>;<quantity>:
     *      <moduleType-Midslot>;<quantity>:
     *      <moduleType-Lowslot>;<quantity>:
     *      <moduleType-Rigslot>;<quantity>:
     *      <chargeType>;<quantity>:
     *      <droneType>;<quantity>
     *
     * Condensed version:
     *  Ship:Subsystem:Highs:Mids:Lows:Rigs:Charges:Drones
     *
     * [PyString "<url=fitting:24698:3841;2:2531;1:19812;1:23527;1:2410;7:1422;4:2547;1:31802;3:2301;1:2454;5::>Anchor</url>"]
     *
     *  current code returns this:
     * "587:8863;1:8863;1:8863;1:499;1:578;1:1798;1:6485;1:2046;1:8325;1:31788;1:31800;1:31788;1::"
     *  need to figure out how to group modules for correct condensed counts
     */
    if (typeID() == EVEDB::invTypes::Capsule) {
        std::stringstream dna;
        dna << typeID() << ":";
        _log(SHIP__MESSAGE, "ShipDNA has compiled DNA of \"%s\" for %s(%u) ", dna.str().c_str(), name(), itemID());
        return dna.str();
    }

    /* find and encode the module typeIDs */
    std::stringstream modHi, modMid, modLow, subSys, modRig, charges, drones;

    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);

    /** @todo update this for multiples of modules */
    for (auto cur : moduleList) {
        if (IsRigSlot(cur->flag())) {
            modRig << cur->typeID() << ";" << cur->quantity() << ":";
        } else if (IsHiSlot(cur->flag())) {
            modHi << cur->typeID() << ";" << cur->quantity() << ":";
        } else if (IsMidSlot(cur->flag())) {
            modMid << cur->typeID() << ";" << cur->quantity() << ":";
        } else if (IsLowSlot(cur->flag())) {
            modLow << cur->typeID() << ";" << cur->quantity() << ":";
        } else if (IsSubSystem(cur->flag())) {
            subSys << cur->typeID() << ":";
        } else {
            ; // error?
        }
    }

    std::map<EVEItemFlags, InventoryItemRef> chargeList;
    m_ModuleManager->GetLoadedCharges(chargeList);
    for (auto cur : chargeList)
        charges << cur.second->typeID() << ";" << cur.second->quantity() << ":";

    /* not sure how to get drones yet.  will work on later */
    drones << ":";

    /* build the dna stream */
    std::stringstream dna;
    dna << typeID() << ":";
    dna << subSys.str() << modHi.str() << modMid.str() << modLow.str() << modRig.str() << charges.str() << drones.str();

    _log(SHIP__MESSAGE, "ShipDNA has compiled \"%s\" for %s(%u) ", dna.str().c_str(), name(), itemID());
    return dna.str();
}

void ShipItem::VerifyHoldType(EVEItemFlags flag, InventoryItemRef iRef, Client* pClient/*nullptr*/)
{
    // if *this ship isnt active, it wont have a pilot to send errors to.  test and set as needed.
    //  to fix this, set client to character calling the move
    if (pClient == nullptr) {
        if (m_pilot == nullptr)
            return;     /** @todo  this is an error  */
        pClient = m_pilot;
    }

    switch (flag) {
        case flagCargoHold:
            // everything can be stored in general cargo
            //  update this later to disallow specialized items?
            return;
        case flagDroneBay: {
            if (iRef->categoryID() != EVEDB::invCategories::Drone) {
                throw CustomError ("%s cannot be stowed in the Drone Bay", sDataMgr.GetGroupName(iRef->groupID()));
            }
            if (groupID() == EVEDB::invGroups::Supercarrier) {
                // these can only carry fighters and fighter/bombers in drone bay.  enforce that here.
                if ((iRef->groupID() != EVEDB::invGroups::Fighter_Bomber)
                and (iRef->groupID() != EVEDB::invGroups::Fighter_Drone)) {
                    throw CustomError ("The %s can only carry fighter drones in it's Drone Bay.  The %s is not allowed.", name(), iRef->name());
                }
            }
        } break;
        case flagShipHangar: {    //AttrShipMaintenanceBayCapacity
            if (!HasAttribute(AttrHasShipMaintenanceBay)) {
                throw CustomError ("Your %s has no ship maintenance bay.", name());
            }
            if (typeID() == EVEDB::invTypes::Rorqual)
                if ((iRef->groupID() != EVEDB::invGroups::MiningBarge)
                and (iRef->groupID() != EVEDB::invGroups::Exhumer)
                and (iRef->groupID() != EVEDB::invGroups::Industrial)
                and (iRef->groupID() != EVEDB::invGroups::TransportShip)
                and (iRef->groupID() != EVEDB::invGroups::Freighter)
                and (iRef->groupID() != EVEDB::invGroups::Prototype_Exploration_Ship)
                and (iRef->groupID() != EVEDB::invGroups::CapitalIndustrialShip)) {
                    throw CustomError ("Only indy ships may be placed into the Rorqual's ship hold.");
                }
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                throw CustomError ("Only ships may be placed into the maintenance bay.");
            }
        } break;
        case flagFuelBay: {    //  AttrFuelBayCapacity
            if ((iRef->groupID() != EVEDB::invGroups::FuelBlock)
            and (iRef->groupID() != EVEDB::invGroups::Ice_Product)) {
                throw CustomError ("Only fuel types may be stored in the fuel bay.");
            }
        } break;
        case flagOreHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Asteroid) {
                throw CustomError ("Only mined ore may be stored in the ore hold.");
            }
        } break;
        case flagGasHold: {
            if (iRef->groupID() != EVEDB::invGroups::Gas_Isotopes) {
                throw CustomError ("Only gas products may be stored in the gas hold.");
            }
        } break;
        case flagMineralHold: {
            if (iRef->groupID() != EVEDB::invGroups::Mineral) {
                throw CustomError ("Only refined minerals may be placed into the mineral hold.");
            }
        } break;
        case flagSalvageHold: {
            if (iRef->groupID() != EVEDB::invGroups::Salvage_Materials) {
                throw CustomError ("Only salvaged materials may be placed into the salvage bay.");
            }
        } break;
        // not sure if all of these flag* are used.  if not, *may* update dgmData to add them....later.
        case flagShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                throw CustomError ("Only ships may be placed into the ship hold.");
            }
        } break;

        /** @todo need to figure out how to separate ships into s/m/l/i for these.... */
        case flagSmallShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                throw CustomError ("Only small ships may be placed into the ship's small ship hold.");
            }
        } break;
        case flagMediumShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                throw CustomError ("Only medium ships may be placed into the ship's medium ship hold.");
            }
        } break;
        case flagLargeShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                throw CustomError ("Only large ships may be placed into the ship's large ship hold.");
            }
        } break;
        case flagIndustrialShipHold: {
            if ((iRef->groupID() != EVEDB::invGroups::MiningBarge)
            and (iRef->groupID() != EVEDB::invGroups::Exhumer)
            and (iRef->groupID() != EVEDB::invGroups::Industrial)
            and (iRef->groupID() != EVEDB::invGroups::TransportShip)
            and (iRef->groupID() != EVEDB::invGroups::Freighter)
            and (iRef->groupID() != EVEDB::invGroups::Prototype_Exploration_Ship)
            and (iRef->groupID() != EVEDB::invGroups::CapitalIndustrialShip)) {
                throw CustomError ("Only indy ships may be placed into the ship's industrial ship hold.");
            }
        } break;
        case flagAmmoHold: {
            if ((iRef->groupID() != EVEDB::invGroups::Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Frequency_Crystal)
            and (iRef->groupID() != EVEDB::invGroups::Mining_Crystal)
            and (iRef->groupID() != EVEDB::invGroups::Mercoxit_Mining_Crystal)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Beam_Laser_Crystal)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Pulse_Laser_Crystal)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Artillery_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Autocannon_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Blaster_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Railgun_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Hybrid_Ammo)) {
                throw CustomError ("Only ammunition and crystals may be placed into the ammo bay.");
                }
        } break;
        case flagHangar:
        case flagCorpHangar2:
        case flagCorpHangar3:
        case flagCorpHangar4:
        case flagCorpHangar5:
        case flagCorpHangar6:
        case flagCorpHangar7: {    //AttrCorporateHangarCapacity
            if (GetAttribute(AttrHasCorporateHangars) == 0) {
                throw CustomError ("Your %s has no corporate hangars.", name());
            }
        } break;
        default: {
            // {'FullPath': u'UI/Messages', 'messageID': 259450, 'label': u'ItemNotHardwareBody'}(u'{[item]itemname.name} cannot be fitted onto a ship. Only hardware modules can be fitted.', None, {u'{[item]itemname.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'itemname'}})

            if ((iRef->categoryID() != EVEDB::invCategories::Module)
            and (iRef->categoryID() != EVEDB::invCategories::Charge)
            and (iRef->categoryID() != EVEDB::invCategories::Subsystem)) {
                throw CustomError ("%s cannot be fitted onto a ship. Only hardware modules may be fitted.", iRef->name());
            }

            if (IsModuleSlot(flag)) {
                if (!Skill::FitModuleSkillCheck(iRef, pClient->GetChar())) {
                    throw CustomError ("You do not have the required skills to fit this %s.  Ref: ServerError 25163.", iRef->name());
                }
                if (!ValidateItemSpecifics(iRef)) {
                    throw CustomError ("Your ship cannot equip the %s.<br>The group '%s' is not allowed on your %s.", \
                            iRef->name(), sDataMgr.GetGroupName(iRef->groupID()), type().name().c_str());
                }
                if (iRef->categoryID() == EVEDB::invCategories::Charge) {
                    GenericModule* pMod(m_ModuleManager->GetModule(flag));
                    if (pMod != nullptr) {
                        // note:  this is also checked in client before calling Load()
                        if (iRef->HasAttribute(AttrChargeSize))
                            if (pMod->GetAttribute(AttrChargeSize) != iRef->GetAttribute(AttrChargeSize)) {
                                sLog.Error("ShipItem::VerifyHoldType", "Charge size %u for %s does not match Module size %u for %s.",\
                                        iRef->GetAttribute(AttrChargeSize).get_uint32(), iRef->name(),\
                                        pMod->GetAttribute(AttrChargeSize).get_uint32(), pMod->GetSelf()->name());
                                throw CustomError ("Incorrect charge size for this module.");
                            }
                            if ((pMod->GetAttribute(AttrChargeGroup1) != iRef->groupID())
                            and (pMod->GetAttribute(AttrChargeGroup2) != iRef->groupID())
                            and (pMod->GetAttribute(AttrChargeGroup3) != iRef->groupID())
                            and (pMod->GetAttribute(AttrChargeGroup4) != iRef->groupID())
                            and (pMod->GetAttribute(AttrChargeGroup5) != iRef->groupID())) {
                                throw CustomError ("Incorrect charge type for this module.");
                            }
                        // NOTE: Module Manager will check for actual room to load charges and make stack splits, or reject loading altogether
                    } else {
                        throw CustomError ("There is no module in %s.  Ref: ServerError 25162.", sDataMgr.GetFlagName(flag));
                    }
                }
            } else {
                sLog.Error("ShipItem::VerifyHoldType", "testing %s to add %u %s of cat %s has reached the end.",
                        sDataMgr.GetFlagName(flag), iRef->quantity(), iRef->name(), sDataMgr.GetCategoryName(iRef->categoryID()));
                throw CustomError ("Internal Server Error.  Ref: ServerError 25162.");
            }
        }
    }
}

// this one is called from GetAllInfo
PyDict* ShipItem::GetShipInfo()
{
    if (!pInventory->LoadContents())  {
        _log( INV__ERROR, "%s(%u): Failed to load contents for ShipGetInfo", name(), itemID());
        return nullptr;
    }

    //first populate the ship...
    Rsp_CommonGetInfo_Entry entry;
    if (!Populate( entry))
        return nullptr;

    PyDict* result = new PyDict();
    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));

    //get modules and charges
    std::vector<InventoryItemRef> equipped;
    pInventory->GetItemsByFlagRange(flagLowSlot0, flagHiSlot7, equipped);
    pInventory->GetItemsByFlagRange(flagRigSlot0, flagRigSlot2, equipped);
    pInventory->GetItemsByFlagRange(flagSubSystem0, flagSubSystem4, equipped);
    //encode each one...
    for (auto cur : equipped) {
        Rsp_CommonGetInfo_Entry entry2;
        if (cur->Populate(entry2)) {
            if (cur->categoryID() == EVEDB::invCategories::Charge) {
                PyTuple* tuple = new PyTuple(3);
                    tuple->SetItem(0, new PyInt(cur->locationID()));
                    tuple->SetItem(1, new PyInt(cur->flag()));
                    tuple->SetItem(2, new PyInt(cur->typeID()));
                result->SetItem(tuple, new PyObject("util.KeyVal", entry2.Encode()));
            } else {
                result->SetItem(new PyInt(cur->itemID()), new PyObject("util.KeyVal", entry2.Encode()));
            }
        } else {
            _log( SHIP__ERROR, "%s(%u): Failed to Populate() %s(%u) for ShipGetInfo", \
                        name(), itemID(), cur->name(), cur->itemID());
        }
    }

    if (is_log_enabled(SHIP__INFO)) {
        _log(SHIP__INFO, "ShipItem::GetShipInfo() decoded:");
        result->Dump(SHIP__INFO, "    ");
    }
    return result;
}

PyDict* ShipItem::GetShipState() {
    if (!pInventory->ContentsLoaded()) {
        if (!pInventory->LoadContents()) {
            _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", name(), itemID());
            return nullptr;
        }
    }
    // Create new dictionary for shipState:
    PyDict *result = new PyDict();
    // Create entry for ShipItem itself:
    result->SetItem(new PyInt(itemID()), GetItemStatusRow());
    // Check for and Create entry for pilot:
    InventoryItemRef iRefPilot(nullptr);
    if (pInventory->GetSingleItemByFlag(flagPilot, iRefPilot))
        result->SetItem(new PyInt(iRefPilot->itemID()), iRefPilot->GetItemStatusRow());

    // Create entries for ALL modules, rigs, and subsystems present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);
    for (auto cur : moduleList)
        result->SetItem(new PyInt(cur->itemID()), cur->GetItemStatusRow());

    return result;
}

PyDict* ShipItem::GetChargeState() {
    if (!pInventory->ContentsLoaded()) {
        if (!pInventory->LoadContents()) {
            _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", name(), itemID());
            return nullptr;
        }
    }

    /* get list of all charges loaded in modules */
    std::map< EVEItemFlags, InventoryItemRef > charges;
    m_ModuleManager->GetLoadedCharges(charges);

    PyDict *result = new PyDict();
    if (charges.empty())
        return result;

    // Create entries in "shipState" dictionary for loaded charges on ship:
    for (auto cur : charges)
        result->SetItem(new PyInt((uint16)cur.first), cur.second->GetChargeStatusRow(itemID()));

    return result;
}

PyList* ShipItem::ShipGetModuleList() {
    if (!pInventory->LoadContents()) {
        _log(INV__ERROR, "%s(%u): Failed to load contents for ShipGetModuleList", name(), itemID());
        return nullptr;
    }

    PyList* result = new PyList();
    // Create entries in "onslimitemchange" modules list for ALL modules, rigs, and subsystems present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);
    for (auto cur : moduleList) {
        PyTuple* module = new PyTuple(2);
        module->SetItem(0, new PyInt(cur->typeID()));
        module->SetItem(1, new PyInt(cur->itemID()));
        result->AddItem(module);
    }

    return result;
}

bool ShipItem::ValidateBoardShip(CharacterRef character) {
    bool result = false;
    EvilNumber skillTypeID(EvilZero);

    if (HasAttribute(AttrRequiredSkill1, skillTypeID)) {
        if (character->HasSkillTrainedToLevel( skillTypeID.get_uint32(), GetAttribute(AttrRequiredSkill1Level).get_uint32()))
            result = true;
        if (HasAttribute(AttrRequiredSkill2, skillTypeID)) {
            if (character->HasSkillTrainedToLevel( skillTypeID.get_uint32(), GetAttribute(AttrRequiredSkill2Level).get_uint32())) {
                result = true;
            } else {
                return false;
            }
            if (HasAttribute(AttrRequiredSkill3, skillTypeID)) {
                if (character->HasSkillTrainedToLevel( skillTypeID.get_uint32(), GetAttribute(AttrRequiredSkill3Level).get_uint32())) {
                    result = true;
                } else {
                    return false;
                }
                if (HasAttribute(AttrRequiredSkill4, skillTypeID)) {
                    if (character->HasSkillTrainedToLevel( skillTypeID.get_uint32(), GetAttribute(AttrRequiredSkill4Level).get_uint32())) {
                        result = true;
                    } else {
                        return false;
                    }
                    if (HasAttribute(AttrRequiredSkill5, skillTypeID)) {
                        if (character->HasSkillTrainedToLevel( skillTypeID.get_uint32(), GetAttribute(AttrRequiredSkill5Level).get_uint32())) {
                            result = true;
                        } else {
                            return false;
                        }
                        if (HasAttribute(AttrRequiredSkill6, skillTypeID)) {
                            if (character->HasSkillTrainedToLevel( skillTypeID.get_uint32(), GetAttribute(AttrRequiredSkill6Level).get_uint32())) {
                                result = true;
                            } else {
                                return false;
                            }
                        }
                    }
                }
            }
        }
    } else {
        result = true;
    }

    return result;
}

bool ShipItem::ValidateItemSpecifics(InventoryItemRef iRef)
{
    bool result(false);
    EvilNumber fitID(0);
    uint16 groupID(type().groupID());
    // If a ship group restriction is specified, the item must be able to fit to at least one ship group.
    _log(SHIP__TRACE, "ShipItem::ValidateItemSpecifics - Beginning the group validation for %s(%u):", iRef->name(), iRef->itemID());
    if (iRef->HasAttribute(AttrCanFitShipGroup1, fitID)) {
        if (fitID == groupID)
            result = true;
        if (iRef->HasAttribute(AttrCanFitShipGroup2, fitID)) {
            if (fitID == groupID)
                result = true;
            if (iRef->HasAttribute(AttrCanFitShipGroup3, fitID)) {
                if (fitID == groupID)
                    result = true;
                if (iRef->HasAttribute(AttrCanFitShipGroup4, fitID)) {
                    if (fitID == groupID)
                        result = true;
                    if (iRef->HasAttribute(AttrCanFitShipGroup5, fitID)) {
                        if (fitID == groupID)
                            result = true;
                        if (iRef->HasAttribute(AttrCanFitShipGroup6, fitID)) {
                            if (fitID == groupID)
                                result = true;
                            if (iRef->HasAttribute(AttrCanFitShipGroup7, fitID)) {
                                if (fitID == groupID)
                                    result = true;
                                if (iRef->HasAttribute(AttrCanFitShipGroup8, fitID)) {
                                    if (fitID == groupID)
                                        result = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        result = true;
    }

    _log(SHIP__TRACE, "ShipItem::ValidateItemSpecifics - Group Validation returning %s.", (result ? "true" : "false"));

    if (result) {
        _log(SHIP__TRACE, "ShipItem::ValidateItemSpecifics - Beginning the type validation for %s(%u):", iRef->name(), iRef->itemID());

        uint16 typeID(type().id());
        if (iRef->HasAttribute(AttrCanFitShipType1, fitID)) {
            result = false;
            if (fitID == typeID)
                result = true;
            if (iRef->HasAttribute(AttrCanFitShipType2, fitID)) {
                if (fitID == typeID)
                    result = true;
                if (iRef->HasAttribute(AttrCanFitShipType3, fitID)) {
                    if (fitID == typeID)
                        result = true;
                    if (iRef->HasAttribute(AttrCanFitShipType4, fitID)) {
                        if (fitID == typeID)
                            result = true;
                    }
                }
            }
        }

        _log(SHIP__TRACE, "ShipItem::ValidateItemSpecifics - Type Validation returning %s.", (result ? "true" : "false"));
    }

    return result;
}


/* DynamicSystemEntity representing ship object in space */
ShipSE::ShipSE(InventoryItemRef self, EVEServiceManager &services, SystemManager* pSystem, const FactionData& data)
: DynamicSystemEntity(self, services, pSystem),
m_shipRef(ShipItemRef::StaticCast(self)),
m_processTimerTick(SHIP_PROCESS_TICK_MS),   //5s
m_processTimer(SHIP_PROCESS_TICK_MS),
m_oldArmor(0),
m_oldShield(0),
m_oldScanRes(0),
m_oldInertia(0.0f),
m_oldTargetRange(0),
m_boosted(false),
m_podShipID(0),
m_allowFleetSMBUsage(false)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    m_boost = BoostData();

    m_towerPass = "";
    m_processTimer.Start(m_processTimerTick);
    _log(SHIP__TRACE, "Created ShipSE %p for item %u", this, self->itemID());
}

float ShipSE::CalculateRechargeRate(float Capacity, float Current, float RechargeTimeMS)
{
    // C = Cmax * [ 1 + ( SQRT(C0/Cmax) - 1) * EXP((t0-t1)/tau) ] ^ 2
    // dC/dt = (SQRT(C/Cmax) - C/Cmax) * 2 * Cmax / tau
    // tau = "Cap Recharge Time" / 5.0

    // prevent divide by zero.
    RechargeTimeMS = (RechargeTimeMS < 1 ? 1 : RechargeTimeMS);
    Current = (Current < 1 ? 1 : Current);
    float Cmax = (Capacity < 1 ? 1 : Capacity);

    // tau = "cap recharge time" / 5.0
    float tau = (RechargeTimeMS / 5000.0);
    // (2*Cmax) / tau
    float Cmax2_tau = ((Cmax * 2) / tau);
    float C = Current;
    // C / Cmax
    float C_Cmax = (C / Cmax);
    // sqrt( C / Cmax)
    float sC_Cmax = sqrt(C_Cmax);
    // charge rate in Gj / sec
    return (Cmax2_tau * (sC_Cmax - C_Cmax));
}

void ShipSE::Process() {
    if (m_killed)
        return;
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    // check to see if this is an empty ship, and exit if so.
    //  we're not worried about recharge and modules for empty ships (segfaults)
    /** @todo m_self is NOT being populated for non-piloted ships...check later */
    if ((m_self.get() == nullptr) or (!m_self->HasPilot()))
        return;

    if (m_processTimer.Check()) {
        double profileStartTime(GetTimeUSeconds());
        // shield
        float Charge = m_self->GetAttribute(AttrShieldCharge).get_float();
        float Capacity = m_self->GetAttribute(AttrShieldCapacity).get_float();
        if (Charge < Capacity) {
            float newCharge = Charge + ((m_processTimerTick /1000) * CalculateRechargeRate(Capacity, Charge, m_self->GetAttribute(AttrShieldRechargeRate).get_float()));
            if (newCharge > Capacity) {
                newCharge = Capacity;
            } else if ((Capacity - newCharge) < 0.3) {
                newCharge = Capacity;
            }
            m_self->SetAttribute(AttrShieldCharge, newCharge);
            SendDamageStateChanged();
            _log(SHIP__RECHARGE, "ShipSE::Process(): %s(%u) - New Shield Charge: %f", m_self->GetPilot()->GetName(), m_self->itemID(), newCharge);
        }

        // cap
        Charge = m_self->GetAttribute(AttrCapacitorCharge).get_float();
        Capacity = m_self->GetAttribute(AttrCapacitorCapacity).get_float();
        if (Charge < Capacity) {
            float newCharge = Charge + ((m_processTimerTick /1000) * CalculateRechargeRate(Capacity, Charge, m_self->GetAttribute(AttrRechargeRate).get_float()));
            if (newCharge > Capacity) {
                newCharge = Capacity;
            } else if ((Capacity - newCharge) < 0.3) {
                newCharge = Capacity;
            }
            m_self->SetAttribute(AttrCapacitorCharge, newCharge);
            _log(SHIP__RECHARGE, "ShipSE::Process(): %s(%u) - New Cap Charge: %f", m_self->GetPilot()->GetName(), m_self->itemID(), newCharge);
        }
        // profile timer for the ship recharge shit
        if (sConfig.debug.UseProfiling)
            sProfiler.AddTime(Profile::ship, GetTimeUSeconds() - profileStartTime);

        // proc heat on the 5s cap/shield tic, if enabled
        if (sConfig.testing.ShipHeat)
            m_shipRef->ProcessHeat();
    }

    m_shipRef->ProcessModules();
}

void ShipSE::DamageRandModule(float chance) {
    if (chance == 0)
        return;
    if (chance > MakeRandomFloat())
        m_shipRef->DamageRandModule();
}

void ShipSE::PayInsurance() {
    if (m_self->groupID() == EVEDB::invGroups::Rookieship) {
        return;
    }

    std::string reason = "Insurance payment for loss of the ship ";
    reason += m_self->itemName();

    AccountService::TransferFunds(
        corpSCC,
        m_ownerID,
        m_db.GetShipInsurancePayout(m_self->itemID()),
        reason,
        Journal::EntryType::Insurance,
        m_self->typeID()
    );

    ShipDB::DeleteInsuranceByShipID(m_self->itemID());
}

void ShipSE::ResetShipSystemMgr(SystemManager* pSystem) {
    m_system = pSystem;
}

void ShipSE::SetPilot(Client* pClient) {
    m_self->SetPlayer(pClient);
    if (pClient == nullptr) {
        m_allyID = 0;
        m_corpID = 0;
        return;
    }
    // set shipSE data
    m_allyID = pClient->GetAllianceID();
    m_corpID = pClient->GetCorporationID();
}

bool ShipSE::IsInvul()
{
    if (m_shipRef->HasPilot())
        return m_shipRef->GetPilot()->IsInvul();
    return false;
}

bool ShipSE::IsLogin()
{
    if (m_shipRef->HasPilot())
        return m_shipRef->GetPilot()->IsLogin();
    return false;
}

void ShipSE::Dock() {
    if (m_targMgr != nullptr) {
        m_targMgr->ClearModules();
        m_targMgr->ClearAllTargets(false);
        //m_targMgr->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::Docking);
    }

    m_shipRef->Dock();
}

void ShipSE::Jump(bool showCloak) {
    if (m_targMgr != nullptr) {
        m_targMgr->ClearModules();
        m_targMgr->ClearAllTargets(false);
        //m_targMgr->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::Jumping);
    }

    m_shipRef->Jump();
    m_destiny->Jump(showCloak);
}

void ShipSE::Warp() {
    if (m_targMgr != nullptr)
        m_targMgr->ClearModules();

    m_shipRef->Warp();
}

void ShipSE::RemoveTarget(SystemEntity* pSE) {
    // target has been unlocked
    m_shipRef->GetModuleManager()->RemoveTarget(pSE);

    m_targMgr->ClearTarget(pSE);
    //m_targMgr->OnTarget(pSE, TargMgr::Mode::Lost, TargMgr::Msg::StoppedTargeting);
}

void ShipSE::EncodeDestiny( Buffer& into) {
    using namespace Destiny;

    uint8 mode = m_destiny->GetState(); //Ball::Mode::STOP;
/*
    NameStruct name;
        name.name = GetName();
        name.name_len = sizeof(name.name);
  */
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        if (m_self->HasPilot()) {
            head.flags = Ball::Flag::IsInteractive | Ball::Flag::IsFree;
        } else {
            head.flags = Ball::Flag::IsFree;
        }
    into.Append( head);
    MassSector mass = MassSector();
        mass.mass = m_destiny->GetMass();
        mass.cloak = (m_destiny->IsCloaked() ? 1 : 0);
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAlliance(m_allyID) ? m_allyID : -1);
    into.Append( mass);
    DataSector data = DataSector();
        data.inertia = m_destiny->GetInertia();
        data.maxSpeed = m_destiny->GetMaxVelocity();
        data.velX = m_destiny->GetVelocity().x;
        data.velY = m_destiny->GetVelocity().y;
        data.velZ = m_destiny->GetVelocity().z;
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data);
    switch (mode) {
        case Ball::Mode::WARP: {
            GPoint target = m_destiny->GetTargetPoint();
            WARP_Struct warp;
                warp.formationID = 0xFF;
                warp.targX = target.x;
                warp.targY = target.y;
                warp.targZ = target.z;
                warp.speed = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
                // warp timing.  see ShipSE::EncodeDestiny() for notes/updates
                warp.effectStamp = -1; //m_destiny->GetStateStamp();   //timestamp when warp started
                warp.followRange = 0;   //this isnt right
                warp.followID = 0;  //this isnt right
            into.Append(warp);
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowDistance();
                follow.formationID = 0xFF;
            into.Append(follow);
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
                orbit.targetID = m_destiny->GetTargetID();
                orbit.followRange = m_destiny->GetFollowDistance();
                orbit.formationID = 0xFF;
            into.Append(orbit);
        }  break;
        case Ball::Mode::GOTO: {
            GPoint target = m_destiny->GetTargetPoint();
            GOTO_Struct go;
                go.formationID = 0xFF;
                go.x = target.x;
                go.y = target.y;
                go.z = target.z;
            into.Append(go);
        }  break;
        default: {
            STOP_Struct main;
                main.formationID = 0xFF;
            into.Append(main);
        } break;
    }

    std::string modeStr = "Goto";
    switch (mode) {
        case 1: modeStr = "Follow"; break;
        case 2: modeStr = "Stop"; break;
        case 3: modeStr = "Warp"; break;
        case 4: modeStr = "Orbit"; break;
        case 5: modeStr = "Missile"; break;
        case 6: modeStr = "Mushroom"; break;
        case 7: modeStr = "Boid"; break;
        case 8: modeStr = "Troll"; break;
        case 9: modeStr = "Miniball"; break;
        case 10: modeStr = "Field"; break;
        case 11: modeStr = "Rigid"; break;
        case 12: modeStr = "Formation"; break;
    }

    _log(SE__DESTINY, "ShipSE::EncodeDestiny(): %s - id:%lli, mode:%s, flags:0x%X, Vel:%.1f, %.1f, %.1f", \
            GetName(), head.entityID, modeStr.c_str(), head.flags, data.velX, data.velY, data.velZ);
}

void ShipSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() + 7;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

PyDict* ShipSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Ship %s(%u)", m_self->name(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",               new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",               new PyInt(m_self->typeID()));
        slim->SetItemString("name",                 new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",              new PyInt(m_ownerID));
        slim->SetItemString("charID",               new PyInt(m_self->GetPilot() ? m_self->GetPilot()->GetCharacterID() : 0));
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("bounty",               new PyFloat(m_self->GetPilot() ? m_self->GetPilot()->GetBounty() : 0));
        slim->SetItemString("securityStatus",       new PyFloat(m_self->GetPilot() ? m_self->GetPilot()->GetSecurityRating() : 0.0));
    if (m_self->typeID() == itemTypeCapsule) {
        slim->SetItemString("launcherID",           new PyInt(m_podShipID));
        return slim;
    } else {
        slim->SetItemString("categoryID",           new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",              new PyInt(m_self->groupID()));
    }

    //encode the hiSlot and Subsystem modules list ONLY
    std::vector<InventoryItemRef> items;
    m_self->GetMyInventory()->GetItemsByFlagRange(flagHiSlot0, flagHiSlot7, items);
    //m_self->GetMyInventory()->GetItemsByFlagRange(flagSubSystem0, flagSubSystem7, items);
    if (!items.empty()) {
        PyList *list = new PyList();
        for (auto cur : items)
            list->AddItem(new_tuple(cur->itemID(), cur->typeID()));

        slim->SetItemString("modules", list );
    }

    if (is_log_enabled(DESTINY__DEBUG)) {
        _log( DESTINY__DEBUG, "ShipSE::MakeSlimItem() - %s(%u)", GetName(), GetID());
        slim->Dump(DESTINY__DEBUG, "     ");
    }

    return slim;
}

void ShipSE::ClearBoostData()
{
    m_oldArmor       = 0;
    m_oldShield      = 0;
    m_oldScanRes     = 0;
    m_oldInertia     = 0.0f;
    m_oldTargetRange = 0;

    m_boost = BoostData();
    m_boosted = false;
}

void ShipSE::RemoveBoost()
{
    _log( FLEET__TRACE, "ShipSE::RemoveBoost() - %s(%u)", GetName(), GetID());

    m_shipRef->SetAttribute(AttrArmorHP, m_oldArmor);
    m_shipRef->SetAttribute(AttrInetia, m_oldInertia);
    m_shipRef->SetAttribute(AttrShieldCapacity, m_oldShield);
    m_shipRef->SetAttribute(AttrScanResolution, m_oldScanRes);
    m_shipRef->SetAttribute(AttrMaxTargetRange, m_oldTargetRange);

    m_destiny->UpdateShipVariables();

    ClearBoostData();
}

void ShipSE::ApplyBoost(BoostData& bData)
{
    // note:  mining boost applied in mining module code

    // remove existing boost
    if (m_boosted)
        RemoveBoost();

    _log( FLEET__TRACE, "ShipSE::ApplyBoost() - %s(%u)", GetName(), GetID());

    m_boost             = bData;
    m_oldArmor          = m_shipRef->GetAttribute(AttrArmorHP).get_uint32();
    m_oldInertia        = m_shipRef->GetAttribute(AttrInetia).get_float();
    m_oldShield         = m_shipRef->GetAttribute(AttrShieldCapacity).get_uint32();
    m_oldScanRes        = m_shipRef->GetAttribute(AttrScanResolution).get_uint32();
    m_oldTargetRange    = m_shipRef->GetAttribute(AttrMaxTargetRange).get_uint32();

    uint16 armorHP      = m_oldArmor       * (1.0f + (0.02f * m_boost.armored)); // 2% increase/level
    uint16 shieldHP     = m_oldShield      * (1.0f + (0.02f * m_boost.siege));// 2% increase/level
    uint16 scanRes      = m_oldScanRes     * (1.0f + (0.02f * m_boost.leader));// 2% increase/level
    uint32 targRange    = m_oldTargetRange * (1.0f + (0.02f * m_boost.info));// 2% increase/level
    float inertia       = m_oldInertia     * (1.0f - (0.02f * m_boost.skirmish));// 2% decrease/level

    m_shipRef->SetAttribute(AttrInetia, inertia);   // lower inertia = lower agility = faster ship
    m_shipRef->SetAttribute(AttrArmorHP, armorHP);
    m_shipRef->SetAttribute(AttrScanResolution, scanRes);  // higher scanRes = faster targeting
    m_shipRef->SetAttribute(AttrShieldCapacity, shieldHP);
    m_shipRef->SetAttribute(AttrMaxTargetRange, targRange);

    m_destiny->UpdateShipVariables();

    m_boosted = true;
}
//{'FullPath': u'UI/Messages', 'messageID': 257802, 'label': u'DronesDroppedBecauseOfBandwidthModificationBody'}(u'The drone control bandwidth of your ship has been modified causing you to lose the ability to control some drones.', None, None)

bool ShipSE::LaunchDrone(InventoryItemRef dRef) {
    Character* pChar = GetPilot()->GetChar().get();
    sLog.Magenta("ShipSE::LaunchDrone()","%s: Launching drone %u",  pChar->name(), dRef->itemID());

    dRef->Move(GetLocationID(), flagNone, true);
    dRef->ChangeSingleton(true);

    GPoint position(GetPosition());
    position.MakeRandomPointOnSphere(500.0);
    dRef->SetPosition(position);

    //now we create an SE to represent it.
    FactionData data = FactionData();
        data.allianceID = pChar->allianceID();
        data.corporationID = pChar->corporationID();
        data.factionID = pChar->warFactionID();
        data.ownerID = pChar->itemID();
    DroneSE* pDrone = new DroneSE(dRef, m_services, m_system, data);

    // tell new drone it's being launched.
    pDrone->Launch(this);
    // add drone to launched drone map (whether onlined or not)
    m_drones.emplace(dRef->itemID(), dRef.get());

    /*
    AttrDroneBandwidth = 1271,     <-- ship attribute  (total)
    AttrDroneBandwidthUsed = 1272, <-- drone attribute
    AttrDroneBandwidthLoad = 1273, <-- ship attribute  (current used)
    */
    //  if ship doesnt have bandwidth for drone, it will not online after launch (inert)
    EvilNumber load = m_shipRef->GetAttribute(AttrDroneBandwidthLoad);
    load += dRef->GetAttribute(AttrDroneBandwidthUsed);
    if (load <= m_shipRef->GetAttribute(AttrDroneBandwidth)) {
        pDrone->Online();
        pDrone->GetAI()->SetIdle();
        m_shipRef->SetAttribute(AttrDroneBandwidthLoad, load, false); // client dont care
        return true;
    }
    //{'FullPath': u'UI/Messages', 'messageID': 258031, 'label': u'MaxBandwidthExceededBody'}(u"You don't have enough bandwidth to launch {droneName}. You need {bandwidthNeeded} Mbit/s but {droneName} requires {droneBandwidthUsed} Mbit/s.", None, {u'{droneName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'droneName'}, u'{droneBandwidthUsed}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'droneBandwidthUsed'}, u'{bandwidthNeeded}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'bandwidthNeeded'}})
    //{'FullPath': u'UI/Messages', 'messageID': 258041, 'label': u'MaxBandwidthExceeded2Body'}(u"You don't have enough bandwidth to launch {droneName}. You need {droneBandwidthUsed} Mbit/s but only have {bandwidthLeft} Mbit/s available.", None, {u'{droneName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'droneName'}, u'{bandwidthLeft}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'bandwidthLeft'}, u'{droneBandwidthUsed}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'droneBandwidthUsed'}})
    return false;
}

void ShipSE::ScoopDrone(SystemEntity* pSE) {
    m_drones.erase(pSE->GetID());
    pSE->GetDroneSE()->Offline();
    EvilNumber load = m_shipRef->GetAttribute(AttrDroneBandwidthLoad);
    load -= pSE->GetSelf()->GetAttribute(AttrDroneBandwidthUsed);
    m_shipRef->SetAttribute(AttrDroneBandwidthLoad, load, false); // client dont care
}

void ShipSE::UpdateDrones(std::map<int16, int8> &attribs) {
    // update drones in space with new attrib settings
    for (auto cur : m_drones) {
        cur.second->SetAttribute(AttrDroneFocusFire, attribs[AttrDroneFocusFire]);
        cur.second->SetAttribute(AttrDroneIsAgressive, attribs[AttrDroneIsAgressive]);
        cur.second->SetAttribute(AttrFightersAttackAndFollow, attribs[AttrFightersAttackAndFollow]);
        //cur.second->SetAttribute(AttrDroneIsChaotic, attribs[AttrDroneIsChaotic]);    // no longer used?
    }
}

void ShipSE::AbandonDrones() {
    SystemEntity* pSE(nullptr);
    EvilNumber load = m_shipRef->GetAttribute(AttrDroneBandwidthLoad);
    for (auto cur : m_drones) {
        pSE = m_system->GetSE(cur.first);
        if (pSE != nullptr)
            if (pSE->IsDroneSE()) {
                pSE->GetDroneSE()->Abandon();
                load -= pSE->GetSelf()->GetAttribute(AttrDroneBandwidthUsed);
            }
    }
    m_shipRef->SetAttribute(AttrDroneBandwidthLoad, load, false); // client dont care
}
//AttrDroneControlDistance
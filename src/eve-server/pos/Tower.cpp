
/**
 * @name Tower.cpp
 *   Class for POS Towers.
 *
 * @Author:         Allan
 * @date:   28 December 17
 */

/*
 * POS__ERROR
 * POS__WARNING
 * POS__MESSAGE
 * POS__DUMP
 * POS__DEBUG
 * POS__DESTINY
 * POS__SLIMITEM
 * POS__TRACE
 */


#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "planet/Moon.h"
#include "pos/Tower.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

/** @todo (Allan) this class needs more research to finish
 * see pics in ::GamePC/G/games/EvE/misc/POS
 * flagStructureActive             = 144,
 * flagStructureInactive           = 145,
 * AttrOperationConsumptionRate = 687,
 * AttrReinforcedConsumptionRate = 688,
 * AttrResourceReinforced1Type = 694,
 * AttrResourceReinforced2Type = 695,
 * AttrResourceReinforced3Type = 696,
 * AttrResourceReinforced4Type = 697,
 * AttrResourceReinforced5Type = 698,
 * AttrResourceReinforced1Quantity = 699,
 * AttrResourceReinforced2Quantity = 700,
 * AttrResourceReinforced3Quantity = 701,
 * AttrResourceReinforced4Quantity = 703,
 * AttrResourceReinforced5Quantity = 704,
 * AttrResourceOnline1Type = 705,
 * AttrResourceOnline2Type = 706,
 * AttrResourceOnline3Type = 707,
 * AttrResourceOnline4Type = 708,
 ***  many other attributes for towers and their modules.....
 * AttrControlTowerMissileVelocityBonus = 792,
 * AttrControlTowerSize = 1031,
 * AttrAnchoringSecurityLevelMax = 1032,
 * AttrAnchoringRequiresSovereignty = 1033,
 * AttrControlTowerMinimumDistance = 1165,
 * AttrPosPlayerControlStructure = 1167,
 * AttrIsIncapacitated = 1168,
 * AttrPosStructureControlAmount = 1174,
 * AttrOnliningRequiresSovereigntyLevel = 1185,
 * AttrPosAnchoredPerSolarSystemAmount = 1195,
 * AttrPosStructureControlDistanceMax = 1214,
 * AttrAnchoringRequiresSovereigntyLevel = 1215,
 * AttrHarvesterType = 709,
 * AttrHarvesterQuality = 710,
 * AttrMoonAnchorDistance = 711,
 * AttrUsageDamagePercent = 712,
 * AttrConsumptionType = 713,
 * AttrConsumptionQuantity = 714,
 * AttrMaxOperationalDistance = 715,    for sma, hangar, etc
 * AttrMaxOperationalUsers = 716,    for sma, hangar, etc
 * AttrRefiningYieldMultiplier = 717,
 * AttrOperationalDuration = 719,
 * AttrRefineryCapacity = 720,
 * AttrRefiningDelayMultiplier = 721,
 * AttrPosControlTowerPeriod = 722,
 * AttrMoonMiningAmount = 726,
 * AttrControlTowerLaserDamageBonus = 728,
 * AttrControlTowerLaserOptimalBonus = 750,
 * AttrControlTowerHybridOptimalBonus = 751,
 * AttrControlTowerProjectileOptimalBonus = 752,
 * AttrControlTowerProjectileFallOffBonus = 753,
 * AttrControlTowerProjectileROFBonus = 754,
 * AttrControlTowerMissileROFBonus = 755,
 * AttrControlTowerMoonHarvesterCPUBonus = 756,
 * AttrControlTowerSiloCapacityBonus = 757,
 * AttrControlTowerLaserProximityRangeBonus = 760,
 * AttrControlTowerProjectileProximityRangeBonus = 761,
 * AttrControlTowerHybridProximityRangeBonus = 762,
 * AttrMaxGroupActive = 763,
 * AttrControlTowerEwRofBonus = 764,
 * AttrScanRange = 765,
 * AttrControlTowerHybridDamageBonus = 766,
 * AttrTrackingSpeedBonus = 767,
 * AttrMaxRangeBonus2 = 769,
 * AttrControlTowerEwTargetSwitchDelayBonus = 770,
 * AttrAmmoCapacity = 771,
 * AttrActivationBlocked = 1349,
 * AttrActivationBlockedStrenght = 1350,
 * AttrPosCargobayAcceptType = 1351,
 * AttrPosCargobayAcceptGroup = 1352,
 */

TowerSE::TowerSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData)
: StructureSE(structure, services, system, fData),
m_pShieldSE(nullptr)
{
    m_hasShield = false;
    m_structs.clear();

    // create AI object for tower here....not written yet.
    //m_ai = new POS_AI(this);

    m_pg = m_self->GetAttribute(AttrPowerOutput).get_int();
    m_cpu = m_self->GetAttribute(AttrCpuOutput).get_int();

    m_tsize = m_self->GetAttribute(AttrControlTowerSize).get_int();
    if ((m_tsize < 1) or (m_tsize > 3))
        m_tsize = 1;  // do something constructive here cause size is wrong
    m_soi = m_self->GetAttribute(AttrPosStructureControlDistanceMax).get_int() * m_tsize;

    m_tdata = EVEPOS::TowerData();

    /** @note these are defined, but i dunno what they are
     * AttrControlTowerMinimumDistance
     *
     *
     *
     *
     *
     */
}

void TowerSE::Init()
{
    StructureSE::Init();

    if (!m_db.GetTowerData(m_tdata, m_data)) {
        _log(SE__TRACE, "TowerSE %s(%u) has no saved data.  Initializing default set.", m_self->name(), m_self->itemID());
        // invalid data....init to 0 as this will only hit for currently-launching items (or errors)
        InitData();
    }

    // if password is already set and tower online, then we can online (create) the forcefield
    m_harmonic = m_tdata.harmonic;
    if ((m_harmonic > EVEPOS::Harmonic::Offline)
    and (!m_tdata.password.empty())
    and (m_data.state > EVEPOS::StructureState::Anchored))
        CreateForceField();

    // if tower anchored, tell moon this is its tower
    if (m_data.state > EVEPOS::StructureState::Unanchored)
        m_moonSE->SetTower(this);

    // set tower in bubble
    if (m_bubble == nullptr)
        assert(0);
    m_bubble->SetTowerSE(this);

    /** @todo
     * will have to check if Online or Operating
     *   in these two cases, remove resources based on time running.
     * if resources run out, reset state and set timer accordingly.
     * if reinforced, remove resources and continue until timer runs out.
     *
     * timer is in base StructureSE code.
     */

    // take resources, move items, process reactions or whatever needs to be done (follow PI proc code)

}

void TowerSE::InitData() {
    // init base data first
    StructureSE::InitData();
    m_tdata.harmonic = m_harmonic;     // set during base SE creation
    m_tdata.standingOwnerID = 0;    /** @todo  get sov holder here. */

    m_db.SaveTowerData(m_tdata, m_data);
}

void TowerSE::Scoop() {
    StructureSE::Scoop();
    m_moonSE->SetTower(nullptr);
    m_tdata = EVEPOS::TowerData();
    m_self->ChangeSingleton(false);
    m_self->SaveItem();
}

void TowerSE::Process()
{
    /* called by EntityList::Process on every loop */

    // starbase charter checks for empire space

    // tower-specific tests here

    /*  Enable base call to Process Anchoring, Targeting and Movement  */
    StructureSE::Process();
}

/*
 * 473     prototypingBonus    250000  NULL
 * 556     anchoringDelay  1800000     NULL
 * 650     maxStructureDistance    50000   NULL
 * 676     unanchoringDelay    NULL    3600000
 * 677     onliningDelay   1800000     NULL
 * 680     shieldRadius    30000   NULL
 * 711     moonAnchorDistance  100000  NULL
 * 1214    posStructureControlDistanceMax  NULL    15000
 */

void TowerSE::SetOnline()
{
    //StructureSE::SetOnline();

    m_data.timestamp = GetFileTimeNow();
    m_self->SetFlag(flagStructureActive);
    m_procState = EVEPOS::ProcState::Online;
    m_data.state = EVEPOS::StructureState::Online;
    m_harmonic = EVEPOS::Harmonic::Online;
    m_tdata.harmonic = m_harmonic;
    SetTimer(m_self->GetAttribute(AttrOnliningDelay).get_int());

    if ((m_harmonic > EVEPOS::Harmonic::Offline)
    and (!m_tdata.password.empty()))
        CreateForceField();

    SendSlimUpdate();
    m_destiny->SendSpecialEffect(m_self->itemID(),m_self->itemID(),m_self->typeID(),0,0,"effects.StructureOnline",0,1,1,-1,0);

    m_db.UpdateBaseData(m_data);

    /** @todo determine fuel supply and make calendar event for expiration

    std::string title = "Expiration of Tower fuel";

    std::string description = "The Control Tower ";
    description += ** get tower name here **;
    description += " in ";
    description += ** get tower location name here **;
    description += " will exhaust it's current fuel supply at this time.";      // change this to eve datetime?
    CalendarDB::SaveSystemEvent(call.client->GetCorporationID(), [towerID here?], expiryTime,
                                Calendar::AutoEvent::PosFuel, title, description, true);
     */
}

void TowerSE::SetOffline()
{
    if (m_hasShield) {
        m_pShieldSE->Delete();
        SafeDelete(m_pShieldSE);
        m_hasShield = false;
    }

    StructureSE::SetOffline();
}


void TowerSE::Online()
{
    // structure online, but not operating
    // take resources or whatever needs to be done
    /*
     * 1031    controlTowerSize    3   NULL
     *
     * POS fuel usage per hour
     *  Fuel blocks    s:10   m:20   l:40
     */

    // if fuel has run out, start reinforced mode.


    // reset timers
    StructureSE::Online();

}

void TowerSE::Operating()
{
    // structure operating
    // take resources or whatever needs to be done

    //1031    controlTowerSize    3

    // if fuel has run out, start reinforced mode.

    // reset timers
    StructureSE::Operating();

}

void TowerSE::ReinforceTower()
{
    //  see how many stront is in tower and set timer accordingly
    // Strontium Clathrates   s:100  m:200  l:400

}

void TowerSE::Reinforced()
{
    // dunno what to do here yet.

}

void TowerSE::UpdatePassword()
{
    if (m_tdata.password.empty()) {
        m_harmonic =EVEPOS::Harmonic::Offline;
        m_tdata.harmonic = m_harmonic;
        if (m_pShieldSE == nullptr)
            return;

        m_pShieldSE->SetHarmonic(m_harmonic);

        //  this is for UPDATING forcefield ONLY...do not send on creation.
        std::vector<PyTuple*> updates;
        //  'massive' enables client-side bounce
        SetBallMassive sbm;
            sbm.entityID = m_pShieldSE->GetSelf()->itemID();
            sbm.is_massive = false;         // disable client-side bump checks
        updates.push_back(sbm.Encode());
        // harmonic for ForceField
        SetBallHarmonic sbh;
            sbh.itemID = m_pShieldSE->GetSelf()->itemID();
            sbh.corpID = m_corpID;
            sbh.allianceID = m_allyID;
            sbh.mass = -1;      // always -1
            sbh.harmonic = m_harmonic;
        updates.push_back(sbh.Encode());
        m_destiny->SendDestinyUpdate(updates); //consumed
    } else {
        m_harmonic = EVEPOS::Harmonic::Online;
        m_tdata.harmonic = m_harmonic;

        if (m_data.state > EVEPOS::StructureState::Anchored)
            CreateForceField();
    }

    m_db.UpdateHarmonicAndPassword(m_data.itemID, m_tdata);
}

void TowerSE::SetDeployFlags(int8 anchor/*0*/, int8 unanchor/*0*/, int8 online/*0*/, int8 offline/*0*/)
{
    m_tdata.anchor = anchor;
    m_tdata.unanchor = unanchor;
    m_tdata.online = online;
    m_tdata.offline = offline;

    m_db.UpdateDeployFlags(m_data.itemID, m_tdata);
}

PyRep* TowerSE::GetDeployFlags()
{
    PyList* header = new PyList(4);
        header->SetItemString(0, "anchor");
        header->SetItemString(1, "unanchor");
        header->SetItemString(2, "online");
        header->SetItemString(3, "offline");
    PyList* line = new PyList(4);           // these are structure permissions for this tower
        line->SetItem(0, new PyInt(m_tdata.anchor));
        line->SetItem(1, new PyInt(m_tdata.unanchor));
        line->SetItem(2, new PyInt(m_tdata.online));
        line->SetItem(3, new PyInt(m_tdata.offline));

    PyDict* dict = new PyDict();
    dict->SetItemString("header", header);
    dict->SetItemString("line", line);

    return new PyObject("util.Row", dict);
}

void TowerSE::SetUseFlags(uint32 itemID, int8 view, int8 take, int8 use/*0*/)
{
    std::map<uint32, StructureSE*>::iterator itr = m_structs.find(itemID);
    if (itr != m_structs.end()) {
        itr->second->SetUsageFlags(view, take, use);
        itr->second->UpdateUsageFlags();
    }
}

PyRep* TowerSE::GetUsageFlagList()
{
    /*
                [PyObject Name: eve.common.script.sys.rowset.Rowset]
                    [PyDict 3 kvp]
                        Key:[PyString "header"]
                        ==Value:[PyList 4 items]
                                    [PyString "structureID"]
                                    [PyString "viewput"]
                                    [PyString "viewputtake"]
                                    [PyString "use"]
                        Key:[PyString "RowClass"]
                        ==Value:[PyToken carbon.common.script.sys.row.Row]
                        Key:[PyString "lines"]
                        ==Value:[PyList 1 items]
                                    [PyList 4 items]
                                        [PyIntegerVar 1010759458081]
                                        [PyInt 3]
                                        [PyInt 0]
                                        [PyInt 0]
                                        */

    PyList* header = new PyList(4);
        header->SetItemString(0, "structureID");
        header->SetItemString(1, "viewput");
        header->SetItemString(2, "viewputtake");
        header->SetItemString(3, "use");
    PyList* lines = new PyList();
    for (auto cur : m_structs) {
        PyList* line = new PyList(4);
            line->SetItem(0, new PyInt(cur.first));
            line->SetItem(1, new PyInt(cur.second->CanView()));
            line->SetItem(2, new PyInt(cur.second->CanTake()));
            line->SetItem(3, new PyInt(cur.second->CanUse()));
        lines->AddItem(line);
    }

    PyDict* dict = new PyDict();
    dict->SetItemString("header", header);
    dict->SetItemString("RowClass", new PyToken("util.Row"));
    dict->SetItemString("lines", lines);

    return new PyObject("util.Rowset", dict);
}

PyRep* TowerSE::GetProcessInfo()
{
    /*
            info = self.posMgr.GetMoonProcessInfoForTower(self.slimItem.itemID)
            itemID, active, reaction, connections, demands, supplies in info:

            if reaction and rec.groupID == const.groupMobileReactor and not demands and not supplies:
                demands = [ (row.typeID, row.quantity) for row in cfg.invtypereactions[reaction[1]] if row.input == 1 ]
                supplies = [ (row.typeID, row.quantity) for row in cfg.invtypereactions[reaction[1]] if row.input == 0 ]
                demand = {}
                demands = demands or []
                for tID, quant in demands:
                    demand[tID] = quant

                    supply = {}
                    supplies = supplies or []
                    for tID, quant in supplies:
                        supply[tID] = quant

                for sourceID, tID in connections:
                    self.sr.structureConnections[tID, sourceID] = itemID

        [PyList 2 items]
          [PyTuple 6 items]
            [PyIntegerVar 1002332982210]
            [PyBool False]
            [PyNone]
            [PyList 0 items]
            [PyList 0 items]
            [PyList 0 items]
          [PyTuple 6 items]
            [PyIntegerVar 1002331680835]
            [PyBool False]
            [PyNone]
            [PyList 0 items]
            [PyList 0 items]
            [PyList 0 items]
            */

    PyList* list = new PyList();
    for (auto cur : m_structs) {
        switch (cur.second->GetSelf()->groupID()) {
            case EVEDB::invGroups::Moon_Mining:
            case EVEDB::invGroups::Silo:
            case EVEDB::invGroups::Mobile_Reactor: {
                PyTuple* tuple = new PyTuple(6);
                tuple->SetItem(0, new PyInt(cur.first));
                tuple->SetItem(1, new PyBool(false));
                tuple->SetItem(2, PyStatic.NewNone());
                tuple->SetItem(3, new PyList());
                tuple->SetItem(4, new PyList());
                tuple->SetItem(5, new PyList());
                list->AddItem(tuple);
            }
        }
    }

    if (is_log_enabled(POS__RSP_DUMP))
        list->Dump(POS__RSP_DUMP, "   ");
    return list;
}

void TowerSE::CreateForceField()
{
    if (m_hasShield)
        return;
    // create and add force field to tower
    ItemData idata(EVEDB::invTypes::ForceField, m_corpID, m_system->GetID(), flagNone, m_ownerID);
    InventoryItemRef ifRef = sItemFactory.SpawnItem(idata);
    if (ifRef.get() == nullptr)
        return;  // we'll get over it
    ifRef->SetPosition(GetPosition());
    ifRef->SetAttribute(AttrRadius, m_self->GetAttribute(AttrShieldRadius), false);
    FactionData data = FactionData();
        data.allianceID = m_allyID;
        data.corporationID = m_corpID;
        data.factionID = m_warID;
        data.ownerID = m_ownerID;
    FieldSE* iSE = new FieldSE(ifRef, m_services, m_system, data);
    // set shield harmonic to tower harmonic
    iSE->SetHarmonic(m_harmonic);
    m_system->AddEntity(iSE);
    m_pShieldSE = iSE;
    m_hasShield = true;
}

PyDict* TowerSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for TowerSE %u", m_self->itemID());
    _log(POS__SLIMITEM, "MakeSlimItem for TowerSE %u", m_self->itemID());

    PyDict *slim = new PyDict();
    slim->SetItemString("name",                     new PyString(m_self->itemName()));
    slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
    slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",                  new PyInt(m_ownerID));
    slim->SetItemString("corpID",                   IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
    slim->SetItemString("allianceID",               IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
    slim->SetItemString("warFactionID",             IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    slim->SetItemString("posTimestamp",             new PyLong(m_data.timestamp));
    slim->SetItemString("posState",                 new PyInt(m_data.state));
    slim->SetItemString("incapacitated",            new PyInt((m_data.state == EVEPOS::StructureState::Incapacitated) ? 1 : 0));
    slim->SetItemString("posDelayTime",             new PyInt(m_delayTime));

    if (is_log_enabled(POS__SLIMITEM)) {
        _log( POS__SLIMITEM, "TowerSE::MakeSlimItem() - %s(%u)", GetName(), GetID());
        slim->Dump(POS__SLIMITEM, "     ");
    }
    return slim;
}
/*
                      [PyString "OnSlimItemChange"]
                      [PyTuple 2 items]
                        [PyIntegerVar 1006120578679]
                        [PyObjectData Name: foo.SlimItem]
                          [PyDict 11 kvp]
                            [PyString "itemID"]
                            [PyIntegerVar 1006120578679]
                            [PyString "typeID"]
                            [PyInt 20060]               <<--  Amarr Control Tower Small
                            [PyString "name"]
                            [PyString "Pix0r monto la torre de al lado"]
                            [PyString "incapacitated"]
                            [PyFloat 0]
                            [PyString "posTimestamp"]
                            [PyIntegerVar 129773067243437304]
                            [PyString "posState"]
                            [PyInt 4]
                            [PyString "warFactionID"]
                            [PyNone]
                            [PyString "allianceID"]
                            [PyInt 99001691]
                            [PyString "corpID"]
                            [PyInt 717154310]
                            [PyString "ownerID"]
                            [PyInt 717154310]
                            [PyString "nameID"]
                            [PyNone]
                    */

/*
 *    def IsShipInRangeOfStructureControlTower(self, shipID, structureID):
 *        """
 *        Returns True if structureID is associated with a control tower (or is itself a tower), and shipID is in range of that tower.
 *        'In range of the tower' means within of the force-field radius.
 *        This function should mirror the behaviour of the equivalent server-side function in park.py
 *        """
 *        structureSlim = self.slimItems.get(structureID)
 *        if structureSlim is None:
 *            return False
 *        controlTowerID = None
 *        if structureSlim.groupID == const.groupControlTower:
 *            controlTowerID = structureID
 *        elif structureSlim.controlTowerID is not None:
 *            controlTowerID = structureSlim.controlTowerID
 *        if controlTowerID is None:
 *            return False
 *        towerSlim = self.slimItems.get(controlTowerID)
 *        if towerSlim is None:
 *            return False
 *        towerShieldRadius = self.broker.godma.GetStateManager().GetType(towerSlim.typeID).shieldRadius
 *        return self.GetCenterDist(controlTowerID, shipID) < towerShieldRadius
 */
 /**
  * @name ProbeItems.cpp
  *     Probe Item/SE class for EVEmu
  *
  * @Author:        Allan
  * @date:          10 March 2018
  *
  */


#include "eve-server.h"

#include "Client.h"
#include "PyServiceMgr.h"
#include "exploration/Probes.h"
#include "exploration/Scan.h"
#include "inventory/AttributeEnum.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

/* SCAN__ERROR
 * SCAN__WARNING
 * SCAN__MESSAGE
 * SCAN__DEBUG
 * SCAN__INFO
 * SCAN__TRACE
 * SCAN__DUMP
 * SCAN__RSPDUMP
 */

ProbeItem::ProbeItem(uint32 itemID, const ItemType& _type, const ItemData& _data)
: InventoryItem(itemID, _type, _data)
{

}
/*
            Scanner_Probe = 479,
            Survey_Probe = 492,
            Warp_Disruption_Probe = 548,
            Obsolete_Probes = 972,
            */
uint32 ProbeItem::CreateItemID(ItemData& data)
{
    return InventoryItem::CreateItemID(data);
}

ProbeItemRef ProbeItem::Load(uint32 itemID)
{
    return InventoryItem::Load<ProbeItem>(itemID );
}

ProbeItemRef ProbeItem::Spawn(ItemData& data)
{
    uint32 itemID = ProbeItem::CreateItemID(data);
    if (itemID == 0 )
        return ProbeItemRef(nullptr);

    return ProbeItem::Load(itemID);
}

void ProbeItem::Delete() {
    // remove from current container's inventory.
    // this should be SolarSystem, but just in case, run tests anyway
    if (IsValidLocation(locationID())) {
        InventoryItemRef iRef = sItemFactory.GetItem(locationID());
        if (iRef.get() != nullptr) {
            iRef->GetMyInventory()->RemoveItem(InventoryItemRef(this));
        } else {
            _log(ITEM__ERROR, "ProbeItem::Delete() - Cant find location %u containing %s.", locationID(), name());
        }
    } else {
        _log(ITEM__ERROR, "ProbeItem::Delete() - Location %u containing %s is invalid.", locationID(), name());
    }

    // remove from DB
    ItemDB::DeleteItem(m_itemID);
    // remove from factory cache
    sItemFactory.RemoveItem(m_itemID);
}


// abandoned probe c'tor
ProbeSE::ProbeSE(ProbeItemRef self, PyServiceMgr& services, SystemManager* system)
: DynamicSystemEntity(self, services, system),
m_scan(nullptr),
m_client(nullptr),
m_shipRef(nullptr),
m_moduleRef(nullptr),
m_secStatus(0),
m_lifeTimer(0),
m_rangeStep(0),
m_scanRange(0),
m_stateTimer(0),
m_returnTimer(0),
m_scanStrength(0),
m_scanDeviation(0),
m_ring(false),
m_sphere(false),
m_scanShips(false)
{
    m_state = Probe::State::Inactive;

    if (self->HasAttribute(AttrProbeCanScanShips))
        m_scanShips = self->GetAttribute(AttrProbeCanScanShips).get_bool();

    // set probe lifetime of 5h
    m_expiry = GetFileTimeNow() + (EvE::Time::Hour *5);  // 5h abandoned lifespan
    m_lifeTimer.Start(5*60*60*1000);        // 5h to ms

    _log(SCAN__INFO, "Created Abandoned ProbeSE for %u. expiry: %li", m_self->itemID(), m_expiry);
}

ProbeSE::ProbeSE(ProbeItemRef self, PyServiceMgr& services, SystemManager* system, InventoryItemRef moduleRef, ShipItemRef shipRef)
: DynamicSystemEntity(self, services, system),
m_scan(nullptr),
m_state(Probe::State::Idle),
m_client(shipRef->GetPilot()),
m_shipRef(shipRef),
m_moduleRef(moduleRef),
m_lifeTimer(0),
m_rangeStep(0),
m_scanRange(0),
m_stateTimer(0),
m_returnTimer(0),
m_rangeFactor(0),
m_scanStrength(0),
m_scanDeviation(0),
m_baseScanRange(0),
m_ring(false),
m_sphere(false),
m_scanShips(false)
{
    m_warID = m_client->GetWarFactionID();
    m_allyID = m_client->GetAllianceID();
    m_corpID = m_client->GetCorporationID();
    m_ownerID = m_client->GetCharacterID();

    m_secStatus = m_client->GetSecurityRating();

    if (self->HasAttribute(AttrProbeCanScanShips))
        m_scanShips = self->GetAttribute(AttrProbeCanScanShips).get_bool();

    // set probe lifetime
    m_expiry = GetFileTimeNow() + (self->GetAttribute(AttrExplosionDelay).get_float() *10000); // ms to filetime
    if (m_expiry < GetFileTimeNow()) {
        m_expiry = GetFileTimeNow() + (EvE::Time::Minute *30);  // 30m default lifespan
        m_lifeTimer.Start(30*60*1000);        // 30m to ms
    } else {
        m_lifeTimer.Start(self->GetAttribute(AttrExplosionDelay).get_uint32());
    }

    // set base str, deviation and range
    m_rangeFactor = self->GetAttribute(AttrRangeFactor).get_uint32();
    m_scanStrength = self->GetAttribute(AttrBaseSensorStrength).get_float();
    m_scanDeviation = self->GetAttribute(AttrBaseMaxScanDeviation).get_float();
    m_baseScanRange = self->GetAttribute(AttrBaseScanRange).get_float() * (ONE_AU_IN_METERS /100);

    // not sure about this one yet....moon survey.  will need special initializers here
    if (self->groupID() == EVEDB::invGroups::Survey_Probe) {
        // 5% reduction to flight time?  10% for t2 ship
    }

    // i think we may have to do probe modifiers here....they are not being done thru fx system
    Character* pChar = m_client->GetChar().get();
    // skills
    m_scanStrength *= (1 + (0.1f * pChar->GetSkillLevel(EvESkill::AstrometricRangefinding)));   // +10% strength per level
    // skill bonuses
    //m_scanStrength *= (1 + (0.01 * pChar->GetSkillLevel(EvESkill::Astrometrics)));             // +1% strength per level
    //m_scanStrength *= (1 + (0.01 * pChar->GetSkillLevel(EvESkill::SignatureAnalysis)));        // +1% strength per level
    //m_scanDeviation *= (1 - (0.01 * pChar->GetSkillLevel(EvESkill::Astrometrics)));            // -1% deviation per level
    //m_scanDeviation *= (1 - (0.01 * pChar->GetSkillLevel(EvESkill::SensorLinking)));           // -1% deviation per level

    // ship
    switch (shipRef->typeID()) {
        //t1
        case 29248: { /* Magnate */
            m_scanStrength *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::AmarrFrigate)))); // +5% strength per level
        } break;
        case 605: { /* Heron */
            m_scanStrength *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::CaldariFrigate)))); // +5% strength per level
        } break;
        case 607: { /* Imicus */
            m_scanStrength *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::GallenteFrigate)))); // +5% strength per level
        } break;
        case 586: { /* Probe */
            m_scanStrength *= (1 + (0.05f * (pChar->GetSkillLevel(EvESkill::MinmatarFrigate)))); // +5% strength per level
        } break;
        //t2 - Anathema, Buzzard, Cheetah, Helios
        case 11188:  /* Anathema */
        case 11192:  /* Buzzard */
        case 11172:  /* Helios */
        case 11182: { /* Cheetah */
            m_scanStrength *= (1 + (0.1f * (pChar->GetSkillLevel(EvESkill::CovertOps)))); // +10% strength per level
        } break;
        //t3
        // just test for subsystem here... typeIDs 30042, 30052, 30062, 30072
        // use same general code as GetRigScanBonus() in MM below...
    }

    // modules (launchers - 5 or 10)
    if (moduleRef->HasAttribute(AttrScanStrengthBonus))
        m_scanStrength *= (1 + (0.01f * moduleRef->GetAttribute(AttrScanStrengthBonus).get_float()));

    // rigs (10 or 15)
    m_scanStrength *= (1 + shipRef->GetModuleManager()->GetRigScanBonus());

    // implants (virtue - 1-5, hardwire 2,6,10)
    // skill (AstrometricRangefinding @ 10%/lvl)  <-- this doesnt get set on character object - checked above...
    // this is just a placeholder.  not sure if this will be how it works yet
    //  also, the bonus will be determined by the implant fx, which would be set in attrib.
    if (pChar->HasAttribute(AttrScanStrengthBonus))
        m_scanStrength *= (1 + (0.01f * pChar->GetAttribute(AttrScanStrengthBonus).get_float()));

    _log(SCAN__INFO, "Created ProbeSE for %u. timeNow: %.0f, expiry: %li, scan Str: %.4f, deviation: %.5f, ship: %s", \
            m_self->itemID(), GetFileTimeNow(), m_expiry, m_scanStrength, m_scanDeviation, m_scanShips?"true":"false");
}

    /*
    AttrScanAnalyzeCount = 791,     (no db data)
    AttrScanProbeStrength = 1116,         (no db data)
    AttrScanStrengthSignatures = 1117,         (no db data)
    AttrScanStrengthDronesProbes = 1118,         (no db data)
    AttrScanStrengthScrap = 1119,         (no db data)
    AttrScanStrengthShips = 1120,         (no db data)
    AttrScanStrengthStructures = 1121,         (no db data)
    AttrMaxScanGroups = 1122,         (no db data)
    AttrScanDuration = 1123,     How long this probe has to scan until it can obtain results. (no db data)
    AttrScanFrequencyResult = 1161,             (no db data)
    AttrProbeCanScanShips = 1413,       boolean

    // these are char attribs
    AttrScanStrengthBonus = 846,        in %
    AttrMaxScanDeviationModifier = 1156,

    // for moon goo
    AttrScanRange = 765,
    AttrMinScanDeviation = 787,
    AttrMaxScanDeviation = 788,

    // new shit from rhea
    AttrScanStrengthBonusModule = 1907,
    */

ProbeSE::~ProbeSE() {
    if (m_scan != nullptr)
        m_scan->RemoveProbe(this);
}

bool ProbeSE::ProcessTic()
{
    if (m_lifeTimer.Check()) {
        Delete();
        delete(this);
        // delete from entity map
        return false;
    }
    if (m_returnTimer.Enabled())
        if (m_returnTimer.Check()) {
            m_returnTimer.Disable();
            _log(SCAN__INFO, "ProbeSE::Process() return timer hit for probeID %u", m_self->itemID());
            if (m_state == Probe::State::Warping)
                SendWarpEnd();
            RemoveProbe();
            m_self->MergeTypesInCargo(m_shipRef.get(), flagCargoHold);
            delete(this);
            // delete from entity map
            return false;
        }
    if (m_stateTimer.Enabled())
        if (m_stateTimer.Check()) {
            m_stateTimer.Disable();
            _log(SCAN__INFO, "ProbeSE::Process() state timer hit for probeID %u  state: %s", \
                    m_self->itemID(), GetStateName(m_state));
            if (IsMoving())
                m_self->SetPosition(m_destination);
            if (m_state == Probe::State::Warping)
                SendWarpEnd();
            SendStateChange(Probe::State::Idle);
        }
    return true;
}

bool ProbeSE::IsMoving()
{
    switch (m_state) {
        case Probe::State::Idle:
        case Probe::State::Inactive:
        case Probe::State::Scanning:
        case Probe::State::Waiting:
            return false;
        case Probe::State::Moving:
        case Probe::State::Warping:
        case Probe::State::Returning:
            return true;
    }
    return false;
}

void ProbeSE::UpdateProbe(ProbeData& data)
{
    m_scanRange = data.scanRange;
    m_rangeStep = data.rangeStep;
    m_destination = data.dest;

    float time(1), dist = GetPosition().distance(m_destination);
    if (dist < 100) {
        time = 0.5f;
    } else if (dist > BUBBLE_RADIUS_METERS){
        float wsm = m_self->GetAttribute(AttrWarpSpeedMultiplier).get_float() * (ONE_AU_IN_METERS /4);
        time = EvE::max(dist / wsm, 1);
        SendStateChange(Probe::State::Warping);
        SendWarpStart(time);
    } else {
        SendStateChange(Probe::State::Moving);
        uint32 mv = m_self->GetAttribute(AttrMaxVelocity).get_uint32();
        time = EvE::max(dist / mv, 1);
    }

    m_stateTimer.Start(time *1000);
    _log(SCAN__TRACE, "ProbeSE::UpdateProbe()  id:%u, state: %s, scanRange: %.2f, step: %u, dist:%.2f, time: %.2f", \
                GetID(), GetStateName(m_state), m_scanRange, m_rangeStep, dist, time );
}

void ProbeSE::RecoverProbe(PyList* list)
{
    if (m_client == nullptr)
        return;
    m_destination = m_shipRef->position() + 250;
    float time(1), dist = GetPosition().distance(m_destination);
    if (dist > BUBBLE_RADIUS_METERS){
        float wsm = m_self->GetAttribute(AttrWarpSpeedMultiplier).get_float() * (ONE_AU_IN_METERS /4);
        time = EvE::max(dist / wsm, 1);
        //SendWarpStart(time);
    } else {
        uint32 mv = m_self->GetAttribute(AttrMaxVelocity).get_uint32();
        time = EvE::max(dist / mv, 1);
    }

    /** @todo  verify probe status and controller before adding to "recover success list" */
    // add to list if still controlled by player
    list->AddItem(new PyInt(m_self->itemID()));
    m_returnTimer.Start(time * 1000);
    SendStateChange(Probe::State::Returning);
    _log(SCAN__TRACE, "ProbeSE::RecoverProbe()  Probe %u returning.  Return time is %.2fs", \
                GetID(), time);
}

void ProbeSE::SendNewProbe()
{
    ScanResultPos ssr_oed;
        ssr_oed.x = GetPosition().x;
        ssr_oed.y = GetPosition().y;
        ssr_oed.z = GetPosition().z;
    PyToken* token = new PyToken("foo.Vector3");
    PyTuple* oed_tuple = new PyTuple(2);
        oed_tuple->SetItem(0, token);
        oed_tuple->SetItem(1, ssr_oed.Encode());
    PyDict* newProbe = new PyDict();
        newProbe->SetItemString("probeID",      new PyLong(m_self->itemID()));
        newProbe->SetItemString("typeID",       new PyInt(m_self->typeID()));
        newProbe->SetItemString("scanRange",    new PyFloat(m_scanRange));
        newProbe->SetItemString("expiry",       new PyLong(m_expiry));
        newProbe->SetItemString("pos",          new PyObjectEx(false, oed_tuple));
    PyTuple* ev = new PyTuple(1);
        ev->SetItem(0, new PyObject("util.KeyVal", newProbe));
    m_client->SendNotification("OnNewProbe", "clientID", &ev);  // this is sequenced
}

void ProbeSE::SendStateChange(uint8 state)
{
    if (m_state == state)
        return;

    _log(SCAN__TRACE, "ProbeSE::SendStateChange()  Probe %u changing state to %s.", GetID(), GetStateName(state));
    m_state = state;
    if (m_client == nullptr)
        return;
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyLong(m_self->itemID()));
        tuple->SetItem(1, new PyInt(state));
    m_client->SendNotification("OnProbeStateChanged", "clientID", &tuple);  // this is sequenced
}

void ProbeSE::RemoveProbe()
{
    // remove from scan map, but check for abandoned probes (no scan)
    if (m_scan != nullptr) {
        m_scan->RemoveProbe(this);
        m_scan = nullptr;
    }
    // remove from system
    m_system->RemoveEntity(this);
    // set item loc to null
    m_self->SetPosition(NULL_ORIGIN);
    // remove from entity list
    sEntityList.RemoveProbe(m_self->itemID());

    if (m_client == nullptr)
        return;
    PyTuple* ev = new PyTuple(1);
        ev->SetItem(0, new PyLong(m_self->itemID()));
    m_client->SendNotification("OnRemoveProbe", "clientID", &ev);  // this is sequenced
}

void ProbeSE::SendWarpStart(float travelTime/*0*/)
{
    // remove from bubble
    m_bubble->Remove(this);

    PyToken* token = new PyToken("foo.Vector3");
    ScanResultPos posFrom;
        posFrom.x = m_self->position().x;
        posFrom.y = m_self->position().y;
        posFrom.z = m_self->position().z;
    ScanResultPos posTo;
        posTo.x = m_destination.x;
        posTo.y = m_destination.y;
        posTo.z = m_destination.z;
    PyTuple* from = new PyTuple(2);
        from->SetItem(0, token);
        from->SetItem(1, posFrom.Encode());
    PyTuple* to = new PyTuple(2);
        to->SetItem(0, token);
        to->SetItem(1, posTo.Encode());
    // OnProbeWarpStart(self, probeID, fromPos, toPos, startTime, duration)
    PyTuple* tuple = new PyTuple(5);
        tuple->SetItem(0, new PyLong(m_self->itemID()));    //probeID
        tuple->SetItem(1, new PyObjectEx(false, from));     //from
        tuple->SetItem(2, new PyObjectEx(false, to));       //to
        tuple->SetItem(3, new PyLong(GetFileTimeNow()));    //startTime
        tuple->SetItem(4, new PyFloat(travelTime));         //duration in ms
    m_client->SendNotification("OnProbeWarpStart", "clientID", &tuple);  // this is sequenced

    // not sure if this will work right...slimItem is removed when probe warps out
    //SendSlimChange();
}

void ProbeSE::SendWarpEnd()
{
    m_self->SetPosition(m_destination);
    sBubbleMgr.Add(this);
    if (m_client == nullptr)
        return;
    PyTuple* tuple = new PyTuple(1);
        tuple->SetItem(0, new PyLong(m_self->itemID()));
    m_client->SendNotification("OnProbeWarpEnd", "clientID", &tuple);  // this is sequenced
}

const char* ProbeSE::GetStateName(uint8 state)
{
    switch(state) {
        case Probe::State::Inactive:   return "Inactive";
        case Probe::State::Idle:       return "Idle";
        case Probe::State::Moving:     return "Moving";
        case Probe::State::Warping:    return "Warping";
        case Probe::State::Scanning:   return "Scanning";
        case Probe::State::Returning:  return "Returning";
    }

    return "Undefined";
}

float ProbeSE::GetDeviation()
{
    // % of current range based on bonuses
    return m_scanRange *m_scanDeviation;
}

float ProbeSE::GetScanStrength()
{
    // factor of range
    return m_scanStrength /pow(2, (m_rangeStep -1));
}

float ProbeSE::GetRangeModifier(float dist) {
    // e^-((targ rang / max range)^2)
    float tmp = pow(dist/(m_baseScanRange *m_rangeStep), m_rangeFactor);
    return log(tmp);
}

//SignatureFocusing?
bool ProbeSE::HasMaxSkill() {
    if (m_client->GetChar()->HasSkillTrainedToLevel(EvESkill::SensorLinking, 5))
        if (m_client->GetChar()->HasSkillTrainedToLevel(EvESkill::AstrometricAcquisition, 5))
            return true;
        return false;
}

PyDict* ProbeSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for ProbeSE %s(%u)", GetName(), m_self->itemID());
    PyDict* slim = new PyDict();
    slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
    slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",                  new PyInt(m_ownerID));
    slim->SetItemString("corpID",                   IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
    slim->SetItemString("allianceID",               IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
    slim->SetItemString("warFactionID",             IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    slim->SetItemString("numLaunchers",             PyStatic.NewOne());
    slim->SetItemString("sourceModuleID",           m_moduleRef.get() != nullptr? new PyInt(m_moduleRef->itemID()): PyStatic.NewNone());
    slim->SetItemString("securityStatus",           new PyFloat(m_secStatus));
    return slim;
}

void ProbeSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1.0;
    into.recharge = 1000000;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0;
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
}

void ProbeSE::SendSlimChange()
{
    PyDict* slim = new PyDict();
        slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",               new PyInt(m_self->categoryID()));
        slim->SetItemString("ownerID",                  new PyInt(m_ownerID));
        slim->SetItemString("corpID",                   IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",               IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",             IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("numLaunchers",             PyStatic.NewOne());
        slim->SetItemString("sourceModuleID",           m_moduleRef.get() != nullptr? new PyInt(m_moduleRef->itemID()):PyStatic.NewNone());
        slim->SetItemString("securityStatus",           new PyFloat(m_secStatus));
        slim->SetItemString("warpingAway",              m_state == Probe::State::Returning ? PyStatic.NewFalse() : PyStatic.NewTrue());    // this is sent when probe warps
    PyTuple* probeData = new PyTuple(2);
        probeData->SetItem(0, new PyLong(m_self->itemID()));
        probeData->SetItem(1, new PyObject("foo.SlimItem", slim));
    PyTuple* updates = new PyTuple(2);
        updates->SetItem(0, new PyString("OnSlimItemChange"));
        updates->SetItem(1, probeData);
    m_destiny->SendSingleDestinyUpdate(&updates, true);
}

/*
                      [PyString "OnSlimItemChange"]
                      [PyTuple 2 items]
                        [PyIntegerVar 9000000000000020440]
                        [PyObjectData Name: foo.SlimItem]
                          [PyDict 10 kvp]
                            [PyString "itemID"]
                            [PyIntegerVar 9000000000000020440]
                            [PyString "typeID"]
                            [PyInt 30028]
                            [PyString "allianceID"]
                            [PyNone]
                            [PyString "sourceModuleID"]
                            [PyIntegerVar 1002331157594]
                            [PyString "corpID"]
                            [PyInt 98038978]
                            [PyString "numLaunchers"]
                            [PyInt 1]
                            [PyString "warpingAway"]
                            [PyInt 1]
                            [PyString "warFactionID"]
                            [PyNone]
                            [PyString "securityStatus"]
                            [PyFloat 3.09141417479958]
                            [PyString "ownerID"]
                            [PyInt 1661059544]
                            */

 /**
  * @name Scan.cpp
  *   Scanning methods for EVEmu
  *
  * @Author:        Allan
  * @date:          7Dec15 (working)
  * @udpate:        12Mar18 (probes)
  *
  * @version:       0.42
  */

// w.i.p.

/* SCAN__ERROR
 * SCAN__WARNING
 * SCAN__MESSAGE
 * SCAN__DEBUG
 * SCAN__INFO
 * SCAN__TRACE
 * SCAN__DUMP
 * SCAN__RSPDUMP
 */

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "StatisticMgr.h"
#include "exploration/Scan.h"
#include "Probes.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

Scan::Scan(Client* pClient)
: m_client(pClient),
  m_system(pClient->SystemMgr())
{
    m_probeScan = false;
    m_probeMap.clear();
    m_activeProbeMap.clear();
}

void Scan::AddProbe(ProbeSE* pProbe)
{
    pProbe->SetScan(this);
    m_probeMap.emplace(pProbe->GetID(), pProbe);
}

void Scan::RemoveProbe(ProbeSE* pProbe)
{
    m_probeMap.erase(pProbe->GetID());
}

void Scan::ProcessScan(bool useProbe/*false*/)
{
    if (!useProbe) {
        ShipScanResult();
        return;
    }
    if (m_probeScan) {
        ProbeScanResult();
        m_probeScan = false;
        m_activeProbeMap.clear();
        return;
    }

    bool idle = true;
    uint16 ntime(0), duration = m_client->GetShip()->GetAttribute(AttrScanSpeed).get_uint32();
    if (duration < 1000)
        duration = 8000;    // 8s default probe scan time.
    for (auto cur : m_activeProbeMap) {
        if (cur.second->IsMoving()) {
            idle = false;
            ntime = cur.second->GetMoveTime();
            // this time isnt right (but very close).  need to really think it thru for correctness
            //  currrently, it will allow probes to hit 'idle' as scanning starts, or a few ms after.
            if (ntime > duration)       // flipped conditional
                duration = ntime;
        } else {
            cur.second->SendStateChange(Probe::State::Idle);
        }
    }
    if (idle) {
        m_probeScan = true;
        for (auto cur : m_activeProbeMap) {
            cur.second->SendStateChange(Probe::State::Scanning);
            cur.second->StartStateTimer(duration);
        }
        SystemScanStarted(duration);
    }
    _log(SCAN__TRACE, "ProcessScan() - probes - active:%u, total:%u, duration: %u, idle: %s", \
                m_activeProbeMap.size(), m_probeMap.size(), duration, idle?"true":"false");
    m_client->SetScanTimer(duration, true);
}


PyRep* Scan::ConeScan(Call_ConeScan args) {
    //  WORKING CODE...DONT FUCK WITH THIS!!  -allan 7Dec15

    // NOTE:  max distance is 14.4AU or maxInt (2417482647 in km)
    /* to find if a point is inside a cone.     (-allan 10Aug20)
     * U = unit vector along cone axis (sent from client, decoded as x,y,z)
     * VR = vector from cone vertex(V) to test point(R).
     * Uvr = unit vector of cone vertex to test point (normalized VR)
     * DP = Uvr dot U (dot product of Uvr and U).  this will give cosine of angle between VR and U
     * acDP = arc cosine of DP to give angle
     * test acDP < cone angle = point is inside cone.
     */
    bool test = false;
    float dot(0), acDP(0), angle(args.ScanAngle/2);
    std::vector<SystemEntity*> seVec;
    const GPoint vertex(m_client->GetShipSE()->GetPosition());
    const GPoint U(args.x, args.y, args.z);
    m_client->SystemMgr()->DScan(args.range, vertex, seVec);
    _log(SCAN__TRACE, "ConeScan() - query returned %u objects within range.  angle is %.3f", seVec.size(), angle);
    PyList* list = new PyList();
    for (auto cur : seVec ) {
        GVector VR(vertex, cur->GetPosition());
        VR.normalize();
        dot = U.dotProduct(VR);
        acDP = acos(dot);
        if (acDP < angle) {
            test = true;
            DirectionScanResult res;
            res.id         = cur->GetID();
            res.typeID     = cur->GetSelf()->typeID();
            res.groupID    = cur->GetSelf()->groupID();
            list->AddItem(res.Encode());
        }
        _log(SCAN__TRACE, "ConeScan() - tested %s(%u).  dot %.5f, acDP %.5f, result %s", cur->GetName(), cur->GetID(), dot, acDP, test?"true":"false");
        test = false;
    }

    return list;
}

void Scan::RequestScans(PyDict* dict) {
    uint16 duration = m_client->GetShip()->GetAttribute(AttrScanSpeed).get_uint32();
    if ((dict == nullptr) or dict->empty()) {
        _log(SCAN__MESSAGE, "Scan::RequestScans() called by %s in %s using ship scanner.", \
                m_client->GetName(), m_client->GetSystemName().c_str());

        OnSystemScanStarted ossst;
            ossst.timestamp = GetFileTimeNow();
            ossst.duration = duration;
            ossst.scanProbesDict = new PyDict();
        PyTuple* ev = ossst.Encode();
        m_client->SendNotification("OnSystemScanStarted", "charid", &ev);
        m_client->SetScanTimer(duration);
        return;
    }

    _log(SCAN__MESSAGE, "Scan::RequestScans() called by %s in %s using %u probes.",\
            m_client->GetName(), m_client->GetSystemName().c_str(), dict->size());

    uint32 probeID(0);
    PyDict::const_iterator cItr = dict->begin();
    for (; cItr != dict->end(); ++cItr) {
        // find probe in map....
        probeID = PyRep::IntegerValueU32(cItr->first);  // key
        std::map<uint32, ProbeSE*>::iterator pItr = m_probeMap.find(probeID);
        if (pItr == m_probeMap.end()) {
            _log(SCAN__ERROR, "Probe %u wasnt found in the probeMap for %s(%u)", \
                    probeID, m_client->GetName(), m_client->GetCharacterID());
            continue;  // make error here?
        }
        std::map<uint32, ProbeSE*>::iterator aItr = m_activeProbeMap.find(probeID);
        if (aItr == m_activeProbeMap.end())
            m_activeProbeMap[probeID] = pItr->second;

        Call_ProbeDataObj args;
        if (!args.Decode(cItr->second)) { // value
            _log(SERVICE__ERROR, "Scan::RequestScans::DecodeProbeData: Failed to decode arguments.");
            // make error here
            continue;
        }

        ProbeData data = ProbeData();
            data.state = args.state;    // do we need this?
            data.expiry = args.expiry;
            data.rangeStep = args.rangeStep;
            data.scanRange = args.scanRange;
        // set probe target
        PyObjectEx* obj = args.destination->AsObjectEx();
        PyTuple* dest = obj->header()->AsTuple()->GetItem(1)->AsTuple();
            data.dest.x = dest->GetItem(0)->AsFloat()->value();
            data.dest.y = dest->GetItem(1)->AsFloat()->value();
            data.dest.z = dest->GetItem(2)->AsFloat()->value();
        pItr->second->UpdateProbe(data);
    }

    // loop thru probe maps to see if any are disabled, then set status as 'inactive' to omit from tracking
    for (auto cur : m_probeMap) {
        std::map<uint32, ProbeSE*>::iterator itr = m_activeProbeMap.find(cur.first);
        if (itr == m_activeProbeMap.end())
            cur.second->SetState(Probe::State::Inactive);
    }

    m_client->SetScanTimer(duration, true);
}

void Scan::SystemScanStarted(uint16 duration)
{
    _log(SCAN__TRACE, "Scan::SystemScanStarted()  for %s in system %u", m_client->GetName(), m_client->GetSystemID());

    GPoint pos(NULL_ORIGIN);
    PyDict* probeDict = new PyDict();
    for (auto cur : m_activeProbeMap) {
        // probe data here...
        ScanProbesDict spd;
            spd.expiry = cur.second->GetExpiryTime();
            spd.maxDeviation = cur.second->GetDeviation();
            spd.probeID = cur.first;
            spd.state = cur.second->GetState();
            spd.rangeStep = cur.second->GetRangeStep();
            spd.scanRange = cur.second->GetScanRange();
            spd.scanStrength = cur.second->GetScanStrength();
            spd.typeID = cur.second->GetSelf()->typeID();
        pos = cur.second->GetPosition();
        ScanResultPos srp;
            srp.x = pos.x;
            srp.y = pos.y;
            srp.z = pos.z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, srp.Encode());
        spd.pos = new PyObjectEx(false, oed_tuple);  // oed goes here
        PyIncRef(spd.pos);
        spd.destination = spd.pos;
        probeDict->SetItem(new PyInt(cur.first), spd.Encode());
    }

    OnSystemScanStarted ossst;
        ossst.timestamp = GetFileTimeNow();
        ossst.duration = duration;
        ossst.scanProbesDict = probeDict;
    PyTuple* ev = ossst.Encode();
    ev->Dump(SCAN__RSPDUMP, "sss-    ");
    m_client->SendNotification("OnSystemScanStarted", "charid", &ev, false);
}

void Scan::ShipScanResult() {
    //  WORKING CODE...DONT FUCK WITH THIS!!  -allan 11Dec15
    /** @todo  see client code to verify what it expects, and what it can calculate */
    // client scan data found in EVE_Scanning.h
    std::vector<CosmicSignature> anom;
    if (m_client->IsShowall()) {
        m_system->GetAllEntities(anom);
        // bubble centers only populate when bubble markers are enabled.
        sBubbleMgr.GetBubbleCenterMarkers(m_system->GetID(), anom);
    } else {
        m_system->GetAnomMgr()->GetAnomalyList(anom);
    }

    PyList* resultList = new PyList();
    // NOTE. cannot scan pos, wrecks, ships, mission sites, or escalations.  they DO have sigIDs, and can get to type (25%), but no farther
    for (auto anoms : anom) {
        SystemScanResult ssr;
            ssr.typeID = anoms.sigTypeID;
            ssr.scanGroupID = anoms.scanGroupID;
            ssr.groupID = anoms.sigGroupID;
            ssr.strengthAttributeID = anoms.scanAttributeID;
            ssr.dungeonName = anoms.sigName;
            ssr.id = anoms.sigID;
            ssr.deviation = 0;     /* for scan probes */
            ssr.degraded = false;  /* dunno what this does.  have only seen 'false' in packets */
            ssr.probeID = new PyInt(m_client->GetShipID());
            ssr.certainty = anoms.sigStrength;
            ssr.pos = PyStatic.NewNone();
        ScanResultPos ssr_oed;
            ssr_oed.x = anoms.position.x;
            ssr_oed.y = anoms.position.y;
            ssr_oed.z = anoms.position.z;
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, new PyToken("foo.Vector3"));
            oed_tuple->SetItem(1, ssr_oed.Encode());
        ssr.data = new PyObjectEx(false, oed_tuple);  // oed goes here
        resultList->AddItem(ssr.Encode());
    }

    OnSystemScanStopped osss;
        osss.systemScanResult = resultList;
        // probeDict and absentTargets are both empty when using ship sensors to scan.
        osss.scanProbesDict = new PyDict();
        osss.absentTargets = new PyList();
    PyTuple* ev = osss.Encode();
    ev->Dump(SCAN__RSPDUMP, "ssr-    ");
    m_client->SendNotification("OnSystemScanStopped", "charid", &ev);
}

void Scan::ProbeScanResult()
{
    // this will use outline of above code, but be MUCH more complicated...
    _log(SCAN__TRACE, "Scan::ProbeScanResult()  for %s in system %u", m_client->GetName(), m_client->GetSystemID());

    PyList* resultList = new PyList();
    std::vector<CosmicSignature> sig, anom;

    // are anomalies shown in probe scan results?  config option?
    m_system->GetAnomMgr()->GetAnomalyList(anom);
    for (auto anoms : anom) {
        SystemScanResult ssr;
            ssr.typeID = anoms.sigTypeID;
            ssr.scanGroupID = anoms.scanGroupID;
            ssr.groupID = anoms.sigGroupID;
            ssr.strengthAttributeID = anoms.scanAttributeID;
            ssr.dungeonName = anoms.sigName;
            ssr.id = anoms.sigID;
            ssr.deviation = 0;     /* 0 for anomalies */
            ssr.degraded = false;
            ssr.probeID = new PyInt(m_client->GetShipID());
            ssr.certainty = anoms.sigStrength;
            ssr.pos = new PyToken("foo.Vector3"); //PyStatic.NewNone();
            /*  this is ship position first, then anomaly position
             *    [PyString "data"]
             *    [PyObjectEx Type2]
             *      [PyTuple 2 items]
             *        [PyTuple 1 items]
             *          [PyObjectEx Type2]
             *            [PyTuple 2 items]
             *              [PyTuple 1 items]
             *                [PyToken foo.Vector3]
             *              [PyTuple 3 items]
             *                [PyFloat 924428329391.783]
             *                [PyFloat 8194006649.63061]
             *                [PyFloat 165918840569.438]
             *        [PyTuple 3 items]
             *          [PyFloat 1231197245226]
             *          [PyFloat -146772910080]
             *          [PyFloat 111531171840]
             */
        ScanResultPos ssr_oed;
            ssr_oed.x = anoms.position.x;
            ssr_oed.y = anoms.position.y;
            ssr_oed.z = anoms.position.z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());
        ssr.data = new PyObjectEx(false, oed_tuple);  // oed goes here
        resultList->AddItem(ssr.Encode());
    }

    m_system->GetAnomMgr()->GetSignatureList(sig);
    for (auto sigs : sig) {
        SignalData data = SignalData();
            data.sig = sigs;
            data.probes = nullptr;
            data.probePos = nullptr;
        if (GetProbeDataForSig(data)) {
            SystemScanResult ssr;
                ssr.id = sigs.sigID;
                ssr.dungeonName = sigs.sigName;
                ssr.typeID = sigs.sigTypeID;
                ssr.groupID = sigs.sigGroupID;
                ssr.scanGroupID = sigs.scanGroupID;
                ssr.strengthAttributeID = sigs.scanAttributeID;
                ssr.degraded = false;
                ssr.deviation = data.deviation; //deviation is the distance between the scan result shown on the map and the actual location of your target.
                ssr.certainty = data.certainty; // this is listed as "signal strength" in scan window
                ssr.probeID = data.probes;
                ssr.pos = data.probePos;
            ScanResultPos ssr_oed;
                ssr_oed.x = data.sig.position.x;
                ssr_oed.y = data.sig.position.y;
                ssr_oed.z = data.sig.position.z;
            PyToken* token = new PyToken("foo.Vector3");
            PyTuple* oed_tuple = new PyTuple(2);
                oed_tuple->SetItem(0, token);
                oed_tuple->SetItem(1, ssr_oed.Encode());
            ssr.data = new PyObjectEx(false, oed_tuple);  // oed goes here
            resultList->AddItem(ssr.Encode());
        }
    }

    GPoint pos(NULL_ORIGIN);
    PyDict* probeDict = new PyDict();
    for (auto cur : m_activeProbeMap) {
        // probe data here...
        ScanProbesDict spd;
        spd.expiry = cur.second->GetExpiryTime();
        spd.maxDeviation = cur.second->GetDeviation();
        pos = cur.second->GetPosition();
        ScanResultPos ssr_oed;
            ssr_oed.x = pos.x;
            ssr_oed.y = pos.y;
            ssr_oed.z = pos.z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());
        spd.pos = new PyObjectEx(false, oed_tuple);  // oed goes here
        spd.destination = spd.pos;
        spd.probeID = cur.first;
        spd.state = cur.second->GetState();
        spd.rangeStep = cur.second->GetRangeStep();
        spd.scanRange = cur.second->GetScanRange();
        spd.scanStrength = cur.second->GetScanStrength();
        spd.typeID = cur.second->GetSelf()->typeID();
        probeDict->SetItem(new PyInt(cur.first), spd.Encode());
    }

    // this will be sigs that are no longer present in current scan range
    //  will have to keep previous list and compare with current list to populate this
    // may not be used.....testing
    PyList* absentList = new PyList();

    OnSystemScanStopped osssp;
        osssp.scanProbesDict = probeDict;
        osssp.systemScanResult = resultList;
        osssp.absentTargets = absentList;
    PyTuple* ev = osssp.Encode();
    ev->Dump(SCAN__RSPDUMP, "psr-    ");
    m_client->SendNotification("OnSystemScanStopped", "charid", &ev);
}


/*  probeID is for those probes that pick up this signal.
 * pos is probe position
 *    will have to call a method to determine sig type, probe data, and signal specifics for each signal
 */
bool Scan::GetProbeDataForSig(SignalData& data)
{
    /*  this will determine sig position vs probe position, range, strength
     * to decide if this sig is picked up by a probe, and return probe data
     * based on that.
     *
struct CosmicSignature {
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    GPoint position;
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
};
     */

    bool hit(false);
    float dist(0);
    std::vector<ProbeSE*> probeVec;
    // check probe scan ranges for signals; verify probe can scan signal
    for (auto cur : m_activeProbeMap) {
        // reset ring/sphere checks
        cur.second->SetRing(false);
        cur.second->SetSphere(false);
        switch (data.sig.scanGroupID) {
            case Scanning::Group::Ship:
            case Scanning::Group::DroneOrProbe:
            case Scanning::Group::Structure: {
                if (cur.second->CanScanShips()) {
                    dist = cur.second->GetPosition().distance(data.sig.position);
                    if (cur.second->GetScanRange() > dist) {
                        hit = true;
                        probeVec.push_back(cur.second);
                    }
                    _log(SCAN__DEBUG, "Scan::GetProbeDataForSig()  scan range for probe %u: %.2fAU, distance to signal '%s' -> %.2fAU - %s",\
                            cur.first, cur.second->GetScanRange() /ONE_AU_IN_METERS, data.sig.sigName.c_str(), \
                            dist /ONE_AU_IN_METERS, hit?"hit":"miss");
                } else {
                    _log(SCAN__TRACE, "Scan::GetProbeDataForSig()  probe %u cannot scan signal %s", cur.first, data.sig.sigName.c_str());
                }
            } break;
            case Scanning::Group::Signature: {
                switch (data.sig.dungeonType) {
                    case Dungeon::Type::Mission: // npc mission
                    case Dungeon::Type::Escalation://  new dungeon from previous site. very limited access
                    case Dungeon::Type::Rated: { // DED rated dungeon
                        _log(SCAN__TRACE, "Scan::GetProbeDataForSig()  probe %u cannot scan signal %s", cur.first, data.sig.sigName.c_str());
                        continue;
                    }
                /*  nothing to check for on these...
                    case Dungeon::Type::Gravimetric:// roids
                    case Dungeon::Type::Magnetometric:// salvage and archeology
                    case Dungeon::Type::Radar:// hacking
                    case Dungeon::Type::Ladar: // gas mining
                    case Dungeon::Type::Wormhole:
                    case Dungeon::Type::Anomaly:// non-rated dungeon that isnt required to scan with probes - will not hit here.
                    case Dungeon::Type::Unrated:// non-rated dungeon  no waves, possible escalation to complex
                        break;
                */
                }
            } // this probe can scan this signal; fall thru to add probe to signal's map
            default: {
                dist = cur.second->GetPosition().distance(data.sig.position);
                if (cur.second->GetScanRange() > dist) {
                    hit = true;
                    probeVec.push_back(cur.second);
                }
                _log(SCAN__DEBUG, "Scan::GetProbeDataForSig()  scan range for probe %u: %.2fAU, distance to signal '%s' -> %.2fAU - %s",\
                        cur.first, cur.second->GetScanRange() /ONE_AU_IN_METERS, data.sig.sigName.c_str(), \
                        dist /ONE_AU_IN_METERS, hit?"hit":"miss");
            } break;
        }
        hit = false;
    }

    _log(SCAN__TRACE, "Scan::GetProbeDataForSig()  probeVec size: %u for signal %s (%s)", \
            probeVec.size(), data.sig.sigName.c_str(), m_system->GetAnomMgr()->GetScanGroupName(data.sig.scanGroupID));

    if (probeVec.empty())
        return false;

    // at this point, we have at least one probe scanning at least one signal
    GetSignalData(data, probeVec);
    if (probeVec.size() > 1) {
        bool isRing(false);
        uint8 count(0);
        GPoint pos(NULL_ORIGIN);
        PyList* list = new PyList();
        PyList* ring = new PyList();
        PyTuple* tuple = new PyTuple(probeVec.size());
        for (auto cur : probeVec) {
            tuple->SetItem(count++, new PyInt(cur->GetID()));
            pos = cur->GetPosition();
            ScanResultPos ssr_oed;
                ssr_oed.x = pos.x;
                ssr_oed.y = pos.y;
                ssr_oed.z = pos.z;
            PyToken* token = new PyToken("foo.Vector3");
            PyTuple* oed_tuple = new PyTuple(2);
                oed_tuple->SetItem(0, token);
                oed_tuple->SetItem(1, ssr_oed.Encode());
            list->AddItem(new PyObjectEx(false, oed_tuple));
            if (cur->IsRing()) {
                isRing = true;
                ring->AddItem(new PyObjectEx(false, oed_tuple));
            }
        }
        if (isRing)
            list->AddItem(ring);
        data.probes = tuple;
        data.probePos = list;
        // there is *something* here where one of the positions is given as a nested list of objects
        /*
                      [PyString "pos"]
                      [PyList 6 items]
                        [PyObjectEx Type2]
                          [PyTuple 2 items]
                            [PyTuple 1 items]
                              [PyToken foo.Vector3]
                            [PyTuple 3 items]
                              [PyFloat 506169425920]
                              [PyFloat 948782891008]
                              [PyFloat 154119258112]
                        [PyList 3 items]
                          [PyObjectEx Type2]
                            [PyTuple 2 items]
                              [PyTuple 1 items]
                                [PyToken foo.Vector3]
                              [PyTuple 3 items]
                                [PyFloat 506169425920]
                                [PyFloat -555071897600]
                                [PyFloat 1636387389440]
                */
    } else {
        ScanResultPos ssr_oed;
            ssr_oed.x = probeVec.at(0)->GetPosition().x;
            ssr_oed.y = probeVec.at(0)->GetPosition().y;
            ssr_oed.z = probeVec.at(0)->GetPosition().z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());
        data.probes = new PyInt(probeVec.at(0)->GetID());
        if (probeVec.at(0)->IsSphere())
            ; // placeholder.  no clue how to do this yet
        data.probePos = new PyObjectEx(false, oed_tuple);
    }
    return true;
}

void Scan::GetSignalData(SignalData& data, std::vector<ProbeSE*>& probeVec)
{
    // probeVec has only probes than are in range of a signal it can scan
    uint8 probeCount = probeVec.size();
    float probeMultiplier(0.0f);
    switch(probeCount) {
        //  new style...already calculated (in python) for 1 to 8 probes
        case 1: probeMultiplier = 0.25774312594204907; break;
        case 2: probeMultiplier = 0.5130245854773758; break;
        case 3: probeMultiplier = 0.7234132613571191; break;
        case 4: probeMultiplier = 0.8824741410676007; break;
        case 5: probeMultiplier = 0.9963325352118082; break;
        case 6: probeMultiplier = 1.0754155621393995; break;
        case 7: probeMultiplier = 1.1296251734489133; break;
        case 8: probeMultiplier = 1.1666968137637062; break;
    }

    GPoint point(data.sig.position);
    data.deviation = 0;
    float scanStr1(0), rangeMod1(0), dist1(0), scanStr2(0), rangeMod2(0), dist2(0), angleMod(0);
    if (probeCount == 1) {
        dist1 = probeVec.at(0)->GetPosition().distance(point);
        rangeMod1 = probeVec.at(0)->GetRangeModifier(dist1);
        scanStr1 = probeVec.at(0)->GetScanStrength();
        data.deviation = probeVec.at(0)->GetDeviation() *1.3;  // fudge a bit for single probe
        data.certainty = data.sig.sigStrength * (scanStr1 / rangeMod1) / 2;
        _log(SCAN__TRACE, "Scan::GetSignalData(1)  dist: %.3fAU, rangeMod: %.5f, scanStr: %.5f", \
                dist1 / ONE_AU_IN_METERS, rangeMod1, scanStr1);
    } else {
        /*  loop thru probes and get range mods and sigStrength for each.
         *  combine all probe's data to get good sum based on probe range and strength
         */
        int8 count(0), max(2);
        if (probeVec.at(0)->HasMaxSkill())
            max = 3;
        std::map<float, std::pair<ProbeSE*, ProbeSE*>> angleMap;     // angle, <probeSE1,probeSE2>
        CalcProbeAngles(point, probeVec, angleMap);    //determine probe angles to target
        float probeSig1(0), probeSig2(0);
        // reverse-iterate to use highest values first
        std::map<float, std::pair<ProbeSE*, ProbeSE*>>::reverse_iterator itr = angleMap.rbegin(), end = angleMap.rend();
        for (; itr != end; ++itr) {
            // we are using top 3 angle pairs of 2 probes/angle, which gives 6 total scans for str calc.
            // if player has signal acquisition and sensor linking both at l5, this will allow another pair of scan results
            if (count > max)
                break;
            angleMod = sin(itr->first / 2);      // get value between 0 and 1 (fuzzy logic as angle modifier)
            // get sigStr from first probe
            dist1 = itr->second.first->GetPosition().distance(point);
            rangeMod1 = itr->second.first->GetRangeModifier(dist1);
            scanStr1 = itr->second.first->GetScanStrength();
            probeSig1 = data.sig.sigStrength * angleMod * probeMultiplier * (scanStr1 / rangeMod1);
            data.certainty += probeSig1;
            // get sigStr from second probe
            dist2 = itr->second.second->GetPosition().distance(point);
            rangeMod2 = itr->second.second->GetRangeModifier(dist2);
            scanStr2 = itr->second.second->GetScanStrength();
            probeSig2 = data.sig.sigStrength * angleMod * probeMultiplier * (scanStr2 / rangeMod2);
            data.certainty += probeSig2;
            // get deviation from both probes
            data.deviation += itr->second.first->GetDeviation();
            data.deviation += itr->second.second->GetDeviation();
            _log(SCAN__TRACE, "Scan::GetSignalData(%u)  #%u - angle %.3f (P1 %u, P2 %u) - dist: %.3fAU, %.3fAU, rangeMod: %.5f, %.5f, scanStr: %.5f, %.5f, angleMod: %.4f, multiplier: %.5f, probeSig: %.5f, %.5f", \
                    probeCount, ++count, EvE::Trig::Rad2Deg(itr->first), itr->second.first->GetID(), itr->second.second->GetID(), dist1 /ONE_AU_IN_METERS, dist2 /ONE_AU_IN_METERS,\
                    rangeMod1, rangeMod2, scanStr1, scanStr2, angleMod, probeMultiplier, probeSig1, probeSig2);
        }
        // get average deviation from all probes
        data.deviation /= count *2;
    }
/*Results, and What They Mean

    Sphere: The result is somewhere in the sphere. One probe only has a hit and approximate range. You can get multiple probes with independent hits, it can be messy. This is the least accurate hit.
    Circle: Two probes have approximate distances. More accurate, and likely to yield better hits with more probes. Target anomaly is located near the circle.
    Two Dots: Three probes have distances, which narrows it down to between two possible locations. Likely to be more or less accurate, but beware of deviation. Target anomaly has been narrowed down to being near one of the dots.
    One Dot: Four or more probes can see the result. Once scan strength reaches 100%, the result will be warpable.
*/
    if ((data.certainty < 0.05) and (probeCount == 1)) { // set sphere
        probeVec.at(0)->SetSphere(true);
    } else if ((data.certainty < 0.08) and (probeCount == 2) ) {
        // set ring
        probeVec.at(0)->SetRing(true);
        probeVec.at(1)->SetRing(true);
    } else if (data.certainty > 0.99) {
        sStatMgr.Increment(Stat::sitesScanned);
    }

    // adjust deviation based on signal strength
    /*
    if (data.certainty < 0.8f)
        data.deviation *= (1 + (1 - data.certainty));
    else
        data.deviation *= (1 - data.certainty);
    */

    /** @todo make sure this 'reported' position is within probe boundary */
    //point.MakeRandomPointOnSphereLayer(data.deviation /2, data.deviation);
    data.sig.position = point;

    _log(SCAN__TRACE, "Scan::GetSignalData() - certainty for signal %s(%s) is %.5f (sigStrength:%.5f) \n Deviation: %.0fm (%.3f AU)", \
            data.sig.sigName.c_str(), data.sig.sigID.c_str(), data.certainty, data.sig.sigStrength, data.deviation, (data.deviation / ONE_AU_IN_METERS));
}

void Scan::CalcProbeAngles(GPoint& sigPos, std::vector<ProbeSE*>& probeVec, std::map<float, std::pair<ProbeSE*, ProbeSE*>>& angleMap ) {
    bool run(true);
    uint8 count(probeVec.size()), x(0), y(1);
    float dot(0.0f), angle(0.0f);
    ProbeSE* p1(nullptr);
    while (x < count) {
        p1 = probeVec.at(x++);
        if (p1 == nullptr)
            continue;
        GVector v1(p1->GetPosition(), sigPos);
        v1.normalize();
        for (auto cur : probeVec) {
            if ((cur == nullptr) or (p1 == cur))
                continue;
            GVector v2(cur->GetPosition(), sigPos);
            v2.normalize();
            dot = v1.dotProduct(v2);        // cosine of angle between v1,v2
            angle = acos(dot);              // angle in rad
            // if this probe set is already inserted, skip and move along.
            if (angleMap.find(angle) != angleMap.end())
                continue;
            // add to map
            angleMap[angle] = std::make_pair(p1, cur);
            _log(SCAN__DEBUG, "Scan::CalcProbeAngles() - adding angle %.3f, p1 %u, p2 %u to map",\
                    /*EvE::Trig::Rad2Deg*/(angle), p1->GetID(), cur->GetID());
        }
    }
}

//  *********  apoc or before.....
/*  probe formulas....
 *
 * single...
 * sp = sb*(1+0.05(a+ar))*(1+l)*(1+h)*pi(1+p*Fi))
 * sb is probe base str
 * a/ar is astrometrics and rangefinding
 * l is launcher bonus
 * h is hull bonus
 * fi is fit bonuses
 * p is stack penalty
 *
 * s3 is effective sig str.  ease with which a given target can be scanned
 * ship/struct sig str is ratio of sig radius to effective sig str of probe
 *
 * nr is probe scan radius.  distance from probe to target with radius of probe.
 * usually given in AU, but could be steps (nr) above min radius (rb = 0.25 for core, 0.5 for combat) where
 *      r = rb*2^nr-1
 *
 *
 * multiple....
 * str = (probe average)*2*(a1+a2+a3+a4)/360;
 * avg is from best 4 probes (within range)
 * a1,a2,a3,a4 = angles of best 4 probes (others disregarded)
 */

// post from 2013
/* probe formula...incomplete and some functions i cant do, so we're using apoc
 *
 * formula steps
 * 1) eval sig str of each probe
 * 2) eval angles of probe pairs to target
 * 3) eval total sig str
 *
 * 1 - eval sig str for probe
 * base sig str = size * probe str * distancMod /4
 * sig str = f(base sig str) + g(f(base sig str), position)
 *
 * size = targ sig radius
 * probe str = probe base str * bonus mods / range mods
 * dist mod = e^-((targ range/max range)^2)
 * pos = 3d coords of probe relative to target
 * f() = asymptotic function where f(x)=x below 25%, then diminishing returns to f(x)=75% for x=infinity
 * g(x,y) = returns percentage of x, between 1.5 and 3.5 (or 1 and 4)
 *
 * 2 - eval angles of probe pairs
 * compute angle between targ and each pair of probes
 * angleMod(a,b) = beta(angle(a,b),x,y)
 * a = probe 1 (arbitrary)
 * b = probe 2 (arbitrary)
 * beta = regularized beta function (found in boost as `ibeta(a,b,x)`)
 * angle = actual angle between a and b that intersects the signal
 * x, y = ratio between str of probe a, probe b and strongest probe
 *
 * 3 - eval total sig str
 * final = sigstrP1 + sigstrP2*(angleMod(p2,p1) + sigstrP3*(anglemod(P3,P1)*anglemod(p3,p2)) ... [to max of 8 probes?]
 */


// unscannable ship is (sigRad/sensorStr < 1.08)

/* center of tetrahedron formula
 * (((x1+x2+x3+x4) /4), ((y1+y2+y3+y4) /4), ((z1+z2+z3+z4) /4))
 */
/*
    One probe will only tell you if something is in range of the probe.
This will generate a red sphere inside your probe’s bubble that is centered on your probe.
The sphere gives you a general idea of how far the target is from your probe.
    Two probes will tell you that something exists on an imaginary ring which shows on what plane and the general area where your target is.
    Three probes will produce two possible locations for your target.
These are still not warpable and will usually show up as just one small circle on your map screen but there will be two entries under your scan results screen with the same ID#.
    Four or more probes will give you a single location shown by a red (less than 50% strength), yellow (50-99%), or green (100% hit).
By maneuvering your probes and decreasing their range they will eventually give a warpable result when the signal strength reaches 100% and turns green.
*/


// these are for moon scanning
void Scan::ScanStart()
{

}

void Scan::SurveyScan() {
    /*
     *
     *                      [PyString "OnSpecialFX"]
     *                      [PyTuple 14 items]
     *                        [PyIntegerVar 1002331681462]
     *                        [PyIntegerVar 1002332233248]
     *                        [PyInt 444]
     *                        [PyNone]
     *                        [PyNone]
     *                        [PyList 0 items]
     *                        [PyString "effects.SurveyScan"]
     *                        [PyBool False]
     *                        [PyInt 1]
     *                        [PyInt 1]
     *                        [PyFloat 4250]
     *                        [PyInt 0]
     *                        [PyIntegerVar 129509430135552798]
     *                        [PyNone]
     *
     */
}

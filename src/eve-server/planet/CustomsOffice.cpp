
/**
 * @name CustomsOffice.h
 *   Class for Customs Offices.
 *
 * @Author:         Allan
 * @date:   14 July 2019
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
#include "StaticDataMgr.h"
#include "manufacturing/Blueprint.h"
#include "map/MapDB.h"
#include "math/Trig.h"
#include "packets/Planet.h"
#include "planet/CustomsOffice.h"
#include "planet/Planet.h"
#include "pos/Structure.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"


CustomsSE::CustomsSE(StructureItemRef sRef, EVEServiceManager &services, SystemManager* system, const FactionData& data)
: ObjectSystemEntity(sRef, services, system),
m_system(system)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    // zero-init data
    m_cData = EVEPOS::CustomsData();
    m_oData = EVEPOS::OrbitalData();

    m_oData.planetID = atoi(m_self->customInfo().c_str());
    m_planetSE = m_system->GetPlanet(m_oData.planetID)->GetPlanetSE();

    sRef->SetMySE(this);

    _log(SE__DEBUG, "Created CustomsSE for item %s (%u).", sRef->name(), sRef->itemID());
}

void CustomsSE::Init()
{
    // load data.
    m_cData.itemID = m_self->itemID();

    // pull saved data from db
    if (!m_db.GetCustomsData(m_cData, m_oData)) {
        _log(POS__MESSAGE, "CustomsSE::Init() - %s(%u) has no saved data.  Initializing default set.", m_self->name(), m_cData.itemID);
        InitData();
        m_db.SaveCustomsData(m_cData, m_oData);
    }

    m_self->SetAttribute(AttrIsGlobal, EvilOne, false);

    // this should be based on state/status
    m_self->SetFlag(flagStructureActive);
}

void CustomsSE::InitData()
{
    // init all data.
    m_cData.state = EVEPOS::EntityState::Anchored;  // allow corp settings menu
    m_cData.timestamp = 0;
    m_cData.status = EVEPOS::StructureState::Online;
    m_cData.allowAlliance = false;
    m_cData.allowStandings  = false;
    m_cData.ownerID = m_self->ownerID();
    m_cData.selectedHour = 0;
    m_cData.standingValue = EVEPOS::StandingValues::StandingNeutral;
    m_oData.level = 1;   //{1-CUSTOMSOFFICE_SPACEPORT, 2-CUSTOMSOFFICE_SPACEELEVATOR}   this is for display model
    m_oData.standingOwnerID = m_cData.ownerID;

    std::map<uint8, float> taxRateValues;
        taxRateValues[EVEPOS::TaxValues::Corp]              = 0.05f;
        taxRateValues[EVEPOS::TaxValues::Alliance]          = 0.07f;
        taxRateValues[EVEPOS::TaxValues::StandingHorrible]  = 0.20f;
        taxRateValues[EVEPOS::TaxValues::StandingBad]       = 0.10f;
        taxRateValues[EVEPOS::TaxValues::StandingNeutral]   = 0.08f;
        taxRateValues[EVEPOS::TaxValues::StandingGood]      = 0.05f;
        taxRateValues[EVEPOS::TaxValues::StandingHigh]      = 0.02f;
    m_cData.taxRateValues = taxRateValues;
/*positionRowHeader = blue.DBRowDescriptor((('itemID', const.DBTYPE_I8),
         ('x', const.DBTYPE_R5),
         ('y', const.DBTYPE_R5),
         ('z', const.DBTYPE_R5),
         ('yaw', const.DBTYPE_R4),
         ('pitch', const.DBTYPE_R4),
         ('roll', const.DBTYPE_R4)))
         */
    // yaw, pitch, roll = dunRotation
    /*  yaw is rotation on y axis [-180/180]
     *   +yaw is counterclockwise (+x starting at z 0)
     * pitch is rotation on x axis [-90/90]
     *   +pitch is downward (-y from 0)
     * roll is rotation on z axis  [-180/180]
     *  +roll is counterclockwise from y 0
     */
    GPoint pos(m_self->position());
    GPoint targ(m_planetSE->GetPosition());
    float z = targ.z - pos.z;   // rise on z axis
    float x = targ.x - pos.x;    // run on x axis
    float y = targ.y - pos.y;   // rise on y axis
    float yaw = atan2(x, z);  // rad from position to target on xz plane
    float hyp = sqrt(pow(z, 2) + pow(x, 2));   // run on y plane
    float pitch = atan2(y, hyp);  // rad from position to target on hy plane

    // verify quadrant and set rotation accordingly
    /*  quadrant signs and corrections
     *      I =  -x,+z      x-90
     *      II =  +x,+z     90-x
     *      III = +x,-z     x+90
     *      IV = -x,-z      x+180
     */
/*
    if (z < 0) {
        if (x < 0)
            yaw += EvE::Trig::Deg2Rad(180);
        else
            yaw += EvE::Trig::Deg2Rad(90);
    } else {
        if (x < 0)
            yaw -= EvE::Trig::Deg2Rad(90);
        else
            yaw = EvE::Trig::Deg2Rad(90) - yaw;
    }
*/
    if (y > 0)
        pitch *= -1;

    /*
     *
     *   Real: 15.280822
     *
     *
     *
     *
     *
     */

    // convert from radians to degrees
    m_oData.rotation.x = EvE::Trig::Rad2Deg(yaw);
    m_oData.rotation.y = EvE::Trig::Rad2Deg(pitch);
    m_oData.rotation.z = 0;
    //m_oData.rotation = m_self->position().rotationTo(m_planetSE->GetPosition());

    _log(POS__MESSAGE, "CustomsSE::InitData() - Yaw: rise: %0.5f, run: %0.5f, angle: %0.5f (%0.3f)", z, x, yaw, m_oData.rotation.x);
    _log(POS__MESSAGE, "CustomsSE::InitData() - Pitch: rise: %0.5f, run: %0.5f, angle: %0.5f (%0.3f)", y, hyp, pitch, m_oData.rotation.y);
}

void CustomsSE::Process() {
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
}

PyRep* CustomsSE::GetSettingsInfo()
{   //self.selectedHour, self.taxRateValues, self.standingValue, self.allowAlliance, self.allowStandings = self.orbitalData
    /*
        self.taxRates = [util.KeyVal(key='corporation'),
         util.KeyVal(key='alliance'),
         util.KeyVal(key='standingHorrible', standing=const.contactHorribleStanding),
         util.KeyVal(key='standingBad', standing=const.contactBadStanding),
         util.KeyVal(key='standingNeutral', standing=const.contactNeutralStanding),
         util.KeyVal(key='standingGood', standing=const.contactGoodStanding),
         util.KeyVal(key='standingHigh', standing=const.contactHighStanding)]
         */
    PyDict* dict = new PyDict();
        dict->SetItemString("corporation", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::Corp]));
        dict->SetItemString("alliance", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::Alliance]));
        dict->SetItemString("standingHorrible", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::StandingHorrible]));
        dict->SetItemString("standingBad", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::StandingBad]));
        dict->SetItemString("standingNeutral", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::StandingNeutral]));
        dict->SetItemString("standingGood", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::StandingGood]));
        dict->SetItemString("standingHigh", new PyFloat(m_cData.taxRateValues[EVEPOS::TaxValues::StandingHigh]));
    PyTuple* tuple = new PyTuple(5);
        tuple->SetItem(0, new PyInt(m_cData.selectedHour));
        tuple->SetItem(1, new PyObject("util.KeyVal", dict));
        tuple->SetItem(2, new PyInt(m_cData.standingValue));    //const.contactHorribleStanding
        tuple->SetItem(3, new PyBool(m_cData.allowAlliance));
        tuple->SetItem(4, new PyBool(m_cData.allowStandings));
    return tuple;
}

void CustomsSE::UpdateSettings(int8 selectedHour, int8 standingValue, bool ally, bool standings, Call_TaxRateValuesDict& taxRateValues)
{
    m_cData.allowAlliance    = ally;
    m_cData.allowStandings   = standings;
    m_cData.selectedHour     = selectedHour;
    m_cData.standingValue    = standingValue;

    using namespace EVEPOS;
    m_cData.taxRateValues[TaxValues::Corp]              = taxRateValues.corporation;
    m_cData.taxRateValues[TaxValues::Alliance]          = taxRateValues.alliance;
    m_cData.taxRateValues[TaxValues::StandingHorrible]  = taxRateValues.standingHorrible;
    m_cData.taxRateValues[TaxValues::StandingBad]       = taxRateValues.standingBad;
    m_cData.taxRateValues[TaxValues::StandingNeutral]   = taxRateValues.standingNeutral;
    m_cData.taxRateValues[TaxValues::StandingGood]      = taxRateValues.standingGood;
    m_cData.taxRateValues[TaxValues::StandingHigh]      = taxRateValues.standingHigh;

    // save updates to db
    m_db.UpdateCustomsData(m_cData, m_oData);
}

float CustomsSE::GetTaxRate(Client* pClient)
{
    // get current tax rate based on set values by owning corp

    return m_cData.taxRateValues[EVEPOS::TaxValues::Corp];
}

void CustomsSE::VerifyAddItem(InventoryItemRef iRef)
{
    // test for planetary resources here
    if ((iRef->categoryID() != EVEDB::invCategories::PlanetaryResources)
    and (iRef->categoryID() != EVEDB::invCategories::PlanetaryCommodities))
        throw CustomError ("You cannot put %s in a %s", iRef->name(), m_self->name());
}

void CustomsSE::GetEffectState(PyList& into)
{
    // not used yet - cannot use OnSpecialFX14 packet.
}

void CustomsSE::SendEffectUpdate(int16 effectID, bool active)
{
    GodmaEnvironment ge;
        ge.selfID = m_cData.itemID;
        ge.charID = m_ownerID;
        ge.shipID = m_cData.itemID;
        ge.target = PyStatic.NewNone();
        ge.subLoc = PyStatic.NewNone();
        ge.area = new PyList();
        ge.effectID = effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = effectID;
        shipEff.timeNow = GetFileTimeNow();
        shipEff.start = (active ? 1 : 0);
        shipEff.active = (active ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;    // do we need to adjust this?
        shipEff.duration = (active ? 0 : -1);
        shipEff.repeat = (active ? 1 : 0);
        shipEff.error = PyStatic.NewNone();
    PyList* events = new PyList();
        events->AddItem(shipEff.Encode());
    PyTuple* event = new PyTuple(1);
        event->SetItem(0, events);
    m_destiny->SendSingleDestinyEvent(&event);   // consumed
}

void CustomsSE::SendSlimUpdate()
{
    PyDict *slim = new PyDict();
        slim->SetItemString("name",                     new PyString(m_self->itemName()));
        slim->SetItemString("itemID",                   new PyLong(m_cData.itemID));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_ownerID));
        slim->SetItemString("corpID",                   IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",               IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",             IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("posTimestamp",             new PyLong(m_cData.timestamp));
        slim->SetItemString("posState",                 new PyInt(m_cData.state));
        slim->SetItemString("incapacitated",            PyStatic.NewZero());
        slim->SetItemString("posDelayTime",             PyStatic.NewZero()); // fix this
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0,                            new PyLong(m_cData.itemID));
        shipData->SetItem(1,                            new PyObject("foo.SlimItem", slim));
    PyTuple* sItem = new PyTuple(2);
        sItem->SetItem(0,                               new PyString("OnSlimItemChange"));
        sItem->SetItem(1,                               shipData);
    m_destiny->SendSingleDestinyUpdate(&sItem);   // consumed
}


void CustomsSE::SetAnchor(Client* pClient, GPoint& pos)
{
    if (m_cData.status > EVEPOS::StructureState::Unanchored) {
        pClient->SendErrorMsg("The %s is already anchored", m_self->name());
        return;  // make error here?
    }

    m_destiny->SetPosition(pos);
    sBubbleMgr.Add(this);

    if (is_log_enabled(POS__TRACE))
        _log(POS__TRACE, "CustomsSE::Anchor() - TowerSE %s(%u) new position %.2f, %.2f, %.2f", GetName(), m_cData.itemID, pos.x, pos.y, pos.z);

    m_destiny->SetPosition(pos);
    m_self->SaveItem();

    m_cData.state = EVEPOS::EntityState::Anchoring;
    m_cData.status = EVEPOS::StructureState::Anchored;
    //m_delayTime = m_self->GetAttribute(AttrAnchoringDelay).get_int();
    //m_procTimer.SetTimer(m_delayTime);
    m_cData.timestamp = GetFileTimeNow();

    SendSlimUpdate();

    std::vector<PyTuple*> updates;
    SetBallFree sbf;
        sbf.entityID = m_self->itemID();
        sbf.is_free = false;
    updates.push_back(sbf.Encode());
    SetBallRadius sbr;
        sbr.entityID = m_self->itemID();
        sbr.radius = m_self->radius();
    updates.push_back(sbr.Encode());
    m_destiny->SendDestinyUpdate(updates); //consumed

    //SendEffectUpdate(anchorDropForOrbitals, true);
    m_destiny->SendSpecialEffect(m_cData.itemID, m_cData.itemID, m_self->typeID(),0,0,"effects.AnchorDrop",0,1,1,-1,0);
}

void CustomsSE::PullAnchor()
{
    if (m_cData.status > EVEPOS::StructureState::Anchored)
        return;  // make error here?

    m_cData.state = EVEPOS::EntityState::Unanchoring;
    m_cData.status = EVEPOS::StructureState::Unanchored;
    //m_delayTime = m_self->GetAttribute(AttrUnanchoringDelay).get_int();
    //m_procTimer.SetTimer(m_delayTime);
    m_cData.timestamp = GetFileTimeNow();

    SendSlimUpdate();

    //SendEffectUpdate(anchorLiftForOrbitals, true);
    m_destiny->SendSpecialEffect(m_cData.itemID, m_cData.itemID, m_self->typeID(),0,0,"effects.AnchorLift",0,1,1,-1,0);
}

void CustomsSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    //const uint16 miniballsCount = GetMiniBalls();
    BallHeader head = BallHeader();
        head.entityID = m_cData.itemID;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.mode = Ball::Mode::RIGID;
        head.flags = Ball::Flag::IsGlobal /*| Ball::Flag::IsMassive | HasMiniBalls*/;
    into.Append( head );

    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    /* TODO  query and configure miniballs for entity
     * NOTE  MiniBalls are BROKEN!!!  DO NOT USE!
     *    into.Append( miniballsCount );
     *    MiniBall miniball;
     *    for (int16 i; i<miniballsCount; i++) {
     *        miniball.x = -7701.181;
     *        miniball.y = 8060.06;
     *        miniball.z = 27878.900;
     *        miniball.radius = 1639.241;
     *        into.Append( miniball );
     *        miniball.clear();
     *    }
[MiniBall]
[Radius: 963.8593]
[Offset: (0, -2302, 1)]
[MiniBall]
[Radius: 1166.27]
[Offset: (0, 1298, 1)]
[MiniBall]
[Radius: 876.2357]
[Offset: (0, -502, 1)]
[MiniBall]
[Radius: 796.5781]
[Offset: (0, 2598, 1)]
*/

    _log(SE__DESTINY, "CustomsSE::EncodeDestiny(): %s - id:%li, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *CustomsSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for CustomsSE %u", m_cData.itemID);
    _log(POS__SLIMITEM, "MakeSlimItem for CustomsSE %u", m_cData.itemID);
    /** @todo (Allan) *Timestamp will need to be set to time current state is started. */
    PyDict *slim = new PyDict();
    slim->SetItemString("name",                 new PyString(m_self->itemName()));
    slim->SetItemString("nameID",               PyStatic.NewNone());
    slim->SetItemString("itemID",               new PyLong(m_cData.itemID));
    slim->SetItemString("typeID",               new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",              new PyInt(m_ownerID));  //1000148 for interbus customs office (to be done on creation)
    slim->SetItemString("corpID",               IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
    slim->SetItemString("allianceID",           IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
    slim->SetItemString("warFactionID",         IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
    slim->SetItemString("level",                new PyInt(m_oData.level));
    slim->SetItemString("orbitalTimestamp",     new PyLong(m_cData.timestamp));
    slim->SetItemString("planetID",             new PyInt(m_oData.planetID));  // planetID for this orbital
    slim->SetItemString("orbitalState",         new PyInt(m_cData.state));   // this needs to be ORBITAL state...not structure state
    PyTuple* tuple = new PyTuple(3);            // yaw, pitch, roll = getattr(slimItem, 'dunRotation', None)
        tuple->SetItem(0,                       new PyFloat(m_oData.rotation.x));
        tuple->SetItem(1,                       new PyFloat(m_oData.rotation.y));
        tuple->SetItem(2,                       new PyFloat(m_oData.rotation.z)); //MakeRandomFloat(-180, 180)
    slim->SetItemString("dunRotation", tuple);  // direction to planet
    //  dunno what these are...
    slim->SetItemString("orbitalHackerProgress",  m_oData.orbitalHackerProgress > 0 ? new PyFloat(m_oData.orbitalHackerProgress) : PyStatic.NewNone());  // packets show this as none if not value
    slim->SetItemString("orbitalHackerID",     m_oData.orbitalHackerID > 0 ? new PyInt(m_oData.orbitalHackerID) : PyStatic.NewNone());   // packets show this as none if not value

    if (is_log_enabled(POS__SLIMITEM)) {
        _log( POS__SLIMITEM, "CustomsSE::MakeSlimItem() - %s(%u)", GetName(), m_cData.itemID);
        slim->Dump(POS__SLIMITEM, "     ");
    }
    return slim;
}

void CustomsSE::Killed(Damage &damage) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr) or (m_system == nullptr))
        return; // make error here?

    uint32 killerID = 0;
    Client* pClient(nullptr);
    SystemEntity* killer = damage.srcSE;

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityList.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (pClient == nullptr) {
            sLog.Error("CustomsSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else {
            killerID = pClient->GetCharacterID();
        }
    } else {
        killerID = killer->GetID();
    }

    std::stringstream blob;
    blob << "<items>";
    std::vector<InventoryItemRef> survivedItems;
    std::map<uint32, InventoryItemRef> deadShipInventory;
    deadShipInventory.clear();
    m_self->GetMyInventory()->GetInventoryMap(deadShipInventory);
    if (!deadShipInventory.empty()) {
        uint32 s = 0, d = 0, x = 0;
        for (auto cur : deadShipInventory) {
            d = 0;
            x = cur.second->quantity();
            s = (cur.second->isSingleton() ? 1 : 0);
            if (cur.second->categoryID() == EVEDB::invCategories::Blueprint) {
                // singleton for bpo = 1, bpc = 2.
                BlueprintRef bpRef = BlueprintRef::StaticCast(cur.second);
                s = (bpRef->copy() ? 2 : s);
            }
            blob << "<i t=" << cur.second->typeID() << " f=" << cur.second->flag() << " s=" << s ;
            // all items have 50% chance of drop, even from popped ship
            if (IsEven(MakeRandomInt(0, 100))) {
                // item survived.  check qty for drop
                if (x > 1) {
                    d = MakeRandomInt(0, x);
                    x -= d;
                }
                // move item to vector for insertion into wreck later on
                survivedItems.push_back(cur.second);
            }
            blob << " d=" << d << " x=" << x << "/>";
        }
    }
    blob << "</items>";

    /* populate kill data for killMail and save to db  -allan 01May16  --updated 13July17 */
    /** @todo  check for tower/tcu/sbu/jammer and make killmail */
    /** @todo send pos mail/notification to corp members */
    KillData data = KillData();
        data.solarSystemID = m_system->GetID();
        data.victimCharacterID = 0; // charID = 0 means strucuture/item
        data.victimCorporationID = m_corpID;
        data.victimAllianceID = m_allyID;
        data.victimFactionID = m_warID;
        data.victimShipTypeID = GetTypeID();

        data.finalCharacterID = killerID;
        data.finalCorporationID = killer->GetCorporationID();
        data.finalAllianceID = killer->GetAllianceID();
        data.finalFactionID = killer->GetWarFactionID();
        data.finalShipTypeID = killer->GetTypeID();
        data.finalWeaponTypeID = damage.weaponRef->typeID();
        data.finalSecurityStatus = 0;  /* fix this */
        data.finalDamageDone = damage.GetTotal();

        uint32 totalHP = m_self->GetAttribute(AttrHP).get_int();
            totalHP += m_self->GetAttribute(AttrArmorHP).get_int();
            totalHP += m_self->GetAttribute(AttrShieldCapacity).get_int();
        data.victimDamageTaken = totalHP;

        data.killBlob = blob.str().c_str();
        data.killTime = GetFileTimeNow();
        data.moonID = m_oData.planetID;    /* denotes moonID for POS/Structure kills */

    ServiceDB::SaveKillOrLoss(data);

    uint32 locationID = GetLocationID();
    //  log faction kill in dynamic data   -allan
    MapDB::AddKill(locationID);
    MapDB::AddFactionKill(locationID);

    if (pClient != nullptr) {
        //award kill bounty.
        //AwardBounty( pClient );
        if (m_system->GetSystemSecurityRating() > 0)
            AwardSecurityStatus(m_self, pClient->GetChar().get());  // this awards secStatusChange for npcs in empire space
    }

    GPoint wreckPosition = m_destiny->GetPosition();
    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    ItemData wreckItemData(3962/*CO gantry*/, killerID, locationID, flagNone, wreck_name.c_str(), wreckPosition, itoa(m_allyID));
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("CustomsSE::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), 3962);
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE))
        _log(PHYSICS__TRACE, "Ship::Killed() - Ship %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
        GetName(), GetID(), x(), y(), z(), wreckItemRef->name(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);

    DropLoot(wreckItemRef, m_self->groupID(), killerID);

    for (auto cur: survivedItems)
        cur->Move(wreckItemRef->itemID(), flagNone); // populate wreck with items that survived

    DBSystemDynamicEntity wreckEntity = DBSystemDynamicEntity();
        wreckEntity.allianceID = killer->GetAllianceID();
        wreckEntity.categoryID = EVEDB::invCategories::Celestial;
        wreckEntity.corporationID = killer->GetCorporationID();
        wreckEntity.factionID = m_warID;
        wreckEntity.groupID = EVEDB::invGroups::Wreck;
        wreckEntity.itemID = wreckItemRef->itemID();
        wreckEntity.itemName = wreck_name;
        wreckEntity.ownerID = killerID;
        wreckEntity.typeID = 3962;
        wreckEntity.position = wreckPosition;

    if (!m_system->BuildDynamicEntity(wreckEntity, m_self->itemID())) {
        sLog.Error("CustomsSE::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", 3962);
        wreckItemRef->Delete();
        return;
    }
    m_destiny->SendJettisonPacket();
}

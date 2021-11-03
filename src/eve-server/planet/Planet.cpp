
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */

 /*
  * PLANET__ERROR
  * PLANET__WARNING
  * PLANET__MESSAGE
  * PLANET__DEBUG
  * PLANET__INFO
  * PLANET__TRACE
  * PLANET__DUMP
  * PLANET__RES_DUMP
  * PLANET__GC_DUMP
  * PLANET__PKT_TRACE
  * PLANET__DB_ERROR
  * PLANET__DB_WARNING
  */


#include "Client.h"
#include "EVEServerConfig.h"
#include "math/Trig.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/CustomsOffice.h"
#include "packets/Planet.h"
#include "pos/Structure.h"
#include "system/Celestial.h"
#include "system/SystemManager.h"


Planet::Planet()
{
    /** @todo  will need to make an InventoryItem class specific for planets here eventually */
}


PlanetSE::PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system),
pCO(nullptr)
{
    m_data = PlanetResourceData();
}

PlanetSE::~PlanetSE()
{
    for (auto cur : m_colonies) {
        cur.second->Shutdown();
        SafeDelete(cur.second);
    }
}

bool PlanetSE::LoadExtras() {
    if (!StaticSystemEntity::LoadExtras())
        return false;

    /** @todo use this to initialize planet data, create planet manager, or whatever else i decide is needed for planet management
     *  this is called after SE is created.
     */
    std::vector<uint16> typeIDs;
    sPlanetDataMgr.GetPlanetData(m_self->typeID(), typeIDs);
    m_data.type_1 = typeIDs.at(0);
    m_data.type_2 = typeIDs.at(1);
    m_data.type_3 = typeIDs.at(2);
    m_data.type_4 = typeIDs.at(3);
    m_data.type_5 = typeIDs.at(4);

    /** @todo save planet data after creation.  change data every x hours?days? */

    /*  quality: (min=1.0, max=154.275)  */
    // these are relative indicators of material quantity.  makes no difference to ecu or extraction amount
    // as system matures, we will begin adjusting these (from extractor data) and saving per planet
    float sysSec = (1.1 - m_system->GetSystemSecurityRating());    // 0.1 - 2.0
    int min = round(sysSec *10);
    m_data.dist_1 = MakeRandomInt(min, 75) * sysSec + MakeRandomFloat(0, 4);
    m_data.dist_2 = MakeRandomInt(min, 75) * sysSec + MakeRandomFloat(0, 4);
    m_data.dist_3 = MakeRandomInt(min, 75) * sysSec + MakeRandomFloat(0, 4);
    m_data.dist_4 = MakeRandomInt(min, 75) * sysSec + MakeRandomFloat(0, 4);
    m_data.dist_5 = MakeRandomInt(min, 75) * sysSec + MakeRandomFloat(0, 4);

    //if (sysSec > 1) {
        for (uint16 i=0; i<3600; ++i)  //this cannot use numList
            m_data.buffer_1 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (uint16 i=0; i<3600; ++i)
            m_data.buffer_2 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (uint16 i=0; i<3600; ++i)
            m_data.buffer_3 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (uint16 i=0; i<3600; ++i)
            m_data.buffer_4 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (uint16 i=0; i<3600; ++i)
            m_data.buffer_5 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
    /* } else {
        for (uint16 i=0; i<1000; ++i) {
            m_data.buffer_1 += asciiList[MakeRandomInt(0,57)];   // random fill buffer to capacity, 1k8 bytes.
            m_data.buffer_2 += alphaList[MakeRandomInt(0,26)];   // random fill buffer to capacity, 1k8 bytes.
            m_data.buffer_3 += numList[MakeRandomInt(0,10)];   // random fill buffer to capacity, 1k8 bytes.
            m_data.buffer_4 += numList[MakeRandomInt(0,10)];   // random fill buffer to capacity, 1k8 bytes.
            m_data.buffer_5 += numList[MakeRandomInt(0,10)];   // random fill buffer to capacity, 1k8 bytes.
        }
    } */
/*
    for (uint16 i=0; i<800; ++i) {
        m_data.buffer_1 += std::to_string(0);
        m_data.buffer_2 += std::to_string(0);
        m_data.buffer_3 += std::to_string(0);
        m_data.buffer_4 += std::to_string(0);
        m_data.buffer_5 += std::to_string(0);
    }
*/
    m_typeBuffers[m_data.type_1] = m_data.buffer_1;
    m_typeBuffers[m_data.type_2] = m_data.buffer_2;
    m_typeBuffers[m_data.type_3] = m_data.buffer_3;
    m_typeBuffers[m_data.type_4] = m_data.buffer_4;
    m_typeBuffers[m_data.type_5] = m_data.buffer_5;

    // should we check for a CO here?
    //  no, it hasnt been loaded at this point

    return true;
}

// called from SystemManager::Process() @ 1m
void PlanetSE::Process()
{
    for (auto cur : m_colonies)
        cur.second->Process();
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict)
{
    /*  from eve/client/script/environment/planet\clientPlanet.py
     * This method is used to fetch spherical harmonic data for a given
     * resource type on the current planet.
     */
    // will update this to use PI skills (sent in dict) as system grows..not sure how yet.
    /** @todo  this needs a minor rewrite....bands are dictated by client request.
     * bufferData is random fill based on bands, but kept per planet
     * will have to create a method to fill buffer with random values, rather than fill with single value
     *  the full 30 band data buffer will be created on planet creation for each resource, and the "bands"
     * are the "layers" of the resource, per se, with more layers giving higher degree of accuracy.
     *  the client sends depth request, and that will determine the bands and buffer size to return.
     * the requested bands will have to be taken from the full 30-band data buffer, as needed.
     * this resource data *MAY* change over the course of the running server, but not decided how/when/why yet.
     */

    // actual hex from live for "data"
    //E80E364382F13EBFEC13F03FEB42483F1263B2BF8C94004075360EC19842ACBEC0B7FABF0C5DA73EF4F6E03FC0B21C3EE0D71F3E5206233E8FC6E43E5245E6BFAD29A63F112C81 \
    BF6038393EFE19C1BF8FA975C2808D093FE0BB5EBF6C9A533FA1DB4FBF04DDAF3D3CE450C0435DB7BCBDDB80BF2EAD783FE028A8C04A733FBF76012BBFEA17FCBFF4F4113E6C7EC9 \
    3FFB8EDBBE52C084BF04608B3DFA748F3E3715823F5DA7F1BE5FA97B4206CE83BE2597E03FF6AF46BF2F6EA9BE160297BFBD70FB3FC02B75BF66A0AD3FA095213F2ED9033E5B9D50 \
    BE94BC53BE2C77E9BFDFDDE24066B21F3F5B1A043FC6531D40E2426CBE5012204036F3ACBF4EA60DBFBDDC46BFB469044020D610BFF68E0EBF1145A8BFD3B0063FF43EB7BFAF69DE \
    3F437B92C14232D1BEA0E45FBFDC639BBE4813C43E476F2D3F3E260F403436BC3EA993B8BFCDE9153F22C3243F0021DCBF9ECB443F619A6EBF824FE03F4118ED3E96AD5C3F9413B5 \
    3FE8D0CA3D0EE85CBDF3EC6D3E32E5EDBE3DBAA13EB2379BBF18A8233FCFA5E1BE91B0C03FA51AC9BE9F3F593F4B160FC0A817F0BEF1E5AE3F487E6C3F586B943F4921A73FA845FE \
    BEAE9D103FE725AEBF1B3A9FC14F69733F3B1B04BF18B9AE3FCBF4D0BD447B483F3E1870BFAAA7A1BEEC3C35C076BB1C3F697F493D00C4C7BEE74CFABFA4E6053F972189BCBDC3AE \
    3F42E984BF85FDBC3E3C9275BF3C07FFBD4C30D0BED828C43E6CF6C3C0C2F5CDBEBD5E3DBFACB29ABFCA75B1BD6A4035BFE537F83E165BD3BD5D3F4FBDACA1F6BF2A97E0BD4C1F44 \
    3FF054B6BE692C0F3F8EFB933F52BDE4BF5D97D33E4998C8BF22E25BBE581710BF9F093A3E3D8E8F3DBFEF453F9D32443E1B2EAD41B9FB39BFA85A753F73FABFBFE1B155BE01FFBE \
    BF897341BF715F19BE5739323EC2E50C3FF7A2B23F47499ABF6E4E1E3F93F555BEF51F8E3FBC21C13DC69CE5BFCD1E5DBF997FC23DDB348D3E300E6B3D49D4183FEDE50CBFD2EBAC \
    BE509C37BE5310D9BF22BFA940BBE2013F9437083FEC5EB33FE79D053E8FB0903F12BB5CBF4D02BC3E26DAC2BF0CD894BDEEB8B7BEAE1F1440564199BEFE322BBFE5F690BE9E1241 \
    3FDD38A5BFB2AEAC3E3C8681BF19A36F3F3E2C9D3EB039813E46BD47BF200996BE12F0A3BFC1DAFE3E9C5EA4BFAC65223F74A7EBC03047C3BB1CB01ABF15148F3E9B71163E6C545E \
    3FDB34553F34096E3D6E12A03F3CF08ABFE904303F367E9ABE064D583FEAE2953F3A5DC7BE

    /*
    dict.resourceTypeID;
    dict.advancedPlanetology;
    dict.oldBand;
    dict.planetology;
    dict.remoteSensing;
    dict.updateTime;
    */
    std::map<uint16, std::string>::iterator itr = m_typeBuffers.find(dict.resourceTypeID);
    if (itr == m_typeBuffers.end())
        return nullptr;
    uint16 size = (uint16)pow(dict.newBand, 2) *4;
    std::string data = itr->second.substr(0, size);
    // adjust data for system security.  not sure how to make it 'less' yet
    _log(PLANET__DEBUG, "PlanetSE::GetResourceData() for %s (%u) using remoteSense: %u, planetology: %u, advPlanetology: %u - updateTime: %u, proximity: %u, newBand: %u, oldBand: %u, bufferSize: %u", \
                sPIDataMgr.GetProductName(dict.resourceTypeID), dict.resourceTypeID, dict.remoteSensing, dict.planetology, dict.advancedPlanetology, \
                dict.updateTime, dict.proximity, dict.newBand, dict.oldBand, size);
    PyDict* args = new PyDict();
        args->SetItemString("data", new PyString(data));
        args->SetItemString("numBands", new PyInt(dict.newBand));
        args->SetItemString("proximity", new PyInt(dict.proximity));
    //PyIncRef(args);
    PyObject* rtn = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        rtn->Dump(PLANET__RES_DUMP, "   ");
    return rtn;
}

PyRep* PlanetSE::GetPlanetResourceInfo()
{
    PyDict* res = new PyDict();
        res->SetItem(new PyInt(m_data.type_1), new PyFloat(m_data.dist_1));
        res->SetItem(new PyInt(m_data.type_2), new PyFloat(m_data.dist_2));
        res->SetItem(new PyInt(m_data.type_3), new PyFloat(m_data.dist_3));
        res->SetItem(new PyInt(m_data.type_4), new PyFloat(m_data.dist_4));
        res->SetItem(new PyInt(m_data.type_5), new PyFloat(m_data.dist_5));
    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "   ");
    return res;
}

PyRep* PlanetSE::GetPlanetInfo(Colony* pColony) {
    PyDict *args = new PyDict();
    args->SetItem("planetTypeID", new PyInt(m_self->typeID()));
    args->SetItem("solarSystemID", new PyInt(m_system->GetID()));
    args->SetItem("radius", new PyInt(GetRadius()));
    args->SetItem("planetID", new PyInt(m_self->itemID()));
    if (pColony->HasColony()) {
        //pColony->Update();
        args->SetItem("level", new PyInt(pColony->GetLevel()));
        args->SetItem("pins", pColony->GetPins());
        args->SetItem("links", pColony->GetLinks());
        args->SetItem("routes", pColony->GetRoutes());
        args->SetItem("currentSimTime", new PyLong(pColony->GetSimTime()));
    }
    //PyIncRef(args);
    PyObject *rtn = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        rtn->Dump(PLANET__RES_DUMP, "   ");
    return rtn;
}

PyRep* PlanetSE::GetExtractorsForPlanet(int32 planetID) {
    // NOTE this gets ALL extractors on this planet
    // returns typeID, ownerID, latitude?, longitude?

    DBQueryResult res;
    // {for ecu in planetID}  SELECT `headID`, `typeID`, `ownerID`, `latitude`, `longitude` FROM `piECUHeads`
    PlanetDB::GetExtractorsForPlanet(planetID, res);

    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict(new PyDict());
            dict->SetItem("pinID", new PyInt(row.GetInt(0)));
            dict->SetItem("typeID", new PyInt(row.GetInt(1)));
            dict->SetItem("ownerID", new PyInt(row.GetInt(2)));
            dict->SetItem("latitude", new PyFloat(row.GetFloat(3)));
            dict->SetItem("longitude", new PyFloat(row.GetFloat(4)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    if (is_log_enabled(PLANET__RES_DUMP))
        list->Dump(PLANET__RES_DUMP, "   ");
    return list;
}

Colony* PlanetSE::GetColony(Client* pClient)
{
    std::map<uint32, Colony*>::const_iterator itr = m_colonies.find(pClient->GetCharacterID());
    if (itr != m_colonies.end())
        return itr->second;
    Colony* pColony = new Colony(&m_services, pClient, this);
    m_colonies[pClient->GetCharacterID()] = pColony;

    return pColony;
}

void PlanetSE::AbandonColony(Colony* pColony)
{
    pColony->AbandonColony();
    m_colonies.erase(pColony->GetOwner());
}

void PlanetSE::CreateCustomsOffice()
{
    /** @todo  will need to write this code and make it play nice with everything else.
     * a CO will be a special container as a CustomsSE, linked to the planet it orbits, and any colony on that planet.
     * there is only one CO per planet, but ALL chars with a colony on that planet can access their items on the same CO
     * the CO will load all items when it loads, but will need checks on "view items" or "open container" to ONLY send
     * items owned by calling char, or NONE for chars that dont have a colony on that planet.
     * i dont know where/how to do that yet...will need testing
     * CO can go into reinforced mode, like towers (hint: use same code)
     */

    //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity, const char *_customInfo = "", bool _contraband = false);
    FactionData data = FactionData();
        data.ownerID = corpInterbus;
        data.factionID = factionInterBus;
        data.allianceID = 0;
        data.corporationID = corpInterbus;
    uint16 typeID = EVEDB::invTypes::InterbusCustomsOffice;

    if (m_system->GetSystemSecurityRating() > 0.49) {
        typeID = EVEDB::invTypes::PlanetaryCustomsOffice;
        data.ownerID = corpCONCORD;
        data.factionID = factionCONCORD; //sDataMgr.GetRegionFaction(m_system->GetRegionID());
        data.allianceID = 0;
        data.corporationID = corpCONCORD;
    }

    /*  this puts CO in warp-in bubble
    // calculate warp-in point
    GPoint pos = GetPosition();
    uint32 radius = m_self->radius();
    srandom(m_self->itemID());
    int64 rand = random();
    double j = (((rand / RAND_MAX) -1.0) / 3.0);
    double s = 20 * pow(0.025 * (10 * log10(radius/1000000) -39), 20) +0.5;
    s = EvE::max(0.5, EvE::min(s, 10.5));
    double t = asin((pos.x/fabs(pos.x)) * (pos.z / sqrt(pow(pos.x, 2) + pow(pos.z, 2)))) +j;
    uint32 d = radius * (s +1) +10000;
    pos.x += d * sin(t);
    pos.y += 0.5 * radius * sin(j);
    pos.z -= d * cos(t);
    // put CO 50km closer to planet than warpIn point.
    GVector dir(pos, m_self->position());
    dir.normalize();
    pos -= (dir * 50000);
    */
    // this puts CO in random 700km orbit around planet
    GPoint pos(GetPosition());
    //pos.MakeRandomPointOnSphere(GetRadius() + 700000);

    //uint32 dist = BUBBLE_RADIUS_METERS + 10000/*m_self->GetAttribute(AttrMoonAnchorDistance).get_int()*/;
    uint32 radius = GetRadius();
    float rad = EvE::Trig::Deg2Rad(25);

    pos.x += radius + 700000 * std::sin(rad);
    pos.z += radius + 700000 * std::cos(rad);
    pos.y += MakeRandomInt(-1000, 1000);

    ItemData idata(typeID, data.ownerID, m_system->GetID(), flagNone, 1, itoa(m_self->itemID()), false);
    StructureItemRef iRef = sItemFactory.SpawnStructure(idata);
    iRef->SetPosition(pos);
    iRef->ChangeSingleton(true, false);
    iRef->SaveItem();
    pCO = new CustomsSE(iRef, m_services, m_system, data);
    pCO->Init();
    m_system->AddEntity(pCO);
}

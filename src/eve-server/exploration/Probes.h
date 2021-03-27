 /**
  * @name Probes.h
  *     Probe Item/SE class for EVEmu
  *
  * @Author:        Allan
  * @date:          10 March 2018
  *
  */

#ifndef EVEMU_EXPLORE_PROBES_H_
#define EVEMU_EXPLORE_PROBES_H_


#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "../../eve-common/EVE_Scanning.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"


/**
 * InventoryItem for probe object.
 */
class ProbeItem
: public InventoryItem
{
    friend class InventoryItem; // to let it construct us
public:
    ProbeItem(uint32 itemID, const ItemType &_type, const ItemData &_data);
    virtual ~ProbeItem()                                { /* Do nothing here */ }

    /* virtual functions default to base class and overridden as needed */
    virtual void Delete();  //totally removes item from game and deletes from the DB.


    static ProbeItemRef Load( uint32 itemID);
    static ProbeItemRef Spawn( ItemData &data);

protected:
    using InventoryItem::_Load;
    //virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 itemID, const ItemType &type, const ItemData &data)
    {
        if ((type.groupID() != EVEDB::invGroups::Scanner_Probe)
        and (type.groupID() != EVEDB::invGroups::Survey_Probe)
        //and (type.groupID() != EVEDB::invGroups::Warp_Disruption_Probe)  this wont work here...
        and (type.groupID() != EVEDB::invGroups::Obsolete_Probes)) {
            _log(ITEM__ERROR, "Trying to load %s as Probe.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

        return ProbeItemRef( new ProbeItem(itemID, type, data));
    }

    static uint32 CreateItemID( ItemData &data);
};



/**
 * DynamicSystemEntity which represents celestial object in space
 */
class PyServiceMgr;
class Scan;

class ProbeSE : public DynamicSystemEntity {
public:
    // abandoned probe c'tor
    ProbeSE(ProbeItemRef self, PyServiceMgr& services, SystemManager* system);
    // launched probe c'tor
    ProbeSE(ProbeItemRef self, PyServiceMgr& services, SystemManager* system, InventoryItemRef moduleRef, ShipItemRef shipRef);
    virtual ~ProbeSE();

    /* Process Calls - Overridden as needed in derived classes */
    virtual bool ProcessTic();

    /* class type pointer querys. */
    virtual ProbeSE*            GetProbeSE()            { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsProbeSE()             { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

    /* specific functions handled in this class. */
    void RecoverProbe(PyList* list);
    void UpdateProbe(ProbeData& data);
    // removes probe from system and scan map, and sends RemoveProbe call to client
    // does not remove probe from entity list
    void RemoveProbe();
    void SendNewProbe();
    void SendSlimChange();
    void SendStateChange(uint8 state);
    void SendWarpStart(float travelTime);
    void SendWarpEnd();

    void SetScan(Scan* pScan)                           { m_scan = pScan; }
    void RemoveScan()                                   { m_scan = nullptr; }
    void StartStateTimer(uint16 time)                   { m_stateTimer.Start(time); }

    bool IsMoving();

    void SetState(uint8 state=Probe::State::Idle)       { m_state = state; }
    uint8 GetState()                                    { return m_state; }
    uint8 GetRangeStep()                                { return m_rangeStep; }

    // remaining move time in ms
    uint16 GetMoveTime()                                { return m_stateTimer.GetRemainingTime(); }
    int64 GetExpiryTime()                               { return m_expiry; }

    // return deviation in meters based on current probe settings
    float GetDeviation();
    // return probe scan range setting in km
    float GetScanRange()                                { return m_scanRange; }
    float GetRangeModifier(float dist);

    // total probe scan strength, based on data modified by char skills, ship, launcher, and range
    float GetScanStrength();

    GPoint GetDestination()                             { return m_destination; }
    const char* GetStateName(uint8 state);

    bool CanScanShips()                                 { return m_scanShips; }

    bool IsRing()                                       { return m_ring; }
    bool IsSphere()                                     { return m_sphere; }
    void SetRing(bool set=false)                        { m_ring = set; }
    void SetSphere(bool set=false)                      { m_sphere = set; }


    // this will allow players with Sensor Linking L5 and Signal Acquisition L5
    //  to add another set of probe results to the signal being scanned (max of 4)
    bool HasMaxSkill();

private:
    Timer m_lifeTimer;
    Timer m_returnTimer;
    Timer m_stateTimer;

    Scan* m_scan;
    Client* m_client;

    GPoint m_destination;

    ShipItemRef m_shipRef;
    InventoryItemRef m_moduleRef;

    bool m_scanShips;
    bool m_ring;                // used to send additional data to client for partial hit
    bool m_sphere;              // used to send additional data to client for partial hit

    uint8 m_state;
    uint8 m_rangeStep;          // range 'click'.  values are 1 to 8.  sent from client
    uint8 m_rangeFactor;        // exponent for range checks

    uint32 m_baseScanRange;     // lowest scan radius, in km

    int64 m_expiry;             // probe lifetime as filetime

    float m_secStatus;
    float m_scanRange;          // in km.  sent from client
    float m_scanStrength;       // scan str in decimal.  unsure of uom
    float m_scanDeviation;      // max scan deviation for this probe, in % where 1.0 = 100

};

#endif  // EVEMU_EXPLORE_PROBES_H_

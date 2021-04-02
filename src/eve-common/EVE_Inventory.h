/*
 *
 *
 *
 */


#ifndef EVE_INVENTORY_H
#define EVE_INVENTORY_H

namespace Inv {

    namespace Container {
        enum {
            Wallet            = 10001,
            Global            = 10002,
            SolarSystem       = 10003,
            Hangar            = 10004,
            ScrapHeap         = 10005,
            Factory           = 10006,
            Bank              = 10007,
            Recycler          = 10008,
            Offices           = 10009,
            StationCharacters = 10010,
            Character         = 10011,
            CorpMarket        = 10012
        };
    }

    namespace Update {
        //these are used with the OnItemChange packet to update client's item data (and trigger other actions)
        // updated for crucible.  -allan 16May16
        enum {
            Item          = 0,    //also ixLauncherCapacity?
            Type          = 1,    //also ixLauncherUsed = 1,
            Owner         = 2,    //also ixLauncherChargeItem?
            Location      = 3,
            Flag          = 4,
            Quantity      = 5,
            Group         = 6,
            Category      = 7,
            CustomInfo    = 8,
            StackSize     = 9,
            Singleton     = 10,
            Subitems      = 11        // not in client data
        };
    }

    /* POD structure for Inventory categories */
    struct CatData {
        bool            published :1;
        uint8           id;
        std::string     name;
        std::string     description;
    };

    /* POD structure for Inventory groups */
    struct GrpData {

        bool            useBasePrice :1;
        bool            allowManufacture :1;
        bool            allowRecycler :1;
        bool            anchored :1;
        bool            anchorable :1;
        bool            fittableNonSingleton :1;
        bool            published :1;
        uint8           catID;
        uint16          id;
        std::string     name;
        std::string     description;
    };

    /* POD structure for Inventory types */
    struct TypeData {
        bool            published :1;
        bool            isRefinable :1;
        bool            isRecyclable :1;
        uint8           race;
        uint8           metaLvl;
        uint16          id;
        uint16          groupID;
        uint16          portionSize;
        uint32          marketGroupID;
        float           chanceOfDuplicating;
        float           radius;
        float           mass;
        float           volume;
        float           capacity;
        double          basePrice;
        std::string     name;
        std::string     description;
    };

    /* POD structure for saving items */
    struct SaveData {
        bool            contraband :1;
        bool            singleton :1;
        EVEItemFlags    flag;
        uint16          typeID;
        uint32          itemID;
        uint32          ownerID;
        uint32          locationID;
        uint32          quantity;
        GPoint          position;
        std::string     customInfo;
    };

    /* POD structure for saving attribute data */
    struct AttrData {
        bool            type;          // 0=int, 1=float
        uint16          attrID;
        uint32          itemID;
        int64           valueInt;
        double          valueFloat;
    };

}

/*  AuditLogSecureContainer shit here....
*/
namespace ALSC {

    namespace Action {
        enum {
            Assemble        = 1,
            Repackage       = 2,
            SetName         = 3,
            Move            = 4,
            SetPassword     = 5,
            Add             = 6,
            Lock            = 7,
            Unlock          = 8,
            EnterPassword   = 9,
            Configure       = 10
        };
    }

    namespace NeedPass {
        enum {
            ToOpen          = 1,
            ToLock          = 2,
            ToUnlock        = 4,
            ToViewAuditLog  = 8
        };
    }
}

#endif  // EVE_INVENTORY_H

/*{'FullPath': u'UI/Inventory', 'messageID': 251033, 'label': u'CantOpenItemsInThisLocations'}(u'You can not open items in the delivery bay.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251034, 'label': u'CantRename'}(u'You cannot rename this type of object.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251035, 'label': u'CantSeeItem'}(u'You can not open that as you can not see it.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251036, 'label': u'CorporationDoesntOwn'}(u'You can not open that as it does not belong to your corporation.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251037, 'label': u'CanTakeDivisionContainer'}(u'Can Take Containers From Division  {[numeric]number} Hangar', None, {u'{[numeric]number}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'number'}})
{'FullPath': u'UI/Inventory', 'messageID': 251038, 'label': u'NoTakeRole'}(u'You require take access to open the container requested.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251039, 'label': u'CanTakeDivision'}(u'Can Take From Division {[numeric]number} Hangar', None, {u'{[numeric]number}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'number'}})
{'FullPath': u'UI/Inventory', 'messageID': 251040, 'label': u'NotSecurityOfficer'}(u'You do not have the role <b>Security Officer</b>.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251041, 'label': u'NoOfficeAtThisSTations'}(u'Your corporation does not have an office at this station.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251042, 'label': u'NoStaionIdInSession'}(u'You can not see that which does not exist.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251043, 'label': u'NoDeliveryHangerForYourCorp'}(u'The requested market delivery area does not belong to your corporation.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251044, 'label': u'CorpRoleMissing'}(u'You do not have the role <b>Accountant</b>, <b>Junior Accountant</b> or <b>Trader</b>.', None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251045, 'label': u'NoCorpHangerAtThisStations'}(u"You don't get delivery bays in the middle of nowhere.", None, None)
{'FullPath': u'UI/Inventory', 'messageID': 251046, 'label': u'OfficeDoesntBelongToYou'}(u'The requested hangar does not belong to a member of your corporation.', None, None)

*/

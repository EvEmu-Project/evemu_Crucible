/**
 * @name DataClasses.h
 *  data container classes that cannot be trivially constructed/destructed
 *
 * @author: allan
 * @date 4 January 2018
 */


#ifndef EVE_DATA_CLASSES_H
#define EVE_DATA_CLASSES_H

#include "eve-server.h"
#include "POD_containers.h"
#include "../eve-common/EVE_POS.h"


// POS class container for processing-type items
struct ReactorData {
public:
    ReactorData();
    ~ReactorData();

    void Init();
    void Clear();

private:
    bool active;
    int32 itemID;
    int16 reaction;     // bp typeID?
    std::map<uint32, EVEPOS::POS_Connections> connections;  // itemID, data
    std::map<uint32, EVEPOS::POS_Resource> demands;         // itemID, resourceData(typeID/quantity)
    std::map<uint32, EVEPOS::POS_Resource> supplies;        // itemID, resourceData(typeID/quantity)
};


#endif  // EVE_DATA_CLASSES_H
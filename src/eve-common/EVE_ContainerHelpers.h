#ifndef EVE_CONTAINER_HELPERS_H
#define EVE_CONTAINER_HELPERS_H

#include <cstdint>

namespace EvEContainer {

// Crucible celestial inventory groups that are player-assemblable containers.
// Do not allow other Celestial objects (stations, wrecks, gates, etc.) here.
inline bool IsAssemblableCargoContainerGroup(uint16_t groupID)
{
    return groupID == 12u   // Cargo Container
        || groupID == 340u  // Secure Cargo Container
        || groupID == 448u  // Audit Log Secure Container
        || groupID == 649u; // Freight Container
}

// Pure validation shared between the broker handler and regression tests.
// Corporate-owned containers are intentionally not handled until role
// permissions and the corporate inventory path are implemented safely.
inline bool CanAssembleCargoContainer(uint16_t groupID, bool isSingleton,
                                      int32_t quantity, uint32_t ownerID,
                                      uint32_t characterID, uint32_t locationID,
                                      uint32_t dockedStationID, bool isDocked)
{
    return IsAssemblableCargoContainerGroup(groupID)
        && !isSingleton && quantity > 0
        && characterID != 0 && ownerID == characterID
        && isDocked && dockedStationID != 0
        && locationID == dockedStationID;
}

} // namespace EvEContainer

#endif // EVE_CONTAINER_HELPERS_H

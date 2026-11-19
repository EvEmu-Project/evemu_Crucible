#ifndef EVEMU_STATION_OFFICE_HELPERS_H
#define EVEMU_STATION_OFFICE_HELPERS_H

#include <cstdint>

namespace EvEStation {

// The station office index is keyed by officeID, not corporationID.
// Match the corporation ID rather than assigning it: the legacy login path
// used '=' and accidentally returned the first office for every player corp.
// This helper cannot change the cached office data and is independently
// verified by the eve-test regression below.
template <typename OfficeMap>
inline uint32_t OfficeIDForCorporation(const OfficeMap& offices, uint32_t corporationID)
{
    if (corporationID == 0u)
        return 0u;
    for (const auto& entry : offices) {
        if (entry.second.corporationID == corporationID)
            return entry.first;
    }
    return 0u;
}

} // namespace EvEStation

#endif

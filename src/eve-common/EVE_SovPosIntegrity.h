#ifndef EVEMU_SOV_POS_INTEGRITY_H
#define EVEMU_SOV_POS_INTEGRITY_H

#include "EVE_Roles.h"
#include <cstdint>
#include <set>
#include <vector>

namespace EvESovPos {

// mapSystemSovInfo.claimTime is BIGINT FILETIME (100 ns ticks since 1601).
inline std::int64_t EffectiveClaimTime(std::int64_t proposed, std::int64_t now)
{
    return proposed > 0 ? proposed : now;
}

// Tower mutation requires the owner corporation's director or StarbaseConfig
// bit. A separate server ADMIN may override for maintenance.
inline bool CanConfigureStructure(std::uint32_t callerCorp,
                                  std::uint32_t structureCorp,
                                  std::int64_t corpRoles,
                                  bool admin)
{
    if (admin)
        return true;
    if (callerCorp == 0 || structureCorp == 0 || callerCorp != structureCorp)
        return false;
    return (corpRoles & (Corp::Role::Director | Corp::Role::StarbaseConfig)) != 0;
}

inline std::size_t CountDistinctNonzeroGateIDs(const std::vector<std::uint32_t>& gateIDs)
{
    std::set<std::uint32_t> unique;
    for (const std::uint32_t id : gateIDs)
        if (id != 0)
            unique.insert(id);
    return unique.size();
}
} // namespace EvESovPos

#endif // EVEMU_SOV_POS_INTEGRITY_H

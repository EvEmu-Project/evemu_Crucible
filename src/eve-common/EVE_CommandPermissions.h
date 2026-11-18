#ifndef EVEMU_COMMAND_PERMISSIONS_H
#define EVEMU_COMMAND_PERMISSIONS_H
#include <cstdint>
namespace EvECommand {
// Every bit of a composite role requirement must be present.
inline bool HasRequiredRole(std::int64_t accountRole, std::int64_t requiredRole)
{
    return (accountRole & requiredRole) == requiredRole;
}
// Alternative is independently sufficient only for a COMMAND_ANY declaration.
inline bool HasAnyRequiredRole(std::int64_t accountRole, std::int64_t requiredRole,
                               std::int64_t alternativeRole = 0)
{
    return HasRequiredRole(accountRole, requiredRole)
        || (alternativeRole != 0 && HasRequiredRole(accountRole, alternativeRole));
}
} // namespace EvECommand
#endif // EVEMU_COMMAND_PERMISSIONS_H

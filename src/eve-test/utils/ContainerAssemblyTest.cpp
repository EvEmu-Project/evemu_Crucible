#include "eve-test.h"
#include "EVE_ContainerHelpers.h"

#include <cstdio>
#include <cstdlib>

int utils_ContainerAssemblyTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const uint32_t characterID = 90000001u;
    const uint32_t stationID = 60000001u;
    auto can = [=](uint16_t group, bool singleton, int32_t quantity,
                   uint32_t owner, uint32_t location, bool docked) {
        return EvEContainer::CanAssembleCargoContainer(group, singleton,
                 quantity, owner, characterID, location, stationID, docked);
    };

    const bool passed =
        EvEContainer::IsAssemblableCargoContainerGroup(12)
        && EvEContainer::IsAssemblableCargoContainerGroup(340)
        && EvEContainer::IsAssemblableCargoContainerGroup(448)
        && EvEContainer::IsAssemblableCargoContainerGroup(649)
        && !EvEContainer::IsAssemblableCargoContainerGroup(6)   // Sun
        && !EvEContainer::IsAssemblableCargoContainerGroup(186) // Wreck
        && !EvEContainer::IsAssemblableCargoContainerGroup(952) // Mission-only
        && can(12, false, 1, characterID, stationID, true)
        && can(340, false, 20, characterID, stationID, true)
        && can(448, false, 2, characterID, stationID, true)
        && can(649, false, 1, characterID, stationID, true)
        && !can(12, true, 1, characterID, stationID, true)
        && !can(12, false, 0, characterID, stationID, true)
        && !can(12, false, -1, characterID, stationID, true)
        && !can(12, false, 1, characterID + 1, stationID, true)
        && !can(12, false, 1, characterID, stationID + 1, true)
        && !can(12, false, 1, characterID, stationID, false)
        && !EvEContainer::CanAssembleCargoContainer(12, false, 1,
                 characterID, 0, stationID, stationID, true)
        && !EvEContainer::CanAssembleCargoContainer(12, false, 1,
                 characterID, characterID, stationID, 0, true);

    if (!passed) {
        ::printf("Crucible container assembly eligibility test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Crucible container assembly eligibility tests passed.\n");
    return EXIT_SUCCESS;
}

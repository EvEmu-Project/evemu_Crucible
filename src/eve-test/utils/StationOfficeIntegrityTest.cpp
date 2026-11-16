#include "eve-test.h"
#include "EVE_StationOfficeHelpers.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>

struct OfficeMock {
    uint32_t corporationID;
};

int utils_StationOfficeIntegrityTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    const std::map<uint32_t, OfficeMock> stationOffices {
        { 66000001u, { 98000011u } },
        { 66000002u, { 98000012u } },
        { 66000003u, { 98000013u } }
    };

    const bool valid =
        EvEStation::OfficeIDForCorporation(stationOffices, 98000011u) == 66000001u
        && EvEStation::OfficeIDForCorporation(stationOffices, 98000012u) == 66000002u
        && EvEStation::OfficeIDForCorporation(stationOffices, 98000013u) == 66000003u
        && EvEStation::OfficeIDForCorporation(stationOffices, 98000014u) == 0u
        && EvEStation::OfficeIDForCorporation(stationOffices, 0u) == 0u
        && stationOffices.at(66000001u).corporationID == 98000011u
        && stationOffices.at(66000002u).corporationID == 98000012u
        && EvEStation::OfficeIDForCorporation(std::map<uint32_t, OfficeMock>{}, 98000011u) == 0u;

    if (!valid) {
        std::printf("Station office corporation matching regression FAILED.\n");
        return EXIT_FAILURE;
    }
    std::printf("Station office corporation matching regression passed.\n");
    return EXIT_SUCCESS;
}

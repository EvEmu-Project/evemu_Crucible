#include "eve-test.h"
#include "EVE_SovPosIntegrity.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>

int utils_SovereigntyPosIntegrityTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    const std::int64_t now = 134151912000000000LL;
    const std::int64_t prior = 132000000000000000LL;
    const bool passed =
        EvESovPos::EffectiveClaimTime(prior, now) == prior
        && EvESovPos::EffectiveClaimTime(0, now) == now
        && EvESovPos::EffectiveClaimTime(-1, now) == now
        && EvESovPos::CanConfigureStructure(1001, 1001, Corp::Role::StarbaseConfig, false)
        && EvESovPos::CanConfigureStructure(1001, 1001, Corp::Role::Director, false)
        && !EvESovPos::CanConfigureStructure(1001, 1002, Corp::Role::StarbaseConfig, false)
        && !EvESovPos::CanConfigureStructure(0, 1001, Corp::Role::StarbaseConfig, false)
        && !EvESovPos::CanConfigureStructure(1001, 1001, 0, false)
        && EvESovPos::CanConfigureStructure(1001, 1002, 0, true)
        && EvESovPos::CountDistinctNonzeroGateIDs(
              std::vector<std::uint32_t>{3000001, 3000001, 3000002, 0}) == 2
        && EvESovPos::CountDistinctNonzeroGateIDs(
              std::vector<std::uint32_t>{0, 0}) == 0;
    if (!passed) {
        ::printf("Sovereignty/POS integrity test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Sovereignty/POS integrity test passed.\n");
    return EXIT_SUCCESS;
}

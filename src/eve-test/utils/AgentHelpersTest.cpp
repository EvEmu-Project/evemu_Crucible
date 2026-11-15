#include "eve-test.h"
#include "EVE_AgentHelpers.h"

#include <cstdio>
#include <cstdlib>
#include <string>

int utils_AgentHelpersTest(int argc, char* argv[]) {
    (void)argc; (void)argv;
    const double accepted = 130000000000000000.0;
    const double tick = 10000000.0;
    const bool passed =
        EvEAgent::IsStationAgentPortraitRequest("Character", 3010819)
        && EvEAgent::IsStationAgentPortraitRequest("Character", 3000000)
        && EvEAgent::IsStationAgentPortraitRequest("Character", 3999999)
        && !EvEAgent::IsStationAgentPortraitRequest("Character", 4000000)
        && !EvEAgent::IsStationAgentPortraitRequest("Character", 90000000)
        && !EvEAgent::IsStationAgentPortraitRequest("Corporation", 3010819)
        && EvEAgent::ModernImageUrl("Character", 3010819, 50)
            == "https://images.evetech.net/characters/3010819/portrait?size=64"
        && EvEAgent::ModernImageUrl("Corporation", 1000012, 64)
            == "https://images.evetech.net/corporations/1000012/logo?size=64"
        && EvEAgent::ModernImageUrl("InventoryType", 587, 32)
            == "https://images.evetech.net/types/587/icon?size=32"
        && EvEAgent::ModernImageUrl("Render", 587, 512)
            == "https://images.evetech.net/types/587/render?size=512"
        && EvEAgent::ModernImageUrl("Unknown", 1, 64).empty()
        && EvEAgent::BonusTimeRemaining(accepted, 30, accepted + 10 * 60 * tick)
            == static_cast<int64_t>(20 * 60 * tick)
        && EvEAgent::BonusTimeRemaining(accepted, 30, accepted + 31 * 60 * tick) == 0
        && EvEAgent::BonusTimeRemaining(0, 30, accepted)
            == static_cast<int64_t>(30 * 60 * tick)
        && EvEAgent::BonusWithinDeadline(accepted, 30, accepted + 10 * 60 * tick, 25000)
        && !EvEAgent::BonusWithinDeadline(accepted, 30, accepted + 40 * 60 * tick, 25000)
        && !EvEAgent::BonusWithinDeadline(accepted, 30, accepted + 10 * 60 * tick, 0);
    if (!passed) {
        ::printf("Agent helper test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Agent helper tests passed.\n");
    return EXIT_SUCCESS;
}

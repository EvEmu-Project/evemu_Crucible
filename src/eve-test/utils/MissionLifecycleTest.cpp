#include "eve-test.h"
#include "EVE_Missions.h"

#include <cstdio>
#include <cstdlib>
#include <map>

int utils_MissionLifecycleTest(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    MissionOffer expired{};
    expired.agentID = 100;
    expired.characterID = 200;
    expired.offerID = 300;
    MissionOffer other{};
    other.agentID = 100;
    other.characterID = 201;
    other.offerID = 301;

    // Simulate two characters talking to the same agent: expiring the first
    // must not remove the second one's offer.
    std::multimap<uint32, MissionOffer> offers;
    offers.emplace(expired.agentID, other);
    offers.emplace(expired.agentID, expired);
    const auto range = offers.equal_range(expired.agentID);
    for (auto it = range.first; it != range.second; ++it) {
        if (Mission::MatchesOffer(it->second, expired.characterID, expired.offerID)) {
            offers.erase(it);
            break;
        }
    }

    const bool passed =
        Mission::IsAwaitingAcceptance(Mission::State::Offered)
        && Mission::IsAwaitingAcceptance(Mission::State::Allocated)
        && !Mission::IsAwaitingAcceptance(Mission::State::Accepted)
        && !Mission::IsAwaitingAcceptance(Mission::State::Completed)
        && !Mission::MatchesOffer(other, expired.characterID, expired.offerID)
        && offers.size() == 1
        && offers.begin()->second.offerID == other.offerID
        && Mission::RemainingItemQuantity(10, 3) == 7
        && Mission::RemainingItemQuantity(10, 10) == 0
        && Mission::RemainingItemQuantity(10, 20) == 0
        && Mission::RemainingItemQuantity(100000, 1) == 99999
        && Mission::RemainingItemQuantity(100000, 0) == 100000;

    if (!passed) {
        ::printf("Mission lifecycle regression test failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("Mission lifecycle regression tests passed.\n");
    return EXIT_SUCCESS;
}

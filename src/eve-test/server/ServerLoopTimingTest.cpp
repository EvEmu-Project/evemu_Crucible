#include "eve-test.h"

#include "ServerLoopTiming.h"

#include <array>
#include <cstdio>

namespace {

struct SleepCase {
    std::uint32_t targetMilliseconds;
    std::uint32_t elapsedMilliseconds;
    std::uint32_t expectedMilliseconds;
};

} // namespace

int server_ServerLoopTimingTest(int, char **) {
    constexpr std::uint32_t kPeriodMilliseconds = 10;
    const std::array<SleepCase, 6> cases = {{
        {kPeriodMilliseconds, 0, kPeriodMilliseconds},
        {kPeriodMilliseconds, 1, kPeriodMilliseconds - 1},
        {kPeriodMilliseconds, kPeriodMilliseconds - 1, 1},
        {kPeriodMilliseconds, kPeriodMilliseconds, 0},
        {kPeriodMilliseconds, kPeriodMilliseconds + 1, 0},
        {0, 0, 0},
    }};

    for (const SleepCase &testCase : cases) {
        const std::uint32_t actualMilliseconds =
            ServerLoop::RemainingSleepMilliseconds(
                testCase.targetMilliseconds, testCase.elapsedMilliseconds);
        if (actualMilliseconds != testCase.expectedMilliseconds) {
            std::fprintf(stderr, "Unexpected loop sleep duration\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

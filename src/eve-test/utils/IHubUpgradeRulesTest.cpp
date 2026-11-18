#include "eve-test.h"
#include "EVE_IHubUpgradeRules.h"
#include <cstdio>
#include <cstdlib>

int utils_IHubUpgradeRulesTest(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    const bool passed =
        EVESovIHub::SupercapitalConstructionFacilities == 2009U
        && EVESovIHub::RequiredStrategicDays(2009U) == 100U
        && !EVESovIHub::MeetsStrategicRequirement(2009U, 0U)
        && !EVESovIHub::MeetsStrategicRequirement(2009U, 99U)
        && EVESovIHub::MeetsStrategicRequirement(2009U, 100U)
        && EVESovIHub::MeetsStrategicRequirement(2009U, 101U)
        && EVESovIHub::RequiredStrategicDays(2008U) == 2U
        && EVESovIHub::RequiredStrategicDays(2001U) == 3U
        && EVESovIHub::RequiredStrategicDays(32422U) == 3U
        && EVESovIHub::MeetsStrategicRequirement(2008U, 2U)
        && EVESovIHub::MeetsStrategicRequirement(2001U, 3U)
        && !EVESovIHub::MeetsStrategicRequirement(2001U, 2U)
        && !EVESovIHub::MeetsStrategicRequirement(0U, 1000U)
        && !EVESovIHub::MeetsStrategicRequirement(99999U, 1000U)
        && EVESovIHub::KindForGroup(1016U) == EVESovIHub::UpgradeKind::Strategic
        && EVESovIHub::KindForGroup(1020U) == EVESovIHub::UpgradeKind::Industrial
        && EVESovIHub::KindForGroup(1021U) == EVESovIHub::UpgradeKind::Military
        && EVESovIHub::KindForGroup(123U) == EVESovIHub::UpgradeKind::Unsupported
        && EVESovIHub::IsSupportedUpgrade(2009U, 1016U)
        && EVESovIHub::IsSupportedUpgrade(12345U, 1020U)
        && EVESovIHub::IsSupportedUpgrade(12345U, 1021U)
        && !EVESovIHub::IsSupportedUpgrade(12345U, 1016U)
        && !EVESovIHub::IsSupportedUpgrade(2009U, 999U)
        && EVESovIHub::MeetsActivityRequirement(30000000U, 5U, 30000000U)
        && EVESovIHub::MeetsActivityRequirement(1000000000U, 5U, 1000000000U)
        && !EVESovIHub::MeetsActivityRequirement(5U, 5U, 30000000U)
        && !EVESovIHub::MeetsActivityRequirement(1000000000U, 6U, 1000000000U);

    if (!passed) {
        ::printf("IHub strategic upgrade validation regression failed.\n");
        return EXIT_FAILURE;
    }
    ::printf("IHub strategic upgrade validation regression passed.\n");
    return EXIT_SUCCESS;
}

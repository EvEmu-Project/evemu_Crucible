#include "eve-test.h"
#include "EVE_IndustryFormulas.h"
#include "utils/EvEMath.h"
#include <cstdio>
#include <limits>

int utils_IndustryFormulaTest(int, char**)
{
    using EVEmuIndustry::RequiredMaterialQuantity;
    using EVEmuIndustry::ValidCopyRuns;
    if (RequiredMaterialQuantity(3, 1.1, 1.0, 3) != 10
        || RequiredMaterialQuantity(100, 0.9, 1.05, 3) != 285
        || RequiredMaterialQuantity(100, 0.8, 1.1, 3, true) != 300
        || RequiredMaterialQuantity(100, 0.8, 1.1, 3, false) != 270
        || RequiredMaterialQuantity(100, 1.0, 1.0, 0) != 0
        || RequiredMaterialQuantity(100, 1.0, 0.5, 5) != 0
        || RequiredMaterialQuantity(5, std::numeric_limits<double>::quiet_NaN(), 1.0, 2) != 0
        || RequiredMaterialQuantity(1000000000U, 100.0, 2.0, 10000)
            != std::numeric_limits<std::uint32_t>::max()
        || !ValidCopyRuns(2, 10, 10)
        || ValidCopyRuns(0, 10, 10)
        || ValidCopyRuns(2, 0, 10)
        || ValidCopyRuns(2, 11, 10)
        || ValidCopyRuns(32768, 1, 10)
        || ValidCopyRuns(1, 32768, 32768)
        || ValidCopyRuns(1, 1, 0)) {
        std::fprintf(stderr, "Industry materials/copy validation mismatch\n");
        return 1;
    }
    // Research skills should REDUCE time for both ME and PE, never increase it.
    if (EvEMath::RAM::ME_ResearchTime(1300, 0, 1, 5)
            >= EvEMath::RAM::ME_ResearchTime(1300, 0, 1, 0)
        || EvEMath::RAM::PE_ResearchTime(1500, 0, 1, 5)
            >= EvEMath::RAM::PE_ResearchTime(1500, 0, 1, 0)
        || EvEMath::RAM::CopyTime(200000, 0) != 200000
        || EvEMath::RAM::CopyTime(200000, 5)
            >= EvEMath::RAM::CopyTime(200000, 0)) {
        std::fprintf(stderr, "Industry research/copy time regression\n");
        return 1;
    }
    std::printf("Industry formula tests passed.\n");
    return 0;
}

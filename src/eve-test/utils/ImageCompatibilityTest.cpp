#include "eve-test.h"
#include "EVE_AgentHelpers.h"
#include "EVE_ImageHelpers.h"
#include "EVE_LegacyAgentThumbnail.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

int utils_ImageCompatibilityTest(int argc, char* argv[]) {
    (void)argc; (void)argv;
    const std::vector<unsigned char> jpeg = { 0xFF, 0xD8, 0xFF, 0xE0, 0x00 };
    const std::vector<unsigned char> png = { 0x89, 0x50, 0x4E, 0x47, 0x0D };
    const std::vector<unsigned char> empty;
    const std::vector<unsigned char> truncated = { 0xFF, 0xD8 };
    const std::vector<char> signedJpeg = {
        static_cast<char>(0xFF), static_cast<char>(0xD8),
        static_cast<char>(0xFF), static_cast<char>(0xE1) };

    // Validate actual production response bytes, rather than just a
    // 50px filename or a loosely checked Start-of-Image signature.
    const unsigned char* art = EvEImage::kLegacyAgentThumbnail50;
    const std::size_t artSize = EvEImage::kLegacyAgentThumbnail50Bytes;
    std::vector<char> thumbnail(art, art + artSize);
    bool baseline50 = false;
    for (std::size_t i = 0; i + 10 < artSize; ++i) {
        if (art[i] == 0xFF && art[i + 1] == 0xC0 &&
            art[i + 2] == 0x00 && art[i + 3] == 0x11 &&
            art[i + 4] == 0x08 && art[i + 5] == 0x00 &&
            art[i + 6] == 50 && art[i + 7] == 0x00 &&
            art[i + 8] == 50 && art[i + 9] == 0x03)
            baseline50 = true;
    }
    const bool valid =
        artSize == 322
        && baseline50
        && EvEImage::HasJpegSignature(thumbnail)
        && art[artSize - 2] == 0xFF && art[artSize - 1] == 0xD9
        && EvEImage::IsLegacyAgentThumbnail50("Character", 3011918u, 50u)
        && EvEImage::IsLegacyAgentThumbnail50("Character", 3000000u, 50u)
        && EvEImage::IsLegacyAgentThumbnail50("Character", 3999999u, 50u)
        && !EvEImage::IsLegacyAgentThumbnail50("Character", 90000000u, 50u)
        && !EvEImage::IsLegacyAgentThumbnail50("Character", 3011918u, 64u)
        && !EvEImage::IsLegacyAgentThumbnail50("Corporation", 3011918u, 50u)
        && EvEImage::HasJpegSignature(jpeg)
        && EvEImage::HasJpegSignature(signedJpeg)
        && !EvEImage::HasJpegSignature(png)
        && !EvEImage::HasJpegSignature(empty)
        && !EvEImage::HasJpegSignature(truncated)
        && EvEAgent::ModernImageUrl("Character", 3011918u, 50u).empty()
        && EvEAgent::ModernImageUrl("Character", 90000000u, 256u).empty()
        && EvEAgent::ModernImageUrl("Corporation", 1000012u, 64u)
            == "https://images.evetech.net/corporations/1000012/logo?size=64"
        && EvEAgent::ModernImageUrl("InventoryType", 587u, 32u)
            == "https://images.evetech.net/types/587/icon?size=32";
    if (!valid) {
        std::printf("Crucible image compatibility regression FAILED.\n");
        return EXIT_FAILURE;
    }
    std::printf("Crucible image compatibility regression passed.\n");
    return EXIT_SUCCESS;
}

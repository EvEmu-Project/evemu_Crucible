#ifndef EVEMU_LEGACY_AGENT_THUMBNAIL_H
#define EVEMU_LEGACY_AGENT_THUMBNAIL_H

// Classify a Crucible-era station-agent 50px portrait request. The image
// content comes from image_cache/Character/<agentID>_50.jpg or from the
// operator-supplied agent_fallback_50.jpg, never from compiled-in JPEG bytes.
#include <cstdint>
#include <string>

namespace EvEImage {
inline bool IsLegacyAgentThumbnail50(const std::string& category,
                                    uint32_t id, uint32_t size)
{
    return category == "Character" && id >= 3000000u
        && id <= 3999999u && size == 50u;
}
} // namespace EvEImage
#endif

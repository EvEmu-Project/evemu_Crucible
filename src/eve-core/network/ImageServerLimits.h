#ifndef __IMAGE_SERVER_LIMITS_H__INCL__
#define __IMAGE_SERVER_LIMITS_H__INCL__

#include <chrono>
#include <cstddef>

namespace ImageServerLimits {

constexpr std::size_t MAX_HEADER_BYTES = 8192u;
constexpr std::size_t MAX_IMAGE_BYTES = 4u * 1024u * 1024u;
constexpr std::size_t MAX_PENDING_UPLOADS = 128u;
constexpr std::size_t MAX_CONNECTIONS = 64u;
constexpr std::chrono::seconds REQUEST_TIMEOUT(10);
constexpr std::chrono::seconds ACCEPT_RETRY_DELAY(1);

}

#endif /* !__IMAGE_SERVER_LIMITS_H__INCL__ */

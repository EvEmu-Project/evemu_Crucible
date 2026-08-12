#ifndef __IMAGE_REQUEST_PARSER_H__INCL__
#define __IMAGE_REQUEST_PARSER_H__INCL__

#include <cstdint>
#include <string>

struct ImageRequest {
    std::string category;
    std::uint32_t id;
    std::uint32_t size;
};

bool ParseImageRequest(
    const std::string& requestLine,
    ImageRequest& request );

#endif /* !__IMAGE_REQUEST_PARSER_H__INCL__ */

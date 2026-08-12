#include "eve-core.h"

#include "network/ImageRequestParser.h"

#include <array>
#include <charconv>
#include <sstream>
#include <string_view>
#include <system_error>

namespace {

constexpr std::size_t MAX_REQUEST_LINE_BYTES = 2048u;
constexpr std::size_t MAX_QUERY_BYTES = 512u;
constexpr std::array<std::string_view, 5> CATEGORIES = {
    "Alliance",
    "Corporation",
    "Character",
    "InventoryType",
    "Render" };

bool ParseId(
    const std::string& value,
    std::uint32_t& parsed )
{
    if ( value.empty() )
        return false;

    const char* begin = value.data();
    const char* end = begin + value.size();
    const std::from_chars_result result =
        std::from_chars( begin, end, parsed, 10 );
    return result.ec == std::errc() && result.ptr == end && parsed > 0;
}

bool IsCategory( const std::string& category )
{
    for ( const std::string_view allowed : CATEGORIES )
        if ( category == allowed )
            return true;
    return false;
}

bool IsExpectedExtension(
    const std::string& category,
    const std::string& extension )
{
    const std::string_view expected =
        category == "Character" ? "jpg" : "png";
    return extension == expected;
}

}

bool ParseImageRequest(
    const std::string& requestLine,
    ImageRequest& request )
{
    if ( requestLine.empty() ||
         requestLine.size() > MAX_REQUEST_LINE_BYTES )
        return false;

    std::istringstream line( requestLine );
    std::string method;
    std::string target;
    std::string version;
    std::string extra;
    if ( !( line >> method >> target >> version ) || line >> extra )
        return false;
    if ( method != "GET" ||
         ( version != "HTTP/1.0" && version != "HTTP/1.1" ) )
        return false;
    if ( target.size() < 2 || target[0] != '/' )
        return false;
    if ( target.find( '#' ) != std::string::npos )
        return false;

    const std::size_t queryStart = target.find( '?' );
    if ( queryStart != std::string::npos &&
         target.size() - queryStart - 1 > MAX_QUERY_BYTES )
        return false;
    const std::string path = target.substr( 0, queryStart );

    const std::size_t categoryEnd = path.find( '/', 1 );
    if ( categoryEnd == std::string::npos )
        return false;

    const std::string category = path.substr( 1, categoryEnd - 1 );
    if ( !IsCategory( category ) )
        return false;

    const std::string filename = path.substr( categoryEnd + 1 );
    const std::size_t separator = filename.find( '_' );
    const std::size_t extensionStart = filename.find( '.', separator );
    if ( separator == std::string::npos ||
         filename.find( '_', separator + 1 ) != std::string::npos ||
         filename.find( '/' ) != std::string::npos ||
         (extensionStart != std::string::npos &&
          filename.find( '.', extensionStart + 1 ) != std::string::npos) )
        return false;

    const std::string id = filename.substr( 0, separator );
    const std::string size = filename.substr(
        separator + 1,
        extensionStart - separator - 1 );
    const std::string extension = extensionStart == std::string::npos
        ? ""
        : filename.substr( extensionStart + 1 );
    if ( !extension.empty() &&
         !IsExpectedExtension( category, extension ) )
        return false;

    ImageRequest parsed = { category, 0, 0 };
    if ( !ParseId( id, parsed.id ) || !ParseId( size, parsed.size ) )
        return false;

    request = parsed;
    return true;
}

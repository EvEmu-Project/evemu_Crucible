/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    ------------------------------------------------------------------------------------
 */

#ifndef __NETWORK__PROTOCOL_LIMITS_H__INCL__
#define __NETWORK__PROTOCOL_LIMITS_H__INCL__

#include <cstddef>
#include <cstdint>

namespace EveProtocol {

constexpr std::size_t MAX_PACKET_SIZE = 1024u * 1024u;
constexpr std::size_t MAX_QUEUED_PACKET_COUNT = 64u;
constexpr std::size_t MAX_QUEUED_PACKET_BYTES = 4u * MAX_PACKET_SIZE;
constexpr std::size_t MAX_BUFFERED_PACKET_BYTES =
    MAX_PACKET_SIZE + MAX_QUEUED_PACKET_BYTES + sizeof(std::uint32_t);
constexpr std::size_t MAX_MARSHAL_DEPTH = 64u;
constexpr std::size_t MAX_MARSHAL_OBJECT_COUNT = 65536u;
constexpr std::size_t MAX_MARSHAL_CONTAINER_COUNT = 65536u;
constexpr std::size_t MAX_MARSHAL_SAVED_OBJECT_COUNT = 16384u;

}  // namespace EveProtocol

#endif /* !__NETWORK__PROTOCOL_LIMITS_H__INCL__ */

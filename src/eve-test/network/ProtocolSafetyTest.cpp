/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    ------------------------------------------------------------------------------------
 */

#include "eve-test.h"

#include "marshal/EVEUnmarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "network/ImageRequestParser.h"
#include "network/ProtocolLimits.h"
#include "network/StreamPacketizer.h"
#include "utils/Deflate.h"

#include <array>
#include <limits>

namespace {

Buffer BuildFrame(uint32 length, uint8 value)
{
    Buffer frame(sizeof(uint32) + length, 0);
    frame.Get<uint32>(0) = length;
    for (uint32 index = 0; index < length; ++index)
        frame.Get<uint8>(sizeof(uint32) + index) = value;
    return frame;
}

Buffer BuildMarshalHeader()
{
    Buffer stream;
    stream.Append<uint8>(MarshalHeaderByte);
    stream.Append<uint32>(0);
    return stream;
}

bool RejectsMarshal(Buffer& stream)
{
    PyRep* rep = Unmarshal(stream);
    const bool rejected = rep == nullptr;
    PySafeDecRef(rep);
    return rejected;
}

bool IsSingleBytePacket(Buffer* packet, uint8 expected)
{
    const bool valid = packet != nullptr && packet->size() == 1 &&
        packet->Get<uint8>(0) == expected;
    SafeDelete(packet);
    return valid;
}

}  // namespace

int network_ProtocolSafetyTest(int, char**)
{
    StreamPacketizer packetizer;

    Buffer oversized(sizeof(uint32), 0);
    oversized.Get<uint32>(0) =
        static_cast<uint32>(EveProtocol::MAX_PACKET_SIZE + 1);
    if (packetizer.InputData(oversized) || !packetizer.IsInvalid())
        return EXIT_FAILURE;

    packetizer.ClearBuffers();
    Buffer zeroLength(sizeof(uint32), 0);
    if (packetizer.InputData(zeroLength) || !packetizer.IsInvalid())
        return EXIT_FAILURE;

    packetizer.ClearBuffers();
    Buffer partial(sizeof(uint32) + 1, 0);
    partial.Get<uint32>(0) =
        static_cast<uint32>(EveProtocol::MAX_PACKET_SIZE);
    partial.Get<uint8>(sizeof(uint32)) = 0xA5;
    if (!packetizer.InputData(partial) || !packetizer.Process())
        return EXIT_FAILURE;
    if (packetizer.PopPacket() != nullptr)
        return EXIT_FAILURE;

    packetizer.ClearBuffers();
    Buffer frames = BuildFrame(1, 0x11);
    Buffer oneByteFrame = BuildFrame(1, 0x11);
    for (std::size_t index = 1;
         index < EveProtocol::MAX_QUEUED_PACKET_COUNT + 1;
         ++index)
        frames.AppendSeq(oneByteFrame.begin<uint8>(), oneByteFrame.end<uint8>());
    if (!packetizer.InputData(frames) || packetizer.Process())
        return EXIT_FAILURE;

    packetizer.ClearBuffers();
    Buffer valid = BuildFrame(1, 0x5A);
    if (!packetizer.InputData(valid) || !packetizer.Process())
        return EXIT_FAILURE;
    if (!IsSingleBytePacket(packetizer.PopPacket(), 0x5A))
        return EXIT_FAILURE;

    Buffer empty;
    if (IsDeflated(empty))
        return EXIT_FAILURE;
    Buffer inflated;
    if (InflateData(empty, inflated))
        return EXIT_FAILURE;
    if (InflateUnmarshal(empty) != nullptr)
        return EXIT_FAILURE;

    Buffer validMarshal = BuildMarshalHeader();
    validMarshal.Append<uint8>(Op_PyNone);
    PyRep* validRep = Unmarshal(validMarshal);
    if (validRep == nullptr || !validRep->IsNone())
        return EXIT_FAILURE;
    PyDecRef(validRep);

    Buffer truncatedHeader = BuildMarshalHeader();
    truncatedHeader.Resize<uint8>(1);
    if (!RejectsMarshal(truncatedHeader))
        return EXIT_FAILURE;

    Buffer truncatedPrimitive = BuildMarshalHeader();
    truncatedPrimitive.Append<uint8>(Op_PyLongLong);
    if (!RejectsMarshal(truncatedPrimitive))
        return EXIT_FAILURE;

    Buffer oversizedString = BuildMarshalHeader();
    oversizedString.Append<uint8>(Op_PyLongString);
    oversizedString.Append<uint8>(0xFF);
    oversizedString.Append<uint32>(
        static_cast<uint32>(EveProtocol::MAX_PACKET_SIZE));
    if (!RejectsMarshal(oversizedString))
        return EXIT_FAILURE;

    Buffer oversizedTuple = BuildMarshalHeader();
    oversizedTuple.Append<uint8>(Op_PyTuple);
    oversizedTuple.Append<uint8>(0xFF);
    oversizedTuple.Append<uint32>(std::numeric_limits<uint32>::max());
    if (!RejectsMarshal(oversizedTuple))
        return EXIT_FAILURE;

    Buffer unhashableKey = BuildMarshalHeader();
    unhashableKey.Append<uint8>(Op_PyDict);
    unhashableKey.Append<uint8>(1);
    unhashableKey.Append<uint8>(Op_PyNone);
    unhashableKey.Append<uint8>(Op_PyEmptyList);
    if (!RejectsMarshal(unhashableKey))
        return EXIT_FAILURE;

    Buffer deepTuple = BuildMarshalHeader();
    for (std::size_t index = 0;
         index < EveProtocol::MAX_MARSHAL_DEPTH + 1; ++index)
        deepTuple.Append<uint8>(Op_PyOneTuple);
    deepTuple.Append<uint8>(Op_PyNone);
    if (!RejectsMarshal(deepTuple))
        return EXIT_FAILURE;

    Buffer trailingData = BuildMarshalHeader();
    trailingData.Append<uint8>(Op_PyNone);
    trailingData.Append<uint8>(Op_PyNone);
    if (!RejectsMarshal(trailingData))
        return EXIT_FAILURE;

    ImageRequest imageRequest = { "", 0, 0 };
    if (!ParseImageRequest(
            "GET /Character/123_512.jpg HTTP/1.0",
            imageRequest) ||
        imageRequest.category != "Character" ||
        imageRequest.id != 123 ||
        imageRequest.size != 512)
        return EXIT_FAILURE;

    if (!ParseImageRequest(
            "GET /Character/123_512.jpg?cache=1 HTTP/1.1",
            imageRequest) ||
        imageRequest.id != 123 ||
        !ParseImageRequest(
            "GET /Corporation/456_64 HTTP/1.0",
            imageRequest) ||
        imageRequest.category != "Corporation" ||
        imageRequest.size != 64)
        return EXIT_FAILURE;

    const std::array<const char*, 5> invalidImageRequests = {
        "GET /Character/-1_512.jpg HTTP/1.0",
        "GET /Character/123_999999999999.jpg HTTP/1.0",
        "GET /Character/123_512.gif HTTP/1.0",
        "GET /Character/123_512.jpg#fragment HTTP/1.0",
        "POST /Character/123_512.jpg HTTP/1.0" };
    for (const char* invalidRequest : invalidImageRequests) {
        if (ParseImageRequest(invalidRequest, imageRequest))
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

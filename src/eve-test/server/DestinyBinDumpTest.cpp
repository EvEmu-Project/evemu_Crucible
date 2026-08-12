#include "eve-test.h"

#include "destiny/DestinyBinDump.h"

#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

namespace {

constexpr LogType kLogType = DESTINY__BALL_DECODE;
constexpr int64 kDefaultEntityID = 1;
constexpr uint8 kFormationID = 0xff;

template <typename T>
void Append(std::vector<uint8> &bytes, const T &value)
{
    const size_t old_size = bytes.size();
    bytes.resize(old_size + sizeof(T));
    std::memcpy(bytes.data() + old_size, &value, sizeof(T));
}

Destiny::BallHeader MakeBallHeader(uint8 mode, uint8 flags, int64 entityID)
{
    Destiny::BallHeader header = {};
    header.entityID = entityID;
    header.mode = mode;
    header.radius = 1.0f;
    header.posX = 1.0;
    header.posY = 2.0;
    header.posZ = 3.0;
    header.flags = flags;
    return header;
}

std::vector<uint8> MakeBall(uint8 mode, uint8 flags,
                            int64 entityID = kDefaultEntityID)
{
    std::vector<uint8> bytes;
    Append(bytes, MakeBallHeader(mode, flags, entityID));

    if (mode != Destiny::Ball::Mode::RIGID) {
        Destiny::MassSector mass = {};
        mass.mass = 100.0;
        mass.allianceID = std::numeric_limits<int64>::max();
        mass.corporationID = 42;
        mass.harmonic = 7;
        Append(bytes, mass);
    }

    if ((flags & Destiny::Ball::Flag::IsFree) != 0) {
        Destiny::DataSector data = {};
        data.maxSpeed = 10.0f;
        data.velX = 1.0;
        data.velY = 2.0;
        data.velZ = 3.0;
        data.inertia = 0.5f;
        data.speedfraction = 0.25f;
        Append(bytes, data);
    }

    return bytes;
}

bool AppendModeTail(std::vector<uint8> &bytes, uint8 mode)
{
    switch (mode) {
        case Destiny::Ball::Mode::GOTO: {
            Destiny::GOTO_Struct tail = {};
            tail.formationID = kFormationID;
            tail.x = 4.0;
            tail.y = 5.0;
            tail.z = 6.0;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::FOLLOW: {
            Destiny::FOLLOW_Struct tail = {};
            tail.formationID = kFormationID;
            tail.followID = std::numeric_limits<int64>::max();
            tail.followRange = 25.0f;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::STOP: {
            Destiny::STOP_Struct tail = {};
            tail.formationID = kFormationID;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::WARP: {
            Destiny::WARP_Struct tail = {};
            tail.formationID = kFormationID;
            tail.targX = 7.0;
            tail.targY = 8.0;
            tail.targZ = 9.0;
            tail.effectStamp = 11;
            tail.followRange = std::numeric_limits<int64>::max();
            tail.followID = std::numeric_limits<int64>::max();
            tail.speed = 12;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::ORBIT: {
            Destiny::ORBIT_Struct tail = {};
            tail.formationID = kFormationID;
            tail.targetID = 1234;
            tail.followRange = 30.0;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::MISSILE: {
            Destiny::MISSILE_Struct tail = {};
            tail.formationID = kFormationID;
            tail.targetID = std::numeric_limits<int64>::max();
            tail.followRange = 2.0f;
            tail.ownerID = std::numeric_limits<int64>::max();
            tail.effectStamp = 13;
            tail.x = 1.0;
            tail.y = 2.0;
            tail.z = 3.0;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::MUSHROOM: {
            Destiny::MUSHROOM_Struct tail = {};
            tail.formationID = kFormationID;
            tail.followRange = 3.0f;
            tail.unknown125 = 4.0;
            tail.effectStamp = 14;
            tail.ownerID = std::numeric_limits<int64>::max();
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::TROLL: {
            Destiny::TROLL_Struct tail = {};
            tail.formationID = kFormationID;
            tail.effectStamp = 15;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::FIELD: {
            Destiny::FIELD_Struct tail = {};
            tail.formationID = kFormationID;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::RIGID: {
            Destiny::RIGID_Struct tail = {};
            tail.formationID = kFormationID;
            Append(bytes, tail);
            break;
        }
        case Destiny::Ball::Mode::FORMATION: {
            Destiny::FORMATION_Struct tail = {};
            tail.formationID = kFormationID;
            tail.followID = std::numeric_limits<int64>::max();
            tail.followRange = 40.0f;
            tail.effectStamp = 16;
            Append(bytes, tail);
            break;
        }
        default:
            return false;
    }

    return true;
}

std::vector<uint8> MakeCompleteBall(
    uint8 mode, uint8 flags = 0, int64 entityID = kDefaultEntityID)
{
    std::vector<uint8> bytes = MakeBall(mode, flags, entityID);
    if (!AppendModeTail(bytes, mode))
        bytes.clear();
    return bytes;
}

std::vector<uint8> MakeUpdate(const std::vector<uint8> &ball)
{
    std::vector<uint8> bytes;
    Destiny::AddBall_header header = {};
    header.packet_type = 1;
    header.stamp = 123;
    Append(bytes, header);
    bytes.insert(bytes.end(), ball.begin(), ball.end());
    return bytes;
}

bool ExpectUsed(const std::vector<uint8> &bytes, uint32 expected,
                const char *caseName)
{
    const uint32 actual = Destiny::DumpBall(
        kLogType, bytes.data(), static_cast<uint32>(bytes.size()));
    if (actual == expected)
        return true;

    std::fprintf(stderr, "%s: expected %u bytes, got %u\n", caseName,
                 expected, actual);
    return false;
}

bool ExpectRejected(const std::vector<uint8> &bytes, const char *caseName)
{
    return ExpectUsed(bytes, 0, caseName);
}

bool TestOuterAndBallHeaders()
{
    for (size_t length = 0; length < sizeof(Destiny::AddBall_header);
         ++length) {
        std::vector<uint8> bytes(length, 0);
        Destiny::DumpUpdate(kLogType, bytes.data(),
                            static_cast<uint32>(bytes.size()));
    }
    Destiny::DumpUpdate(kLogType, nullptr, sizeof(Destiny::AddBall_header));

    for (size_t length = 0; length < sizeof(Destiny::BallHeader); ++length) {
        std::vector<uint8> bytes(length, 0);
        if (!ExpectRejected(bytes, "BallHeader truncation"))
            return false;
    }

    return true;
}

bool TestSectorBoundaries()
{
    const std::vector<uint8> massBall =
        MakeCompleteBall(Destiny::Ball::Mode::STOP);
    const size_t massStart = sizeof(Destiny::BallHeader);
    const size_t massEnd = massStart + sizeof(Destiny::MassSector);
    for (size_t length = massStart; length < massEnd; ++length) {
        std::vector<uint8> prefix(massBall.begin(), massBall.begin() + length);
        if (!ExpectRejected(prefix, "MassSector truncation"))
            return false;
    }

    const std::vector<uint8> dataBall = MakeCompleteBall(
        Destiny::Ball::Mode::STOP, Destiny::Ball::Flag::IsFree);
    const size_t dataStart =
        sizeof(Destiny::BallHeader) + sizeof(Destiny::MassSector);
    const size_t dataEnd = dataStart + sizeof(Destiny::DataSector);
    for (size_t length = dataStart; length < dataEnd; ++length) {
        std::vector<uint8> prefix(dataBall.begin(), dataBall.begin() + length);
        if (!ExpectRejected(prefix, "DataSector truncation"))
            return false;
    }

    return true;
}

bool TestModeTails()
{
    const uint8 modes[] = {
        Destiny::Ball::Mode::GOTO,
        Destiny::Ball::Mode::FOLLOW,
        Destiny::Ball::Mode::STOP,
        Destiny::Ball::Mode::WARP,
        Destiny::Ball::Mode::ORBIT,
        Destiny::Ball::Mode::MISSILE,
        Destiny::Ball::Mode::MUSHROOM,
        Destiny::Ball::Mode::TROLL,
        Destiny::Ball::Mode::FIELD,
        Destiny::Ball::Mode::RIGID,
        Destiny::Ball::Mode::FORMATION,
    };

    for (uint8 mode : modes) {
        const std::vector<uint8> bytes = MakeCompleteBall(mode);
        const size_t prefixSize =
            sizeof(Destiny::BallHeader) +
            (mode == Destiny::Ball::Mode::RIGID
                 ? 0
                 : sizeof(Destiny::MassSector));
        for (size_t length = prefixSize; length < bytes.size(); ++length) {
            std::vector<uint8> prefix(bytes.begin(), bytes.begin() + length);
            if (!ExpectRejected(prefix, "mode tail truncation"))
                return false;
        }

        if (!ExpectUsed(bytes, static_cast<uint32>(bytes.size()),
                        "complete mode tail")) {
            return false;
        }
    }

    return true;
}

bool TestIDsAndModes()
{
    const int64 invalidIDs[] = {
        0,
        -1,
        std::numeric_limits<int64>::min(),
    };
    for (int64 entityID : invalidIDs) {
        const std::vector<uint8> bytes = MakeCompleteBall(
            Destiny::Ball::Mode::STOP, 0, entityID);
        if (!ExpectRejected(bytes, "invalid entity ID"))
            return false;
    }

    const int64 permittedIDs[] = {
        static_cast<int64>(2147483648LL),
        std::numeric_limits<int64>::max(),
    };
    for (int64 entityID : permittedIDs) {
        const std::vector<uint8> bytes = MakeCompleteBall(
            Destiny::Ball::Mode::STOP, 0, entityID);
        if (!ExpectUsed(bytes, static_cast<uint32>(bytes.size()),
                        "64-bit entity ID")) {
            return false;
        }
    }

    const uint8 invalidModes[] = {
        static_cast<uint8>(Destiny::MAX_DSTBALL + 1),
        std::numeric_limits<uint8>::max(),
    };
    for (uint8 mode : invalidModes) {
        if (!ExpectRejected(MakeBall(mode, 0), "invalid mode"))
            return false;
    }

    const uint8 unsupportedModes[] = {
        Destiny::Ball::Mode::BOID,
        Destiny::Ball::Mode::MINIBALL,
    };
    for (uint8 mode : unsupportedModes) {
        if (!ExpectRejected(MakeBall(mode, 0), "unsupported mode"))
            return false;
    }

    return true;
}

bool TestMiniBallCounts()
{
    const uint8 mode = Destiny::Ball::Mode::TROLL;
    std::vector<uint8> missingList = MakeCompleteBall(
        mode, Destiny::Ball::Flag::HasMiniBalls);
    if (!ExpectRejected(missingList, "missing mini-ball list"))
        return false;

    Destiny::MiniBallList emptyList = {};
    std::vector<uint8> zeroCount = MakeCompleteBall(
        mode, Destiny::Ball::Flag::HasMiniBalls);
    Append(zeroCount, emptyList);
    if (!ExpectUsed(zeroCount, static_cast<uint32>(zeroCount.size()),
                    "zero mini-ball count")) {
        return false;
    }

    Destiny::MiniBallList oneList = {};
    oneList.count = 1;
    std::vector<uint8> truncated = MakeCompleteBall(
        mode, Destiny::Ball::Flag::HasMiniBalls);
    Append(truncated, oneList);
    Destiny::MiniBall oneBall = {};
    Append(truncated, oneBall);
    for (size_t length = truncated.size() - sizeof(oneBall);
         length < truncated.size(); ++length) {
        std::vector<uint8> prefix(truncated.begin(),
                                  truncated.begin() + length);
        if (!ExpectRejected(prefix, "mini-ball truncation"))
            return false;
    }

    if (!ExpectUsed(truncated, static_cast<uint32>(truncated.size()),
                    "complete mini-ball")) {
        return false;
    }

    Destiny::MiniBallList impossible = {};
    impossible.count = std::numeric_limits<uint16>::max();
    std::vector<uint8> impossibleCount = MakeCompleteBall(
        mode, Destiny::Ball::Flag::HasMiniBalls);
    Append(impossibleCount, impossible);
    return ExpectRejected(impossibleCount, "impossible mini-ball count");
}

void ExerciseUpdatePrefixes(const std::vector<uint8> &ball)
{
    const std::vector<uint8> packet = MakeUpdate(ball);
    for (size_t length = 0; length < packet.size(); ++length) {
        Destiny::DumpUpdate(kLogType, packet.data(),
                            static_cast<uint32>(length));
    }
    Destiny::DumpUpdate(kLogType, packet.data(),
                        static_cast<uint32>(packet.size()));
}

bool TestUpdatePrefixes()
{
    const uint8 modes[] = {
        Destiny::Ball::Mode::GOTO,
        Destiny::Ball::Mode::FOLLOW,
        Destiny::Ball::Mode::STOP,
        Destiny::Ball::Mode::WARP,
        Destiny::Ball::Mode::ORBIT,
        Destiny::Ball::Mode::MISSILE,
        Destiny::Ball::Mode::MUSHROOM,
        Destiny::Ball::Mode::TROLL,
        Destiny::Ball::Mode::FIELD,
        Destiny::Ball::Mode::RIGID,
        Destiny::Ball::Mode::FORMATION,
    };

    for (uint8 mode : modes) {
        const std::vector<uint8> ball = MakeCompleteBall(mode);
        ExerciseUpdatePrefixes(ball);
    }

    ExerciseUpdatePrefixes(MakeCompleteBall(
        Destiny::Ball::Mode::STOP, Destiny::Ball::Flag::IsFree));
    return true;
}

} // namespace

int server_DestinyBinDumpTest(int, char **)
{
    if (!TestOuterAndBallHeaders() || !TestSectorBoundaries() ||
        !TestModeTails() || !TestIDsAndModes() || !TestMiniBallCounts()) {
        return EXIT_FAILURE;
    }

    if (!TestUpdatePrefixes())
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
    Rewrite:    Allan
*/

#include "eve-common.h"

#include "destiny/DestinyBinDump.h"

#include <cstring>

namespace Destiny {

    const char *const modeNames[] = {
        "GOTO",
        "FOLLOW",
        "STOP",
        "WARP",
        "ORBIT",
        "MISSILE",
        "MUSHROOM",
        "BOID",
        "TROLL",
        "MINIBALL",
        "FIELD",
        "RIGID",
        "FORMATION"
    };

namespace {

constexpr uint32 kDiagnosticHexLimit = 128;

bool Consume(const uint8 *&data, uint32 &len, size_t size)
{
    if (data == nullptr || size > static_cast<size_t>(len))
        return false;

    data += size;
    len -= static_cast<uint32>(size);
    return true;
}

template <typename T>
bool ReadStruct(LogType into, const char *name, const uint8 *&data,
                uint32 &len, T &value)
{
    const uint8 *source = data;
    const size_t required = sizeof(T);
    if (!Consume(data, len, required)) {
        _log(into, "Error: Truncated %s: need %u bytes, have %u", name,
             static_cast<uint32>(required), len);
        return false;
    }

    std::memcpy(&value, source, required);
    return true;
}

void DumpDiagnosticHex(LogType into, const uint8 *data, uint32 len)
{
    if (data == nullptr || len == 0)
        return;

    const uint32 dump_len =
        (len < kDiagnosticHexLimit ? len : kDiagnosticHexLimit);
    _hex(into, data, dump_len);
}

bool ReadMiniBalls(LogType into, const uint8 *&data, uint32 &len)
{
    MiniBallList list = {};
    if (!ReadStruct(into, "MiniBallList", data, len, list))
        return false;

    const uint32 mini_ball_size = static_cast<uint32>(sizeof(MiniBall));
    const uint32 count = list.count;
    if (count > len / mini_ball_size) {
        _log(into,
             "Error: Impossible mini-ball count %u with %u bytes remaining",
             count, len);
        return false;
    }

    if (count != 0)
        _log(into, "    MiniBall Count: %u", count);

    const uint32 bytes = count * mini_ball_size;
    return Consume(data, len, bytes);
}

} // namespace

void DumpUpdate(LogType into, const uint8 *data, uint32 len) {
    const uint32 packet_len = len;
    AddBall_header global_head = {};
    if (!ReadStruct(into, "AddBall header", data, len, global_head))
        return;

    _log(into, "AddBall: packet_type: %u, len: %u, stamp: %u ",
         global_head.packet_type, packet_len, global_head.stamp);

    while (len > 0) {
        uint32 used = DumpBall(into, data, len);
        if (used == 0)
            return;    //error
        if (used > len) {
            _log(into, "Error: Ball consumed %u bytes, only %u remain", used,
                 len);
            return;
        }
        data += used;
        len -= used;
    }
}

uint32 DumpBall(LogType into, const uint8 *data, uint32 len) {
    const uint32 init_len = len;
    const uint8 *ball_start = data;

    BallHeader ballhead = {};
    if (!ReadStruct(into, "BallHeader", data, len, ballhead))
        return 0;

    if (ballhead.entityID <= 0) {
        _log(into, "Error: Invalid entityID for ball %lli", ballhead.entityID);
        return 0;
    }

    if (ballhead.mode > MAX_DSTBALL) {
        _log(into, "Error: Invalid ball mode %u for ball %lli", ballhead.mode, ballhead.entityID);
        DumpDiagnosticHex(into, ball_start, init_len);
        return 0;
    }

    _log(into, "entity: %lli, mode: %s(%u) flags: %s", ballhead.entityID, modeNames[ballhead.mode], ballhead.mode, Destiny::GetFlagNames(ballhead.flags).c_str());
    _log(into, "   pos: %.2f, %.2f, %.2f, radius: %.1f", ballhead.posX, ballhead.posY, ballhead.posZ, ballhead.radius);

    if (ballhead.mode != Ball::Mode::RIGID) {
        MassSector masschunk = {};
        if (!ReadStruct(into, "MassSector", data, len, masschunk))
            return 0;

        _log(into, "   mass: %.2f, cloak: %u, harmonic: %i, corp: %i, alliance: %lli" ,
            masschunk.mass, masschunk.cloak, masschunk.harmonic, masschunk.corporationID, masschunk.allianceID);
    }

    //this seems a little strange, but this is how it works...
    if ((ballhead.flags & Ball::Flag::IsFree) == Ball::Flag::IsFree) {
        DataSector shipchunk = {};
        if (!ReadStruct(into, "DataSector", data, len, shipchunk))
            return 0;

        _log(into, "   maxSpeed: %.2f, Velocity: %.2f, %.2f, %.2f IM: %.4f, SF: %.3f",
            shipchunk.maxSpeed,
            shipchunk.velX, shipchunk.velY, shipchunk.velZ,
            shipchunk.inertia,
            shipchunk.speedfraction);
    }

    _log(into, "   %s:", modeNames[ballhead.mode]);
    switch(ballhead.mode) {
        case Ball::Mode::BOID:
        case Ball::Mode::MINIBALL: {
            _log(into, "       This is not coded (or correct) yet.");
            return 0;
        } break;
        case Ball::Mode::GOTO: {
            GOTO_Struct b = {};
            if (!ReadStruct(into, "GOTO tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, direction: %.2f, %.2f, %.2f", b.formationID, b.x, b.y, b.z);
        } break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct b = {};
            if (!ReadStruct(into, "FOLLOW tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, followID: %lli, distance: %.1f", b.formationID, b.followID, b.followRange);
        } break;
        case Ball::Mode::STOP: {
            STOP_Struct b = {};
            if (!ReadStruct(into, "STOP tail", data, len, b))
                return 0;
            _log(into, "       formID: %u ", b.formationID);
        } break;
        case Ball::Mode::WARP: {
            WARP_Struct b = {};
            if (!ReadStruct(into, "WARP tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, TargPt: %.2f, %.2f, %.2f start: %i", b.formationID, b.targX, b.targY, b.targZ, b.effectStamp);
            _log(into, "       followRange: %lli, followID: %lli, warpSpeed: %i", b.followRange, b.followID, b.speed);
        } break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct b = {};
            if (!ReadStruct(into, "ORBIT tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, targetID: %u, distance: %.1f", b.formationID, b.targetID, b.followRange);
        } break;
        case Ball::Mode::MISSILE: {
            MISSILE_Struct b = {};
            if (!ReadStruct(into, "MISSILE tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, targetID: %lli, followRange: %.1f, ownerID: %lli, start: %i", b.formationID, b.targetID, b.followRange, b.ownerID, b.effectStamp);
            _log(into, "       pos: %.2f, %.2f, %.2f", b.x, b.y, b.z);
        } break;
        case Ball::Mode::MUSHROOM: {
            MUSHROOM_Struct b = {};
            if (!ReadStruct(into, "MUSHROOM tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, distance: %.2f, u125: %.3f, start: %i, ownerID: %lli", b.formationID, b.followRange, b.unknown125, b.effectStamp, b.ownerID);
        } break;
        case Ball::Mode::TROLL: {
            TROLL_Struct b = {};
            if (!ReadStruct(into, "TROLL tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, start: %i", b.formationID, b.effectStamp);
        } break;
        case Ball::Mode::FIELD: {
            FIELD_Struct b = {};
            if (!ReadStruct(into, "FIELD tail", data, len, b))
                return 0;
            _log(into, "       formID: %u ", b.formationID);
        } break;
        case Ball::Mode::RIGID: {
            RIGID_Struct b = {};
            if (!ReadStruct(into, "RIGID tail", data, len, b))
                return 0;
            _log(into, "       formID: %u ", b.formationID);
        } break;
        case Ball::Mode::FORMATION: {   // not used
            FORMATION_Struct b = {};
            if (!ReadStruct(into, "FORMATION tail", data, len, b))
                return 0;
            _log(into, "       formID: %u, followID: %lli, followRange: %.2f, start: %i", b.formationID, b.followID, b.followRange, b.effectStamp);
        } break;
        default:
            _log(into, "Error: Unknown ball mode %u!", ballhead.mode);
            DumpDiagnosticHex(into, ball_start, init_len);
            return 0;
    }

    if ((ballhead.flags & Ball::Flag::HasMiniBalls) ==
        Ball::Flag::HasMiniBalls) {
        if (!ReadMiniBalls(into, data, len))
            return 0;
    }

    if (len > init_len) {
        _log(into, "ERROR: Consumed more bytes than given: had %u, used %u", init_len, len);
        return 0;
    }
    return init_len - len;
}

std::string GetFlagNames(uint8 flags)
{
    std::string res = "";
    if (flags & Ball::Flag::IsFree) {
        res += "IsFree";
        if (flags > Ball::Flag::IsFree)
            res += ", ";
    }
    if (flags & Ball::Flag::IsGlobal) {
        res += "IsGlobal";
        if (flags > Ball::Flag::IsGlobal)
            res += ", ";
    }
    if (flags & Ball::Flag::IsMassive) {
        res += "IsMassive";
        if (flags > Ball::Flag::IsMassive)
            res += ", ";
    }
    if (flags & Ball::Flag::IsInteractive) {
        res += "IsInteractive";
        if (flags > Ball::Flag::IsInteractive)
            res += ", ";
    }
    if (flags & Ball::Flag::IsMoribund) {
        res += "IsMoribund";
        if (flags > Ball::Flag::IsMoribund)
            res += ", ";
    }
    if (flags & Ball::Flag::HasMiniBalls)
        res += "HasMiniBalls";

    res += "(";
    res += std::to_string(flags);
    res += ")";
    return res;
}

}

/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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

void DumpUpdate(LogType into, const uint8 *data, uint32 len) {
    const AddBall_header *global_head = (const AddBall_header *) data;
    _log(into, "AddBall: packet_type: %u, len: %u, stamp: %u ", global_head->packet_type, len, global_head->stamp);
    data += sizeof(AddBall_header);
    len -= sizeof(AddBall_header);

    while(len > 0) {
        uint32 used = DumpBall(into, data, len);
        if (used == 0)
            return;    //error
        data += used;
        len -= used;
    }
}

uint32 DumpBall(LogType into, const uint8 *data, uint32 len) {
    uint32 init_len = len;

    const BallHeader *ballhead = (const BallHeader *) data;
    data += sizeof(BallHeader);
    len -= sizeof(BallHeader);

    if ((ballhead->entityID == 0) or (ballhead->entityID > 2147483647)) { // max int32
        _log(into, "Error: Invalid entityID for ball %li", ballhead->entityID);
        return 0;
    }

    if (ballhead->mode > MAX_DSTBALL) {
        _log(into, "Error: Invalid ball mode %u for ball %li", ballhead->mode, ballhead->entityID);
        return 0;
    }

    /*  not used yet.
    const NameStruct *name = (const NameStruct *) data;
    data += sizeof(NameStruct);
    len -= sizeof(NameStruct);
    if (name->name_len > 0) {
        _log(into, "   Name: len: %u", name->name_len);
        _log(into, "    ~ %s", name->name);

        data += name->name_len*sizeof(uint16);
        len  -= name->name_len*sizeof(uint16);
    }
    */
    _log(into, "entity: %li, mode: %s(%u) flags: %s", ballhead->entityID, modeNames[ballhead->mode], ballhead->mode, Destiny::GetFlagNames(ballhead->flags).c_str());
    _log(into, "   pos: %.2f, %.2f, %.2f, radius: %.1f", ballhead->posX, ballhead->posY, ballhead->posZ, ballhead->radius);

    if (ballhead->mode != Ball::Mode::RIGID) {
        const MassSector *masschunk = (const MassSector *) data;
        data += sizeof(MassSector);
        len -= sizeof(MassSector);

        _log(into, "   mass: %.2f, cloak: %u, harmonic: %i, corp: %i, alliance: %li" ,
            masschunk->mass, masschunk->cloak, masschunk->harmonic, masschunk->corporationID, masschunk->allianceID);
    }

    //this seems a little strange, but this is how it works...
    if ((ballhead->flags & Ball::Flag::IsFree) == Ball::Flag::IsFree) {
        const DataSector *shipchunk = (const DataSector *) data;
        data += sizeof(DataSector);
        len -= sizeof(DataSector);

        _log(into, "   maxSpeed: %.2f, Velocity: %.2f, %.2f, %.2f IM: %.4f, SF: %.3f",
            shipchunk->maxSpeed,
            shipchunk->velX, shipchunk->velY, shipchunk->velZ,
            shipchunk->inertia,
            shipchunk->speedfraction);
    }

    _log(into, "   %s:", modeNames[ballhead->mode]);
    switch(ballhead->mode) {
        case Ball::Mode::BOID:
        case Ball::Mode::MINIBALL: {
            _log(into, "       This is not coded (or correct) yet.");
            return 0;
        } break;
        case Ball::Mode::GOTO: {
            const GOTO_Struct *b = (const GOTO_Struct *) data;
            data += sizeof(GOTO_Struct);
            len -= sizeof(GOTO_Struct);
            _log(into, "       formID: %u, direction: %.2f, %.2f, %.2f", b->formationID, b->x, b->y, b->z);
        } break;
        case Ball::Mode::FOLLOW: {
            const FOLLOW_Struct *b = (const FOLLOW_Struct *) data;
            data += sizeof(FOLLOW_Struct);
            len -= sizeof(FOLLOW_Struct);
            _log(into, "       formID: %u, followID: %li, distance: %.1f", b->formationID, b->followID, b->followRange);
        } break;
        case Ball::Mode::STOP: {
            const STOP_Struct *b = (const STOP_Struct *) data;
            data += sizeof(STOP_Struct);
            len -= sizeof(STOP_Struct);
            _log(into, "       formID: %u ", b->formationID);
        } break;
        case Ball::Mode::WARP: {
            const WARP_Struct *b = (const WARP_Struct *) data;
            data += sizeof(WARP_Struct);
            len -= sizeof(WARP_Struct);
            _log(into, "       formID: %u, TargPt: %.2f, %.2f, %.2f start: %i", b->formationID, b->targX, b->targY, b->targZ, b->effectStamp);
            _log(into, "       followRange: %li, followID: %li, warpSpeed: %i", b->followRange, b->followID, b->speed);
        } break;
        case Ball::Mode::ORBIT: {
            const ORBIT_Struct *b = (const ORBIT_Struct *) data;
            data += sizeof(ORBIT_Struct);
            len -= sizeof(ORBIT_Struct);
            _log(into, "       formID: %u, targetID: %u, distance: %.1f", b->formationID, b->targetID, b->followRange);
        } break;
        case Ball::Mode::MISSILE: {
            const MISSILE_Struct *b = (const MISSILE_Struct *) data;
            data += sizeof(MISSILE_Struct);
            len -= sizeof(MISSILE_Struct);
            _log(into, "       formID: %u, targetID: %li, followRange: %.1f, ownerID: %li, start: %i", b->formationID, b->targetID, b->followRange, b->ownerID, b->effectStamp);
            _log(into, "       pos: %.2f, %.2f, %.2f", b->x, b->y, b->z);
        } break;
        case Ball::Mode::MUSHROOM: {
            const MUSHROOM_Struct *b = (const MUSHROOM_Struct *) data;
            data += sizeof(MUSHROOM_Struct);
            len -= sizeof(MUSHROOM_Struct);
            _log(into, "       formID: %u, distance: %.2f, u125: %.3f, start: %i, ownerID: %li", b->formationID, b->followRange, b->unknown125, b->effectStamp, b->ownerID);
        } break;
        case Ball::Mode::TROLL: {
            const TROLL_Struct *b = (const TROLL_Struct *) data;
            data += sizeof(TROLL_Struct);
            len -= sizeof(TROLL_Struct);
            _log(into, "       formID: %u, start: %i", b->formationID, b->effectStamp);
        } break;
        case Ball::Mode::FIELD: {
            const FIELD_Struct *b = (const FIELD_Struct *) data;
            data += sizeof(FIELD_Struct);
            len -= sizeof(FIELD_Struct);
            _log(into, "       formID: %u ", b->formationID);
        } break;
        case Ball::Mode::RIGID: {
            const RIGID_Struct *b = (const RIGID_Struct *) data;
            data += sizeof(RIGID_Struct);
            len -= sizeof(RIGID_Struct);
            _log(into, "       formID: %u ", b->formationID);
        } break;
        case Ball::Mode::FORMATION: {   // not used
            const FORMATION_Struct *b = (const FORMATION_Struct *) data;
            data += sizeof(FORMATION_Struct);
            len -= sizeof(FORMATION_Struct);
            _log(into, "       formID: %u, followID: %li, followRange: %.2f, start: %i", b->formationID, b->followID, b->followRange, b->effectStamp);
        } break;
        default:
            _log(into, "Error: Unknown ball mode %u!", ballhead->mode);
            _hex(into, data-sizeof(BallHeader), (len>128)?128:(len+sizeof(BallHeader)));
            return 0;
    }

    /*
    if (ballhead->flags & HasMiniBalls) {
        const MiniBallList* mbl = (const MiniBallList*)data;
        data += sizeof( MiniBallList );
        len -= sizeof( MiniBallList );

        if (mbl->count) {
            _log( into, "    MiniBall Count: %d", mbl->count );
            for( uint16 r = 0; r < mbl->count; ++r ) {
                const MiniBall* mini = (const MiniBall*)data;
                data += sizeof( MiniBall );
                len -= sizeof( MiniBall );
                _log( into, "        [%d] pos=(%.3f, %.3f, %.3f) radius: %.2f", r, mini->x, mini->y, mini->z, mini->radius );
            }
        }
    }
    */
    if (len > init_len) {
        _log(into, "ERROR: Consumed more bytes than given: had %u, used %u", init_len, len);
        return init_len;
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

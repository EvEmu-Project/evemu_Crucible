/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
*/

#include "eve-common.h"

#include "destiny/DestinyBinDump.h"
#include "destiny/DestinyStructs.h"

namespace Destiny {

    const char *const DSTBALL_modeNames[] = {
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

    const Destiny::AddBall_header *global_head = (const Destiny::AddBall_header *) data;
    _log(into, "AddBall: packet_type=%d, sequence=%d (len %d)", global_head->packet_type, global_head->sequence, len);
    data += sizeof(Destiny::AddBall_header);
    len -= sizeof(Destiny::AddBall_header);

    while(len > 0) {
        uint32 used = DumpBall(into, data, len);
        if(used == 0)
            return;    //error
        data += used;
        len -= used;
    }
}

uint32 DumpBall(LogType into, const uint8 *data, uint32 len) {
    uint32 init_len = len;

    const Destiny::BallHeader *ballhead = (const Destiny::BallHeader *) data;
    data += sizeof(Destiny::BallHeader);
    len -= sizeof(Destiny::BallHeader);

    if(ballhead->mode > Destiny::MAX_DSTBALL) {
        _log(into, "Error: Invalid ball mode %d for ball %d", ballhead->mode, ballhead->entityID);
        return(0);
    }

    _log(into, "AddBall: entity=%d, mode=%s (%d) subtype=%d",
        ballhead->entityID, Destiny::DSTBALL_modeNames[ballhead->mode], ballhead->mode, ballhead->sub_type);
    _log(into, "   x=%.3f, y=%.3f, z=%.3f, radius=%.2f",
        ballhead->x, ballhead->y, ballhead->z, ballhead->radius);

    if(ballhead->mode != Destiny::DSTBALL_RIGID) {
        const Destiny::MassSector *masschunk = (const Destiny::MassSector *) data;
        data += sizeof(Destiny::MassSector);
        len -= sizeof(Destiny::MassSector);

        _log(into, "   mass=%.2f, cloak=%d, hardmonic=%f, corp=%u, alliance=0x%" PRIx64,
            masschunk->mass, masschunk->cloak, masschunk->Harmonic, masschunk->corpID, masschunk->allianceID);
    }

    //this seems a little strange, but this is how it works...
    if(ballhead->sub_type & 0x1) {
        const Destiny::ShipSector *shipchunk = (const Destiny::ShipSector *) data;
        data += sizeof(Destiny::ShipSector);
        len -= sizeof(Destiny::ShipSector);

        _log(into, "   maxSpeed=%.2f, V=(%.3f, %.3f, %.3f) PS=%.4f, SF=%.3f",
            shipchunk->max_speed,
            shipchunk->velocity_x, shipchunk->velocity_y, shipchunk->velocity_z,
            shipchunk->agility,
            shipchunk->speed_fraction);
    }

    _log(into, "   %s:", Destiny::DSTBALL_modeNames[ballhead->mode]);
    switch(ballhead->mode) {
    case Destiny::DSTBALL_GOTO: {
        const Destiny::DSTBALL_GOTO_Struct *b = (const Destiny::DSTBALL_GOTO_Struct *) data;
        data += sizeof(Destiny::DSTBALL_GOTO_Struct);
        len -= sizeof(Destiny::DSTBALL_GOTO_Struct);
        _log(into, "       formID=%d, Goto=(%.3f, %.3f, %.3f)", b->formationID,
            b->x, b->y, b->z);
    } break;
    case Destiny::DSTBALL_FOLLOW: {
        const Destiny::DSTBALL_FOLLOW_Struct *b = (const Destiny::DSTBALL_FOLLOW_Struct *) data;
        data += sizeof(Destiny::DSTBALL_FOLLOW_Struct);
        len -= sizeof(Destiny::DSTBALL_FOLLOW_Struct);
        _log(into, "       formID=%d, followID=%u, distance=%.1f", b->formationID,
            b->followID, b->followRange);
    } break;
    case Destiny::DSTBALL_STOP: {
        const Destiny::DSTBALL_STOP_Struct *b = (const Destiny::DSTBALL_STOP_Struct *) data;
        data += sizeof(Destiny::DSTBALL_STOP_Struct);
        len -= sizeof(Destiny::DSTBALL_STOP_Struct);
        _log(into, "       formID=%d ", b->formationID);
    } break;
    case Destiny::DSTBALL_WARP: {
        const Destiny::DSTBALL_WARP_Struct *b = (const Destiny::DSTBALL_WARP_Struct *) data;
        data += sizeof(Destiny::DSTBALL_WARP_Struct);
        len -= sizeof(Destiny::DSTBALL_WARP_Struct);
        _log(into, "       formID=%d, To=(%.3f, %.3f, %.3f) effectStamp=%u", b->formationID,
            b->unknown_x, b->unknown_y, b->unknown_z, b->effectStamp);
        _log(into, "       followRange=%.3f, followID=%u, ownerID=%u",
            b->followRange, b->followID, b->ownerID);
    } break;
    case Destiny::DSTBALL_ORBIT: {
        const Destiny::DSTBALL_ORBIT_Struct *b = (const Destiny::DSTBALL_ORBIT_Struct *) data;
        data += sizeof(Destiny::DSTBALL_ORBIT_Struct);
        len -= sizeof(Destiny::DSTBALL_ORBIT_Struct);
        _log(into, "       formID=%d, orbitID=%u, distance=%.1f", b->formationID,
            b->followID, b->followRange);
    } break;
    case Destiny::DSTBALL_MISSILE: {
        const Destiny::DSTBALL_MISSILE_Struct *b = (const Destiny::DSTBALL_MISSILE_Struct *) data;
        data += sizeof(Destiny::DSTBALL_MISSILE_Struct);
        len -= sizeof(Destiny::DSTBALL_MISSILE_Struct);
        _log(into, "       formID=%d, target=%u, followRange?=%.1f, ownerID=%u, effectStamp=%u", b->formationID,
            b->followID, b->followRange, b->ownerID, b->effectStamp);
        _log(into, "       u=(%.3f, %.3f, %.3f)",
            b->x, b->y, b->z);
    } break;
    case Destiny::DSTBALL_MUSHROOM: {
        const Destiny::DSTBALL_MUSHROOM_Struct *b = (const Destiny::DSTBALL_MUSHROOM_Struct *) data;
        data += sizeof(Destiny::DSTBALL_MUSHROOM_Struct);
        len -= sizeof(Destiny::DSTBALL_MUSHROOM_Struct);
        _log(into, "       formID=%d, distance=%.3f, u125=%.3f, effectStamp=%u, ownerID=%u", b->formationID,
            b->followRange, b->unknown125, b->effectStamp, b->ownerID);
    } break;
    case Destiny::DSTBALL_BOID: {
        _log(into, "       NOT ALLOWED IN STREAM!");
        return(0);
    } break;
    case Destiny::DSTBALL_TROLL: {
        const Destiny::DSTBALL_TROLL_Struct *b = (const Destiny::DSTBALL_TROLL_Struct *) data;
        data += sizeof(Destiny::DSTBALL_TROLL_Struct);
        len -= sizeof(Destiny::DSTBALL_TROLL_Struct);
        _log(into, "       formID=%d, effectStamp=%u", b->formationID,
            b->effectStamp);
    } break;
    case Destiny::DSTBALL_MINIBALL: {
        _log(into, "       NOT ALLOWED IN STREAM!");
        return(0);
    } break;
    case Destiny::DSTBALL_FIELD: {
        const Destiny::DSTBALL_FIELD_Struct *b = (const Destiny::DSTBALL_FIELD_Struct *) data;
        data += sizeof(Destiny::DSTBALL_FIELD_Struct);
        len -= sizeof(Destiny::DSTBALL_FIELD_Struct);
        _log(into, "       formID=%d ", b->formationID);
    } break;
    case Destiny::DSTBALL_RIGID: {
        const Destiny::DSTBALL_RIGID_Struct *b = (const Destiny::DSTBALL_RIGID_Struct *) data;
        data += sizeof(Destiny::DSTBALL_RIGID_Struct);
        len -= sizeof(Destiny::DSTBALL_RIGID_Struct);
        _log(into, "       formID=%d ", b->formationID);
    } break;
    case Destiny::DSTBALL_FORMATION: {
        const Destiny::DSTBALL_FORMATION_Struct *b = (const Destiny::DSTBALL_FORMATION_Struct *) data;
        data += sizeof(Destiny::DSTBALL_FORMATION_Struct);
        len -= sizeof(Destiny::DSTBALL_FORMATION_Struct);
        _log(into, "       formID=%d, followID=%u, followRange=%.3f, effectStamp=%u", b->formationID,
            b->followID, b->followRange, b->effectStamp);
    } break;
    default:
        _log(into, "Error: Unknown ball mode %d!", ballhead->mode);
        _hex(into, data-sizeof(Destiny::BallHeader), (len>128)?128:(len+sizeof(Destiny::BallHeader)));
        return(0);
    }

    //not right:
    if( ( ballhead->sub_type == 64 ) || ( ballhead->sub_type == 66 ) )
    {
        const Destiny::MiniBallList* mbl = (const Destiny::MiniBallList*)data;
        data += sizeof( Destiny::MiniBallList );
        len -= sizeof( Destiny::MiniBallList );

        if( 0 < mbl->count )
        {
            _log( into, "    MiniBall Count: %d", mbl->count );

            for( uint16 r = 0; r < mbl->count; ++r )
            {
                const Destiny::MiniBall* mini = (const Destiny::MiniBall*)data;
                data += sizeof( Destiny::MiniBall );
                len -= sizeof( Destiny::MiniBall );

                _log( into, "        [%d] pos (%.3f, %.3f, %.3f) radius %.2f",
                      r, mini->x, mini->y, mini->z, mini->radius );
            }
        }
    }

    const Destiny::NameStruct *name = (const Destiny::NameStruct *) data;
    data += sizeof(Destiny::NameStruct);
    len -= sizeof(Destiny::NameStruct);
    if(name->name_len > 0) {
        _log(into, "   Name: len=%u", name->name_len);
        _log(into, "     %.*ls", name->name_len, name->name);

        data += name->name_len*sizeof(uint16);
        len  -= name->name_len*sizeof(uint16);
    }

    if(len > init_len) {
        _log(into, "ERROR: Consumed more bytes than given: had %d, used %d", init_len, len);
        return(init_len);
    }
    return(init_len - len);
}



}



















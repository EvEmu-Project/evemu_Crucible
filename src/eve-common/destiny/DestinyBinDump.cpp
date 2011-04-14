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
	Author:		Zhur
*/

#include "EVECommonPCH.h"

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
	_log(into, "AddBall: more=%d, sequence=%d (len %d)", global_head->more, global_head->sequence, len);
	data += sizeof(Destiny::AddBall_header);
	len -= sizeof(Destiny::AddBall_header);
		
	while(len > 0) {
		uint32 used = DumpBall(into, data, len);
		if(used == 0)
			return;	//error
		data += used;
		len -= used;
	}
}

uint32 DumpBall(LogType into, const uint8 *data, uint32 len) {
	uint32 init_len = len;
	
#ifndef OLD_DESTINY_DECODER
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
		
		_log(into, "   mass=%.2f, cloak=%d, u52=0x"I64x", corp=%u, alliance=0x%x",
			masschunk->mass, masschunk->cloak, masschunk->unknown52, masschunk->corpID, masschunk->allianceID);
	}
	
	//this seems a little strange, but this is how it works...
	if(ballhead->sub_type & 0x1) {
		const Destiny::ShipSector *shipchunk = (const Destiny::ShipSector *) data;
		data += sizeof(Destiny::ShipSector);
		len -= sizeof(Destiny::ShipSector);
		
		_log(into, "   maxSpeed=%.2f, V=(%.3f, %.3f, %.3f) unknown=(%.3f, %.3f, %.3f) PS=%.4f, SF=%.3f",
			shipchunk->max_speed,
            shipchunk->velocity_x, shipchunk->velocity_y, shipchunk->velocity_z,
            shipchunk->unknown_x, shipchunk->unknown_y, shipchunk->unknown_z,
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
	
#else
	const Destiny::AddBall_itemHeader *head = (const Destiny::AddBall_itemHeader *) data;
	data += sizeof(Destiny::AddBall_itemHeader);
	len -= sizeof(Destiny::AddBall_itemHeader);
	_log(into, "AddBall: entity=%d, type=%d",
		head->entityID, head->type);

	if(head->type == Destiny::AddBallType_ship) {
		const Destiny::AddBall_ship *ball = (const Destiny::AddBall_ship *) data;
		_log(into, " Ship:  x=%.3f, y=%.3f, z=%.3f, radius=%.2f, mass=%.2f",
			ball->x, ball->y, ball->z, ball->radius, ball->mass);
		_log(into, "   type=%d, u51=%d, u52=0x"I64x", corp=%u, u64=0x%x",
			ball->sub_type, ball->unknown51, ball->unknown52, ball->corpID, ball->unknown64);

		data += sizeof(*ball);
		len  -= sizeof(*ball);

		if(ball->sub_type == 1) {
			const Destiny::AddBall_ship1_cloud *sub_ball = (const Destiny::AddBall_ship1_cloud *) data;

			_log(into, "   u68=%.3f, u76=%.3f, u84=%.3f, u92=%.3f",
				sub_ball->unknown68, sub_ball->unknown76, sub_ball->unknown84, sub_ball->unknown92);
			_log(into, "   u100=%.3f, u108=%.3f",
				sub_ball->unknown100, sub_ball->unknown108);
			_log(into, "   formID=%d, Name: len=%d",
				sub_ball->formationID, sub_ball->name_len);
			_log(into, "   (cloud?) Name:");
			_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));
			
			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else if(ball->sub_type == 4) {
			const Destiny::AddBall_ship4 *sub_ball = (const Destiny::AddBall_ship4 *) data;
			
			_log(into, "   formID=%d, Name: len=%d", sub_ball->formationID, sub_ball->name_len);
			_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));

			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else if(ball->sub_type == 5 || ball->sub_type == 13) {
			const Destiny::AddBall_ship5_13 *sub_ball = (const Destiny::AddBall_ship5_13 *) data;

			_log(into, "   capacity=%.3f, ps=%.3f, u108=%.3f",
				sub_ball->capacity, sub_ball->propulsion_strength, sub_ball->unknown108);
			_log(into, "   formID=%d, namelen=%d, Zeros:",
				sub_ball->formationID, sub_ball->name_len);
			_hex(into, sub_ball->zeros, sizeof(sub_ball->zeros));
			_log(into, "   (ship?) Name:");
			_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));
			
			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else if(ball->sub_type == 9) {
			const Destiny::AddBall_ship9 *sub_ball = (const Destiny::AddBall_ship9 *) data;

			_log(into, "   radius=%.3f, u76=%.3f, u84=%.3f, u92=%.3f",
				sub_ball->radius, sub_ball->unknown76, sub_ball->unknown84, sub_ball->unknown92);
			_log(into, "   ps=%.3f, u108=%.3f",
				sub_ball->propulsion_strength, sub_ball->unknown108);
			_log(into, "   formID=%d, Name: len=%d",
				sub_ball->formationID, sub_ball->name_len);
			_log(into, "   Name:");
			_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));
			
			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else if(ball->sub_type == 64) {
			const Destiny::AddBall_ship64_collideable *sub_ball = (const Destiny::AddBall_ship64_collideable *) data;
			
			_log(into, "   formID=%d, Points: count=%d", sub_ball->formationID, sub_ball->name_len);

			int r;
			for(r = 0; r < sub_ball->point_count; r++) {
				_log(into, "    [%02d] (%.3f, %.3f, %.3f, %.3f)", 
					r, sub_ball->points[r*4 + 0],
					sub_ball->points[r*4 + 1],
					sub_ball->points[r*4 + 2],
					sub_ball->points[r*4 + 3] );
			}
			
			data += sub_ball->point_count*sizeof(double)*4;
			len  -= sub_ball->point_count*sizeof(double)*4;
			sub_ball = (const Destiny::AddBall_ship64_collideable *) data;

			//only the name fields are valid now!!
			
			_log(into, "   (collidable) Name: len=%d", sub_ball->name_len);
			_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));

			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else {
			_log(into, "Unknown ship sub-ball type %d!", ball->sub_type);
			_hex(into, data, (len>128)?128:len);
			return(0);
			
		}
	} else if(head->type == Destiny::AddBallType_unmannedShip) {
		const Destiny::AddBall_unmannedShip *ball = (const Destiny::AddBall_unmannedShip *) data;
		_log(into, " UnmannedShip:  x=%.3f, y=%.3f, z=%.3f, vol=%.2f, mass=%.2f",
			ball->x, ball->y, ball->z, ball->volume, ball->mass);
		_log(into, "   u42=%d, u51=%d, u52=0x"I64x", corp=%u, u64=0x%x",
			ball->unknown42, ball->unknown51, ball->unknown52, ball->corpID, ball->unknown64);

		_log(into, "   u68=%.3f, u76=%.3f, u84=%.3f, u92=%.3f",
			ball->unknown68, ball->unknown76, ball->unknown84, ball->unknown92);
		_log(into, "   ps=%.3f, u108=%.3f, formID=%d, loc=%u, u121=%.3f",
			ball->propulsion_strength, ball->unknown108, ball->formationID, ball->locationID, ball->unknown121);
		_log(into, "   Name: len=%d", ball->name_len);
		_hex(into, ball->name, ball->name_len*sizeof(uint16));
		
		data += sizeof(*ball) + ball->name_len*sizeof(uint16);
		len  -= sizeof(*ball) + ball->name_len*sizeof(uint16);
	} else if(head->type == Destiny::AddBallType_shipext2) {
		const Destiny::AddBall_shipext2 *ball = (const Destiny::AddBall_shipext2 *) data;
		_log(into, " ShipExt2:  x=%.3f, y=%.3f, z=%.3f, vol=%.2f, mass=%.2f",
			ball->x, ball->y, ball->z, ball->volume, ball->mass);
		_log(into, "   u42=%d, u51=%d, u52=0x"I64x", corp=%u, u64=0x%x",
			ball->unknown42, ball->unknown51, ball->unknown52, ball->corpID, ball->unknown64);

		_log(into, "   u68=%.3f, u76=%.3f, u84=%.3f, u92=%.3f",
			ball->unknown68, ball->unknown76, ball->unknown84, ball->unknown92);
		_log(into, "   ps=%.3f, u108=%.3f, formID=%d",
			ball->propulsion_strength, ball->unknown108, ball->formationID);
		_log(into, "   x2=%.3f, y2=%.3f, z2=%.3f",
			ball->x2, ball->y2, ball->z2);
		_log(into, "   Name: len=%d", ball->name_len);
		_hex(into, ball->name, ball->name_len*sizeof(uint16));
		
		data += sizeof(*ball) + ball->name_len*sizeof(uint16);
		len  -= sizeof(*ball) + ball->name_len*sizeof(uint16);
	} else if(head->type == Destiny::AddBallType_shipadd) {
		const Destiny::AddBall_shipadd *ball = (const Destiny::AddBall_shipadd *) data;
		_log(into, " ShipAdd:  x=%.3f, y=%.3f, z=%.3f, radius=%.2f, mass=%.2f",
			ball->x, ball->y, ball->z, ball->radius, ball->mass);
		_log(into, "   subtype=%d, u51=%d, u52=0x"I64x", corp=%u, u64=0x%x",
			ball->sub_type, ball->unknown51, ball->unknown52, ball->corpID, ball->unknown64);

		_log(into, "   radius=%.3f, dX=%.3f, dY=%.3f, dZ=%.3f",
			ball->radius, ball->headingX, ball->headingY, ball->headingZ);
		_log(into, "   ps=%.3f, u108=%.3f, formID=%d",
			ball->propulsion_strength, ball->unknown108, ball->formationID);
		_log(into, "   warpX=%.3f, warpY=%.3f, warpZ=%.3f",
			ball->warpToX, ball->warpToY, ball->warpToZ);
		_log(into, "   u143=0x%x, u147=0x%x, u151=0x%x, u155=0x%x, u159=0x%x",
			ball->unknown143, ball->unknown147, ball->unknown151, ball->unknown155, ball->unknown159);
		if(ball->name_len > 0) {
			_log(into, "   Name: len=%d", ball->name_len);
			_hex(into, ball->name, ball->name_len*sizeof(uint16));
		}
		
		data += sizeof(*ball) + ball->name_len*sizeof(uint16);
		len  -= sizeof(*ball) + ball->name_len*sizeof(uint16);
	} else if(head->type == Destiny::AddBallType_npc) {
		const Destiny::AddBall_npc *ball = (const Destiny::AddBall_npc *) data;
		_log(into, " NPC:  x=%.3f, y=%.3f, z=%.3f, radius=%.2f, mass=%.2f",
			ball->x, ball->y, ball->z, ball->radius, ball->mass);
		_log(into, "   u42=%d, u51=%d, u52=0x"I64x", corp=%u, u64=0x%x",
			ball->unknown42, ball->unknown51, ball->unknown52, ball->corpID, ball->unknown64);

		_log(into, "   u68=%.3f, u76=%.3f, u84=%.3f, u92=%.3f",
			ball->unknown68, ball->unknown76, ball->unknown84, ball->unknown92);
		_log(into, "   u100=%.3f, u108=%.3f, formID=%d, station=%u, u121=%.3f",
			ball->unknown100, ball->unknown108, ball->formationID, ball->stationID, ball->unknown121);
		if(ball->name_len > 0) {
			_log(into, "   Name: len=%d", ball->name_len);
			_hex(into, ball->name, ball->name_len*sizeof(uint16));
		}
		
		data += sizeof(*ball) + ball->name_len*sizeof(uint16);
		len  -= sizeof(*ball) + ball->name_len*sizeof(uint16);
	} else if(head->type == Destiny::AddBallType_loot) {
		const Destiny::AddBall_loot *ball = (const Destiny::AddBall_loot *) data;
		_log(into, " Loot:  x=%.3f, y=%.3f, z=%.3f, vol=%.2f, mass=%.2f",
			ball->x, ball->y, ball->z, ball->volume, ball->mass);
		_log(into, "   u42=%d, u51=%d, u52=0x"I64x", corp=%u, u64=0x%x",
			ball->unknown42, ball->unknown51, ball->unknown52, ball->corpID, ball->unknown64);

		_log(into, "   u68=%.3f, u76=%.3f, u84=%.3f, u92=%.3f",
			ball->unknown68, ball->unknown76, ball->unknown84, ball->unknown92);
		_log(into, "   u100=%.3f, u108=%.3f, formID=%d, u117=%u",
			ball->unknown100, ball->unknown108, ball->formationID, ball->unknown117);
		if(ball->name_len > 0) {
			_log(into, "   Name: len=%d", ball->name_len);
			_hex(into, ball->name, ball->name_len*sizeof(uint16));
		}
		
		data += sizeof(*ball) + ball->name_len*sizeof(uint16);
		len  -= sizeof(*ball) + ball->name_len*sizeof(uint16);
	} else if(head->type == Destiny::AddBallType_spaceItem) {
		
		const Destiny::AddBall_spaceItem *ball = (const Destiny::AddBall_spaceItem *) data;
		_log(into, " SpaceItem: x=%.3f, y=%.3f, z=%.3f, vol=%.2f, type=%d ",
			ball->x, ball->y, ball->z, ball->volume, ball->sub_type);

		data += sizeof(*ball);
		len  -= sizeof(*ball);

		if(   ball->sub_type == 4 	//cargo container?
		   || ball->sub_type == 2	//asteroid belt
		   || ball->sub_type == 6	//moon, planet, star
		) {
			const Destiny::AddBall_spaceItem_cargoContainer *sub_ball = (const Destiny::AddBall_spaceItem_cargoContainer *) data;
			_log(into, "   u43=0x%x, Name: len=%d", 
				sub_ball->unknown43, sub_ball->name_len);
			if(sub_ball->name_len > 0) {
				_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));
			}
			
			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else if(ball->sub_type == 66) {
			//stargate, station...
			const Destiny::AddBall_spaceItem_station *sub_ball = (const Destiny::AddBall_spaceItem_station *) data;
			
			_log(into, "   u43=%d, Points: count=%d", 
				sub_ball->unknown43, sub_ball->point_count);

			int r;
			for(r = 0; r < sub_ball->point_count; r++) {
				_log(into, "    [%02d] (%.3f, %.3f, %.3f, %.3f)", 
					r, sub_ball->points[r*4 + 0],
					sub_ball->points[r*4 + 1],
					sub_ball->points[r*4 + 2],
					sub_ball->points[r*4 + 3] );
			}
			
			data += (sub_ball->point_count*4 - 1)*sizeof(double);	//1 is already in the struct...
			len  -= (sub_ball->point_count*4 - 1)*sizeof(double);
			sub_ball = (const Destiny::AddBall_spaceItem_station *) data;

			//only the name fields are valid now!!
			
			_log(into, "   Name: len=%d", sub_ball->name_len);
			_hex(into, sub_ball->name, sub_ball->name_len*sizeof(uint16));

			data += sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
			len  -= sizeof(*sub_ball) + sub_ball->name_len*sizeof(uint16);
		} else {
			_log(into, "Unknown CC sub-ball type %d!", ball->sub_type);
			_hex(into, data, (len>128)?128:len);
			return(0);
		}
		

		
		
	} else {
		_log(into, "Unknown ball type %d!", head->type);
		_hex(into, data, (len>128)?128:len);
		return(0);
	}
#endif
	if(len > init_len) {
		_log(into, "ERROR: Consumed more bytes than given: had %d, used %d", init_len, len);
		return(init_len);
	}
	return(init_len - len);
}



}



















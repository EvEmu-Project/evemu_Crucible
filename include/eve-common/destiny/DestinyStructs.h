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

/*

some_factor = exp(- (space_friction * num1) / (mass * agility))

Vector vector_to_goal(current_position, goal_point);
distance_to_goal = vector_to_goal.length();
delta = ((num1 * speed_fraction * maxVelocity) + radius);
accel = (space_friction*speed_fraction*maxVelocity) / (agility * mass);
if(delta*delta >= distance_to_goal*distance_to_goal) {
    // we are going to reach our goal
    tmp = distance_to_goal/(delta*delta);
    accel *= tmp*tmp;
} else {
    vector_to_goal /= distance_to_goal; //normalize
}
Vector acceleration = vector_to_goal * accel;


*/

#ifndef __DESTINY_STRUCTS_H__
#define __DESTINY_STRUCTS_H__

#ifdef WIN32
	//VC++ whines a lot about the [0] structure sizes done in here...
	//#pragma warning(disable:4200)
#endif//WIN32

namespace Destiny {

#pragma pack(1)

struct AddBall_header {
/*000*/uint8 more;	//no idea...
/*001*/uint32 sequence;
};

#ifdef OLD_STRUCTS
struct AddBall_itemHeader {
/*005*/uint32 entityID;
/*009*/uint8 type;
};

enum { //destiny mode!
	AddBallType_shipext2        = 0,
	AddBallType_unmannedShip    = 1,	//name wrong.
	AddBallType_ship            = 2,	//this can be NPCs too... seems to be 'idle ship'
	AddBallType_shipadd         = 3,
	AddBallType_npc             = 4,	// really seems to be 'orbiting ship'
    AddBallType_unk1            = 5,
    AddBallType_unk2            = 6,
    AddBallType_unk3            = 7,
	AddBallType_loot            = 8,
    AddBallType_unk4            = 9,
    AddBallType_unk5            = 10,
	AddBallType_spaceItem       = 11,
	AddBallType_collideable     = 64
};
#endif//OLD_STRUCTS

typedef enum {
	DSTBALL_GOTO        = 0,    // Also used for AlignTo
	DSTBALL_FOLLOW      = 1,
	DSTBALL_STOP        = 2,
	DSTBALL_WARP        = 3,
	DSTBALL_ORBIT       = 4,
	DSTBALL_MISSILE     = 5,
	DSTBALL_MUSHROOM    = 6,    //Expanding gravity wall
	DSTBALL_BOID        = 7,    //Swarm like behavior
	DSTBALL_TROLL       = 8,    //used for ejected cans. Free ball that will become fixed after a while.
	DSTBALL_MINIBALL    = 9,
    DSTBALL_FIELD       = 10,    //Force field ball
	DSTBALL_RIGID       = 11,   //A ball that will never move, stations, etc..
	DSTBALL_FORMATION   = 12
} BallMode;

static const uint8 MAX_DSTBALL = DSTBALL_FORMATION;

/* capt: disabled for now
 * why: I dono
 */
#if 0

struct MiniballSet {
			struct miniball {
			} miniballs[point_count];
};
struct NameStruct {
/*117*/uint8  name_len;		//in 16 bit increments
/*118*/uint16 name[0]; 		//utf16
};
struct CommonTail {
	if(something != 64) {
		MiniballSet miniballs;
	}
	NameStruct name;
};

struct AddBall {
/*005*/uint32 entityID;
/*009*/uint8 mode;		//see DSTBALL_
/*010*/double radius;	//7.230769230769231e-3
/*018*/double x;	//1.33638303942e12
/*026*/double y;	//6.252761088e10
/*034*/double z;	//4.1517330432e11
/*042*/uint8  sub_type;		// seen 13
	if(mode != DSTBALL_RIGID) {
	/*043*/double mass;		//1.7e6
	/*051*/uint8  unknown51;		// seen 0
	/*052*/uint64 unknown52;	// seen all 1's
	/*060*/uint32 corpID;
	/*064*/uint32 unknown64;	// seen all 1's
	} //total offset 0x44

	if(sub_type & 0x1) {
/*068*/double max_speed;		//1.2e2
/*076*/double velocity_x;	//4.5e1  I think this is velocity...
/*084*/double velocity_y;	//0
/*092*/double velocity_z;	//0
/*100*/double propulsion_strength;	//3.037999998778105
/*108*/double speed_fraction;	// seen 1.0
		//this field gets multiplied by unknwon76 for something...
		// v28 = z - (unknown76_z*unknown108);
		// v20 = y - (unknown76_y*unknown108);
		// v18 = x - (unknown76_x*unknown108);
		// v10 = v20
		// v8 = v28
	}
/*116*/uint8  formationID;

	switch(mode) {
	case DSTBALL_GOTO:
/*117*/double x;		//object+0xD0 (as a set of 3)
/*125*/double y;
/*133*/double z;
		CommonTail;
		break;
	case DSTBALL_FOLLOW:
/*117*/	uint32 followID;	//object+0xBC
/*121*/	double distance;	//object+0xC0
		CommonTail;
		break;
	case DSTBALL_STOP:
	case DSTBALL_FIELD:
	case DSTBALL_RIGID:
		CommonTail;
		break;
	case DSTBALL_WARP:
/*117*/double unknown_x;	//object+0xD0 (as a set of 3)
/*125*/double unknown_y;
/*133*/double unknown_z;
/*141*/	uint32 unknown141;	//FOLLOW,MISSILE,etc, object+0xCC
/*145*/	double distance;	//object+0xC0
/*149*/	uint32 unknown149;	//object+0xBC
/*153*/	uint32 unknown153;	//object+0xC8
		CommonTail;
		break;
	case DSTBALL_ORBIT:
/*117*/	uint32 orbitID;		//object+0xBC
/*121*/	double distance;	//object+0xC0
		CommonTail;
		break;
	case DSTBALL_MISSILE:
/*117*/	uint32 unknown117;	//same loc as FOLLOW followID, object+0xBC
/*121*/	double unknown121;	//same loc as FOLLOW distance, object+0xC0
/*129*/	uint32 unknown129;	//object+0xC8
/*133*/	uint32 unknown133;	//same loc as TROLL unknown117, object+0xCC
/*137*/double x;		//object+0xD0 (as a set of 3)
/*145*/double y;
/*143*/double z;
		CommonTail;
		break;
	case DSTBALL_MUSHROOM:
/*117*/	double unknown117;	//object+0xC0
/*125*/	double unknown125;	//maybe object+0xD0
/*133*/	uint32 unknown133;	//same loc as TROLL unknown117, object+0xCC
/*137*/	uint32 unknown137;	//object+0xC8
		CommonTail;
		break;
	case DSTBALL_BOID:
		//unsupported in stream.
		break;
	case DSTBALL_MINIBALL:
		//unsupported in stream.
		break;
	case DSTBALL_TROLL:
/*117*/	uint32 unknown117;	//common with FORMATION unknown129, object+0xCC
		CommonTail;
		break;
	case DSTBALL_FORMATION:
/*117*/	uint32 unknown117;	//same loc as FOLLOW followID, object+0xBC
/*121*/	double unknown121;	//same loc as FOLLOW distance, object+0xC0
/*129*/	uint32 unknown129;	//same loc as TROLL unknown117, object+0xCC
		CommonTail;
		break;
	}

	//in ball->SetMass, they did:
	//	v8 = mass*propulsion_strength
	// 	v18 = 1.0 * 1.0e6 * -v8 = -0.1936
	// 	something = exp(v18); = .0823966

//unconfirmed:
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint8  name_len;		//in 16 bit increments
/*118*/uint16 name[0]; 		//utf16
};

#endif//disabled

#ifdef OLD_STRUCTS

struct AddBall_spaceItem {	//DSTBALL_RIGID
/*000*/AddBall_itemHeader head;
/*010*/double volume;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  sub_type;
};

enum {
	AddBallType_spaceItem_asteroidBelt = 2,
	AddBallType_spaceItem_cargoContainer = 4,
	AddBallType_spaceItem_planet = 6,	//star & moon too
	AddBallType_spaceItem_station = 66	//stargate too
};

struct AddBall_spaceItem_cargoContainer {
/*043*/uint8  unknown43;		//seen 0xFF
/*044*/uint8  name_len;		//in 16 bit increments
/*045*/uint16 name[0]; 		//utf16
};

struct AddBall_spaceItem_asteroidBelt {
/*043*/uint8  unknown43;		//seen 0xFF
/*044*/uint8  name_len;		//in 16 bit increments
/*045*/uint16 name[0]; 		//utf16
};

struct AddBall_spaceItem_planet {
/*043*/uint8  unknown43;		//seen 0xFF
/*044*/uint8  name_len;		//in 16 bit increments
/*045*/uint16 name[0]; 		//utf16
};

struct AddBall_spaceItem_station {
/*043*/uint8  unknown43;		//seen 0xFF
/*044*/uint16 point_count;
/*046*/double points[1];	//point_count*4 of them... (windows doesn't like two [0] arrays in one struct)
							// I think this is (x,y,z,radius) for miniballs w/ relative coordinates.
/*069*/uint8  name_len;		//in 16 bit increments
/*070*/uint16 name[0]; 		//utf16
};

struct AddBall_ship {
/*000*/AddBall_itemHeader head;
/*010*/double radius;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  sub_type;		// seen 13
/*043*/double mass;
/*051*/uint8  unknown51;		// seen 0
/*052*/uint64 unknown52;	// seen all 1's
/*060*/uint32 corpID;
/*064*/uint32 unknown64;	// seen all 1's
};

struct AddBall_ship4 {
/*068*/uint8  formationID;	// seen 0xFF
/*069*/uint8  name_len;		//in 16 bit increments
/*070*/uint16 name[0]; 		//utf16
};

struct AddBall_ship1_cloud {
/*068*/double unknown68;	//1.0
/*076*/double unknown76;	//0
/*084*/double unknown84;	//0
/*092*/double unknown92;	//0
/*100*/double unknown100;	//1.0
/*108*/double unknown108;	//1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint8  name_len;		//in 16 bit increments
/*118*/uint16 name[0]; 		//utf16
};

struct AddBall_ship5_13 {	//player?
/*068*/double capacity;		//guessed, seen 260
/*076*/uint8  zeros[24];
/*100*/double propulsion_strength;
/*108*/double unknown108;	// seen 1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint8  name_len;		//in 16 bit increments
/*118*/uint16 name[0]; 		//utf16
};

struct AddBall_ship5_13_build {
	AddBall_header head;
	AddBall_ship ship;
	AddBall_ship5_13 sub;
};

struct AddBall_ship9 {
/*068*/double radius;		//~287
/*076*/double unknown76;	//0
/*084*/double unknown84;	//0
/*092*/double unknown92;	//-94.7346982656287
/*100*/double propulsion_strength;	//? 2.6226000000000
/*108*/double unknown108;	// seen 1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint8 name_len;		//in 16 bit increments
/*118*/uint16 name[0]; 		//utf16
};

#ifndef WIN32 /*windows doesn't like two [0] in a struct.. */
struct AddBall_ship64_collideable {
/*068*/uint8  formationID;	// seen 0xFF
/*069*/uint16 point_count;
/*073*/double points[0];	//point_count*4 of them...
/*069*/uint8  name_len;		//in 16 bit increments
/*070*/uint16 name[0]; 		//utf16
};
#endif//WIN32

struct AddBall_unmannedShip {	//name wrong.
/*000*/AddBall_itemHeader head;
/*010*/double volume;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  unknown42;		// seen 13
/*043*/double mass;
/*051*/uint8  unknown51;		// seen 0
/*052*/uint64 unknown52;	// seen all 1's
/*060*/uint32 corpID;
/*064*/uint32 unknown64;	// seen all 1's
/*068*/double unknown68;	//seen 260.0
/*076*/double unknown76;	//seen 0.0004687178610
/*084*/double unknown84;	//seen -0.0001796522749
/*092*/double unknown92;	//seen  0.0000664194042
/*100*/double propulsion_strength;
/*108*/double unknown108;	// seen 1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint32 locationID;	// ??? seen 2100000086/60004420
/*121*/double unknown121;	// seen 500.0
/*117*/uint8 name_len;		//in 16 bit increments
/*118*/uint16 name[0]; 		//utf16
};

struct AddBall_shipext2 {
/*000*/AddBall_itemHeader head;
/*010*/double volume;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  unknown42;		// seen 13
/*043*/double mass;
/*051*/uint8  unknown51;		// seen 0
/*052*/uint64 unknown52;	// seen all 1's
/*060*/uint32 corpID;
/*064*/uint32 unknown64;	// seen all 1's
/*068*/double unknown68;	//seen 260.0
/*076*/double unknown76;	//seen 0.0004687178610
/*084*/double unknown84;	//seen -0.0001796522749
/*092*/double unknown92;	//seen  0.0000664194042
/*100*/double propulsion_strength;	//?
/*108*/double unknown108;	// seen 1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/double x2;
/*125*/double y2;
/*133*/double z2;
/*141*/uint8  name_len;		//in 16 bit increments
/*142*/uint16 name[0]; 		//utf16
};

//this is a warping ship I think...
struct AddBall_shipadd {
/*000*/AddBall_itemHeader head;
/*010*/double radius;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  sub_type;		// seen 13
/*043*/double mass;
/*051*/uint8  unknown51;		// seen 0
/*052*/uint64 unknown52;	// seen all 1's
/*060*/uint32 corpID;
/*064*/uint32 unknown64;	// seen all 1's
/*068*/double capacity;	//guessed... seen 283.
/*076*/double headingX;	//seen 184272.3712161785500
/*084*/double headingY;	//seen 8607.9384411140636
/*092*/double headingZ;	//seen -152138.0674139560100
/*100*/double propulsion_strength;
/*108*/double unknown108;	// seen 1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/double warpToX;
/*125*/double warpToY;
/*133*/double warpToZ;
/*143*/uint32 unknown143;	//seen 39385
/*147*/uint32 unknown147;
/*151*/uint32 unknown151;
/*155*/uint32 unknown155;	//possibly a float, seen 15000 in float.
/*159*/uint32 unknown159;
/*163*/uint8  name_len;		//in 16 bit increments
/*164*/uint16 name[0]; 		//utf16
};

struct AddBall_npc {
/*000*/AddBall_itemHeader head;
/*010*/double radius;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  unknown42;		// seen 5, probably a type
/*043*/double mass;
/*051*/uint8  unknown51;		// seen 0
/*052*/uint64 unknown52;	// seen -2
/*060*/uint32 corpID;		//? seen -1
/*064*/uint32 unknown64;	// seen all 1's
/*068*/double unknown68;	//seen 350.0
/*076*/double unknown76;	//seen -66.8146951144018
/*084*/double unknown84;	//seen 135.4525106240360
/*092*/double unknown92;	//seen 52.6178482184676
/*100*/double unknown100;	// seen 1.0
/*108*/double unknown108;	// seen 0.4571428571429
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint32 stationID;	//? seen 60004420	orbit? owner?
/*121*/double unknown121;	// seen 12000.0000000000000
/*129*/uint8  name_len;		//in 16 bit increments
/*130*/uint16 name[0]; 		//utf16
};
	
struct AddBall_loot {
/*000*/AddBall_itemHeader head;
/*010*/double volume;
/*018*/double x;
/*026*/double y;
/*034*/double z;
/*042*/uint8  unknown42;		// seen 13
/*043*/double mass;
/*051*/uint8  unknown51;		// seen 0
/*052*/uint64 unknown52;	// seen all 1's
/*060*/uint32 corpID;
/*064*/uint32 unknown64;	// seen all 1's
/*068*/double unknown68;	//1.0
/*076*/double unknown76;	//0
/*084*/double unknown84;	//0
/*092*/double unknown92;	//0
/*100*/double unknown100;	//1.0
/*108*/double unknown108;	//1.0
/*116*/uint8  formationID;	// seen 0xFF
/*117*/uint32 unknown117;	//seen 48487
/*121*/uint8  name_len;		//in 16 bit increments
/*122*/uint16 name[0]; 		//utf16
};

#endif//OLD_STRUCTS

enum ball_sub_type
{
    IsFree = 0x01,          // set if ball is free to move, has extra BallData
    IsGlobal = 0x02,        // set if ball should be visible from all
    IsMassive = 0x04,       // set if ball is solid
    IsInteractive = 0x08,   // set if ball is interactive
    IsMoribund = 0x10,      // set if ball is dieing.. this is a rough guess..
    HasMiniBalls = 0x40,    // if set, the reader tries to read extra mini balls
};

/**
 * a orbital asteroid entity has sub_type IsFree | IsMassive
 */

/** old nd shouldn't be used
enum {
    AddBallSubType_asteroidBelt = 2,	//seen rigids
    AddBallSubType_cargoContainer_asteroid = 4,	//and some NPCs and asteroids
    AddBallSubType_orbitingNPC = 5,
    AddBallSubType_planet = 6,	//star & moon too
    //subtype 8 gets put on some list during evolution.
    //seen a player with 9.
    AddBallSubType_player = 13,
    AddBallSubType_dungeonEntrance = 64,
    AddBallSubType_station = 66	//stargate too
};
*/

struct BallHeader {
    uint32 entityID;
    uint8 mode;		//see DSTBALL_
    double radius;	//7.230769230769231e-3
    double x;	//1.33638303942e12
    double y;	//6.252761088e10
    double z;	//4.1517330432e11
    uint8  sub_type;		// seen 13
};

//included if not a RIGID body
struct MassSector {
    double mass;
    uint8  cloak;		// indicates cloak
    uint64 unknown52;	// seen all 1's  "harmonic_30"
    uint32 corpID;		// may be all 1's
    uint32 allianceID;	// seen all 1's
};

//only included if the thing can move...
struct ShipSector {
    double max_speed;
    double velocity_x;
    double velocity_y;
    double velocity_z;
    double unknown_x; //might not really be x at all
    double unknown_y; //might not really be y at all
    double unknown_z; //might not really be z at all
    double agility;	//3.037999998778105 = object+0x0A8
    double speed_fraction;
};

struct MiniBall {
    double x;
    double y;
    double z;
    double radius;
};

struct MiniBallList {
	uint16 count;
	MiniBall balls[0];
};

struct NameStruct {
	uint8  name_len;		//in 16 bit increments
	uint16 name[0]; 		//utf16
};

struct DSTBALL_GOTO_Struct {
    uint8  formationID;
    double x;		//object+0xD0 (as a set of 3)
    double y;
    double z;
};

struct DSTBALL_FOLLOW_Struct {
    uint8  formationID;
    uint32 followID;
    double followRange;
};

struct DSTBALL_STOP_Struct {
    uint8  formationID;
};

struct DSTBALL_WARP_Struct {
    uint8  formationID;
    double unknown_x;	//object+0xD0 (as a set of 3)
    double unknown_y;
    double unknown_z;
    uint32 effectStamp;	//might be a destiny sequence number (back)
    double followRange;
    uint32 followID;
    uint32 ownerID;
};

struct DSTBALL_ORBIT_Struct {
    uint8  formationID;
    uint32 followID;	//orbitID
    double followRange;
};

struct DSTBALL_MISSILE_Struct {
    uint8  formationID;
    uint32 followID;	//targetID
    double followRange;
    uint32 ownerID;
    uint32 effectStamp;
    double x;
    double y;
    double z;
};

struct DSTBALL_MUSHROOM_Struct {
    uint8  formationID;
    double followRange;
    double unknown125;
    uint32 effectStamp;
    uint32 ownerID;
};

struct DSTBALL_TROLL_Struct {
    uint8  formationID;
    uint32 effectStamp;	//is a destiny sequence number (forward)
};

struct DSTBALL_FIELD_Struct {
    uint8  formationID;
};

struct DSTBALL_RIGID_Struct {
    uint8  formationID;
/* there is an optional 2 byte thing here, if it is non-zero. not sure what it
   is yet. */
};

struct DSTBALL_FORMATION_Struct {
    uint8  formationID;
    uint32 followID;
    double followRange;
    uint32 effectStamp;	//is a destiny sequence number (forward?)
};

union SpecificSectors {
	struct DSTBALL_GOTO_Struct GOTO;
	struct DSTBALL_FOLLOW_Struct FOLLOW;
	struct DSTBALL_STOP_Struct STOP;
	struct DSTBALL_WARP_Struct WARP;
	struct DSTBALL_ORBIT_Struct ORBIT;
	struct DSTBALL_MISSILE_Struct MISSILE;
	struct DSTBALL_MUSHROOM_Struct MUSHROOM;
	struct DSTBALL_TROLL_Struct TROLL;
	struct DSTBALL_FIELD_Struct FIELD;
	struct DSTBALL_RIGID_Struct RIGID;
	struct DSTBALL_FORMATION_Struct FORMATION;
};

struct TotalStruct {
	struct BallHeader head;
	struct MassSector mass;
	struct ShipSector ship;
	union SpecificSectors specific;
};


#pragma pack()

	
} //end Destiny

#endif

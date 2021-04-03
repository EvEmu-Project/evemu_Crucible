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

#ifndef __DESTINY_STRUCTS_H__
#define __DESTINY_STRUCTS_H__

namespace Destiny {

    static const uint8 MAX_DSTBALL = 12;

    namespace Ball {
        namespace Mode {
            enum {
                GOTO        = 0,    // Also used for AlignTo
                FOLLOW      = 1,
                STOP        = 2,
                WARP        = 3,
                ORBIT       = 4,
                MISSILE     = 5,
                MUSHROOM    = 6,    //Expanding gravity wall (bomb?)
                BOID        = 7,    //Swarm-like behavior, based on birds (flocks) but could also be herds, and schools
                TROLL       = 8,    //used for jetcans and wrecks (only?). Free ball that will become fixed after a while.
                MINIBALL    = 9,    //Guns/Sentry
                FIELD       = 10,   //Force field ball
                RIGID       = 11,   //A ball that will never move, stations, etc..
                FORMATION   = 12
            };
        }

        namespace Flag {
            enum  {
                IsFree          = 0x01,   // ball is free to move, has DataSector
                IsGlobal        = 0x02,   // ball is be visible system-wide (shows in overview like static items)
                IsMassive       = 0x04,   // ball is solid (collidable)
                IsInteractive   = 0x08,   // ball is interactive - piloted ships, containers in space
                IsMoribund      = 0x10,   // ball is dying.
                HasMiniBalls    = 0x40,   // if set, the client expects mini balls...not sure *what* miniballs are....collision data maybe?
            };
        }
        /*
         * not sure what these are for yet.  calls in destiny.dll ?
DST_CREATE = 1
DST_DESTROY = 2
DST_PROXIMITY = 3
DST_PRETICK = 4
DST_POSTTICK = 5
DST_COLLISION = 6
DST_RANGE = 7
DST_MODECHANGE = 8
DST_PARTITION = 9
DST_WARPACTIVATION = 10
DST_WARPEXIT = 11
*/
    }

/** @note  this file MUST use packed data, and variable types must remain as-is
 * client will not recognize it with byte-ordered padding, and expects values to be at specific locations
 * (error: malformed packet)
 */
#pragma pack(1)

struct AddBall_header {
    uint8 packet_type;  /* 0 = full state, 1 = balls */
    uint32 stamp;  /* statestamp */
};

struct NameSector {
    uint8  name_len;        //in 16 bit increments
    uint16 name[0];         //utf16
};

struct BallHeader {
    int64 entityID;
    uint8 mode;
    float radius;
    double posX;
    double posY;
    double posZ;
    uint8 flags;
};

//included if not a RIGID body
struct MassSector {
    double mass;
    uint8  cloak;        // indicates cloak
    int64 allianceID;
    int32 corporationID;
    int32 harmonic;
};

//only included if the thing can move...
struct DataSector {
    float maxSpeed;
    double velX;        // these *MAY* be heading in degrees
    double velY;
    double velZ;
    float inertia;
    float speedfraction;
};

struct MiniBall {
    double posX;
    double posY;
    double posZ;
    float radius;
};

struct MiniBallList {
    uint16 count;
    MiniBall balls[0];
};

struct GOTO_Struct {
    uint8  formationID;
    double x;        //object+0xD0 (as a set of 3)
    double y;
    double z;
};

struct FOLLOW_Struct {
    uint8  formationID;
    int64 followID;
    float followRange;
};

struct STOP_Struct {
    uint8  formationID;
};

struct WARP_Struct {
    uint8  formationID;
    double targX;             //object+0xD0 (as a set of 3)
    double targY;
    double targZ;
    int32 effectStamp;   //statestamp of when warp started
    int64 followRange;   //unknown   -4616189618054758400 when warp is initiated.  calculation unknown for other values (used during warp)
    int64 followID;      //unknown   4669471951536783360 when warp is initiated or ship enters new bubble (AddBalls), 0 otherwise
    int32 speed;
};

struct ORBIT_Struct {
    uint8  formationID;
    uint32 targetID;
    double followRange;
};

struct MISSILE_Struct {
    uint8  formationID;
    int64 targetID;
    float followRange;
    int64 ownerID;
    int32 effectStamp;
    double x;
    double y;
    double z;
};

struct MUSHROOM_Struct {
    uint8  formationID;
    float followRange;
    double unknown125;
    int32 effectStamp;
    int64 ownerID;
};

struct TROLL_Struct {
    uint8  formationID;
    uint32 effectStamp;   //statestamp of when effect started
};

struct FIELD_Struct {
    uint8  formationID;
};

struct RIGID_Struct {
    uint8  formationID;
/* there is an optional 2 byte thing here, if it is non-zero. not sure what it
   is yet. */
};

struct FORMATION_Struct {
    uint8  formationID;
    int64 followID;
    float followRange;
    int32 effectStamp;  //statestamp of when warp started
};

union SpecificSectors {
    struct GOTO_Struct GOTO;
    struct FOLLOW_Struct FOLLOW;
    struct STOP_Struct STOP;
    struct WARP_Struct WARP;
    struct ORBIT_Struct ORBIT;
    struct MISSILE_Struct MISSILE;
    struct MUSHROOM_Struct MUSHROOM;
    struct TROLL_Struct TROLL;
    struct FIELD_Struct FIELD;
    struct RIGID_Struct RIGID;
    struct FORMATION_Struct FORMATION;
};

struct TotalStruct {
    struct BallHeader head;
    struct MassSector mass;
    struct DataSector ship;
    union SpecificSectors specific;
};


#pragma pack()


} //end Destiny

#endif

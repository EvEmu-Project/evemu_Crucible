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
*/

/* Shared pieces of the client's movement simulation (the Crucible client's destiny.dll).
 *
 * DestinyManager simulates each ball's own movement.  This file holds the parts that need more than one ball:
 *  ball-to-ball collisions (which are also how missile impacts are found), plus the low-level equations both use.
 *
 * The client only runs collisions for massive ("solid") balls, and only against other massive, uncloaked,
 *  non-missile balls it knows about (its ballpark: the bubble's balls plus the system's static balls).
 *  A collision does not move anything directly.  It sets an extra acceleration that is added to the ball's
 *  thrust for this tick's integration step.
 * Missiles are flagged massive by the LaunchMissile update, and only ever test against their own target.
 *  The tick a missile's path touches its target is the tick it hits.
 */

#ifndef __DESTINYCOLLISION_H_INCL__
#define __DESTINYCOLLISION_H_INCL__

#include "eve-compat.h"

class SystemBubble;
class SystemEntity;
class SystemManager;

namespace Destiny {

    // client ballpark constants (Ballpark constructor in destiny.dll)
    static const double FRICTION = 1000000.0;
    static const double TICK = 1.0;                 // seconds per tick
    static const int32 TICK_MS = 1000;              // the same, in the integer milliseconds the client also uses
    static const double FIXED_MASS = 1.0e34;        // mass the collision code uses for balls that aren't free to move

    // Ballpark::Integrate.  exact solution for constant acceleration against linear drag over time t.
    //  m is mass * agility, and dragFactor is the ball's cached exp(-friction * TICK / m), which is only valid
    //  for t == TICK.  for any other t the client works it out again
    void Integrate(GPoint& pos, GVector& vel, const GVector& accel, double m, double dragFactor, double t);

    // drag factor for a full tick, as the client caches it (Ball+0x80)
    double DragFactor(double m);

    // missiles get more agile (lighter) as they speed up.  this is the agility the client uses for a ball in MISSILE mode
    double MissileAgility(double agility, const GVector& vel);

    /* a ball, as the client's collision code sees it */
    struct CollisionBall {
        int64 id = 0;
        uint8 mode = 0;
        bool isFree = false;
        bool isMassive = false;
        bool isCloaked = false;
        double radius = 0.0;            // client keeps these three as floats
        double maxVelocity = 0.0;
        double agility = 1.0;
        double mass = 0.0;              // raw mass, as sent in the ball's MassSector
        double dragFactor = 1.0;
        GPoint pos;
        GVector vel;
        GVector thrust;                 // this tick's acceleration from EvolveThrust() (Ball+0x168)
        int64 followID = 0;             // target, for missiles
        int64 ownerID = 0;              // launcher, for missiles
        SystemEntity* pSE = nullptr;

        // mass * agility used in the equations of motion, or FIXED_MASS for balls that can't move
        double CollisionMass() const;
    };

    // result of the collision checks for one ball this tick (Ball+0x180 and +0x88)
    struct CollisionResult {
        GVector accel;                  // extra acceleration for this tick's integration
        double time = -1.0;             // fraction of the tick at which the collision that set 'accel' happens
        bool hitFollowTarget = false;   // a missile's path touches its target this tick
    };

    // Ballpark::Collide (destiny.dll 0x1001d9e0).  checks ball 'a' against 'b' over the coming tick, and records the
    //  acceleration that bounces 'a' off 'b' if it is the earliest collision so far.  only 'a' is affected
    void Collide(CollisionBall a, CollisionBall b, int depth, CollisionResult& result);

    /* the client's view of the balls in a system for one tick.  built once per tick by SystemManager::ProcessDestiny(),
     *  after all thrusts are known and before anything moves */
    class BallparkSnapshot {
    public:
        explicit BallparkSnapshot(SystemManager* pSystem);

        // ball data for an entity, as the client has it.  returns nullptr if the entity can't be encoded
        const CollisionBall* Get(SystemEntity* pSE);

        // every massive, uncloaked, non-missile ball a client in this bubble knows about, in id order
        const std::vector<const CollisionBall*>& GetMassiveBalls(SystemBubble* pBubble);

    private:
        SystemManager* m_system;
        std::map<uint32, CollisionBall> m_balls;
        std::map<SystemBubble*, std::vector<const CollisionBall*>> m_massive;
    };
}

#endif  // __DESTINYCOLLISION_H_INCL__

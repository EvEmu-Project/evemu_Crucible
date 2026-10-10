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

/* Ported from the Crucible client's destiny.dll.  The arithmetic is written out component by component, in the
 *  client's order, so that both sides round identically.  Please keep it that way when editing.
 */

#include "eve-server.h"

#include "destiny/DestinyStructs.h"
#include "system/DestinyCollision.h"
#include "system/DestinyManager.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "utils/Buffer.h"
#include <cmath>
#include <cstring>

namespace Destiny {

void Integrate(GPoint& pos, GVector& vel, const GVector& accel, double m, double dragFactor, double t) {
    if (t == 0.0)
        return;
    const double k = FRICTION;
    double tf = dragFactor;
    if (t != TICK)
        tf = std::exp(-((k / m) * t));

    const double k2 = k * k;
    const double tfm1 = tf - 1.0;
    const double ook2 = 1.0 / k2;
    pos.x = ((accel.x * m * tfm1 + ((vel.x + accel.x * t) - vel.x * tf) * k) * m + k2 * pos.x) * ook2;
    pos.y = ((accel.y * m * tfm1 + ((vel.y + accel.y * t) - vel.y * tf) * k) * m + k2 * pos.y) * ook2;
    pos.z = ((accel.z * m * tfm1 + ((vel.z + accel.z * t) - vel.z * tf) * k) * m + k2 * pos.z) * ook2;

    const double ook = 1.0 / k;
    vel.x = (accel.x * m - (accel.x * m - vel.x * k) * tf) * ook;
    vel.y = (accel.y * m - (accel.y * m - vel.y * k) * tf) * ook;
    vel.z = (accel.z * m - (accel.z * m - vel.z * k) * tf) * ook;
}

// Ball::UpdateDragFactor (destiny.dll 0x10021f60)
double DragFactor(double m) {
    if (m <= 0.0)
        return 0.0;
    return std::exp(-((TICK * FRICTION) / m));
}

double MissileAgility(double agility, const GVector& vel) {
    double speed = std::sqrt((vel.x * vel.x + vel.y * vel.y) + vel.z * vel.z);
    double result = (std::tanh((1500.0 - speed) * 0.0035) + 1.0) * 0.5 * agility;
    if (1e-05 > result)
        result = 1e-05;
    return result;
}

double CollisionBall::CollisionMass() const {
    if (!isFree)
        return FIXED_MASS;
    if (mode == Ball::Mode::MISSILE)
        return mass * MissileAgility(agility, vel);
    return mass * agility;
}

/* the client predicts both balls' motion over the tick and finds when (if ever) their spheres first touch.  then:
 *   - moving into each other:  works out the acceleration that gives 'a' the velocity it would have after bouncing
 *       off 'b' at that moment, mass-weighted if 'b' can move, or a mirror bounce if it can't
 *   - already overlapping:  if both can move, it checks once more with them pushed apart, then pushes 'a' back out
 *       by its share of the overlap
 * the result is scaled by 0.85, and only kept if it is the earliest collision (the strongest one on a tie)
 */
void Collide(CollisionBall a, CollisionBall b, int depth, CollisionResult& result) {
    const double k = FRICTION;
    const double dt = TICK;
    const double radii = a.radius + b.radius;
    const double massA = a.CollisionMass();
    const double massB = b.CollisionMass();

    // where each would be at the end of the tick under its own thrust
    GPoint aEnd(a.pos), bEnd(b.pos);
    GVector aVel(a.vel), bVel(b.vel);
    Integrate(aEnd, aVel, a.thrust, massA, a.dragFactor, dt);
    Integrate(bEnd, bVel, b.thrust, massB, b.dragFactor, dt);

    const double dx = a.pos.x - b.pos.x;
    const double dy = a.pos.y - b.pos.y;
    const double dz = a.pos.z - b.pos.z;
    const double rvx = (aEnd.x - a.pos.x) - (bEnd.x - b.pos.x);
    const double rvy = (aEnd.y - a.pos.y) - (bEnd.y - b.pos.y);
    const double rvz = (aEnd.z - a.pos.z) - (bEnd.z - b.pos.z);
    const double dist2 = (dx * dx + dy * dy) + dz * dz;
    const double qa = (rvx * rvx + rvy * rvy) + rvz * rvz;
    const double qb = ((rvx * dx + rvy * dy) + rvz * dz) * 2.0;

    // time of first contact, as a fraction of the tick.  0 if they already overlap
    double time = -1.0;
    bool solved = false;
    if (qa != 0.0) {
        const double radii2 = radii * radii;
        const double disc = qb * qb - (dist2 - radii2) * (qa * 4.0);
        if (!(0.0 > disc)) {
            solved = true;
            const double root = std::sqrt(disc);
            const double t1 = ((root - qb) * 0.5) / qa;
            const double t2 = ((-qb - root) * 0.5) / qa;
            if ((t1 >= 0.0) and (t1 <= 1.0))
                time = t1;
            if ((t2 >= 0.0) and (t2 <= 1.0) and (t1 > t2))
                time = t2;
            if (radii2 > dist2) {
                time = 0.0;
            } else if (0.0 > time) {
                return;
            }
        }
    }
    if (!solved) {
        if (!(radii * radii > dist2))
            return;
        time = 0.0;
    }

    GVector accel;
    if (time > 0.0) {
        // move both to the moment of contact
        const double tc = dt * time;
        GPoint aPos(a.pos), bPos(b.pos);
        GVector aV(a.vel), bV(b.vel);
        Integrate(aPos, aV, a.thrust, massA, a.dragFactor, tc);
        Integrate(bPos, bV, b.thrust, massB, b.dragFactor, tc);

        GVector n(bPos.x - aPos.x, bPos.y - aPos.y, bPos.z - aPos.z);
        const double len = std::sqrt((n.x * n.x + n.y * n.y) + n.z * n.z);
        if (len != 0.0) {
            const double inv = 1.0 / len;
            n.x *= inv;
            n.y *= inv;
            n.z *= inv;
        }

        // bounce
        const double vn = (n.x * aV.x + n.y * aV.y) + n.z * aV.z;
        if (!b.isFree) {
            const double f = vn * 2.0;
            aV.x = aV.x - n.x * f;
            aV.y = aV.y - n.y * f;
            aV.z = aV.z - n.z * f;
        } else {
            const double vnB = (n.x * bV.x + n.y * bV.y) + n.z * bV.z;
            const double q = ((vnB * (b.mass * 2.0) + (a.mass * vn - b.mass * vn)) / (b.mass + a.mass)) - vn;
            aV.x = q * n.x + aV.x;
            aV.y = n.y * q + aV.y;
            aV.z = n.z * q + aV.z;
        }

        // and on to the end of the tick
        Integrate(aPos, aV, a.thrust, massA, a.dragFactor, (1.0 - time) * dt);

        // the extra acceleration over a whole tick that ends with that velocity
        const double tf = a.dragFactor;
        const double tfm = tf * massA;
        const double px = aV.x * k, py = aV.y * k, pz = aV.z * k;
        const double qx = (tf * a.vel.x) * k, qy = (a.vel.y * tf) * k, qz = (a.vel.z * tf) * k;
        const double nx = -((((-massA * a.thrust.x) + a.thrust.x * tfm) - qx) + px);
        const double ny = -((((-massA * a.thrust.y) + a.thrust.y * tfm) - qy) + py);
        const double nz = -((((-massA * a.thrust.z) + a.thrust.z * tfm) - qz) + pz);
        const double oom = 1.0 / massA;
        const double ootf = 1.0 / (tf - 1.0);
        accel = GVector((oom * nx) * ootf, (oom * ny) * ootf, (oom * nz) * ootf);
    } else {
        // overlapping now.  push apart along the line between centres
        GVector n;
        if (dist2 == 0.0) {
            n = GVector((a.id > b.id) ? 1.0 : -1.0, 0.0, 0.0);
        } else {
            n = GVector(b.pos.x - a.pos.x, b.pos.y - a.pos.y, b.pos.z - a.pos.z);
            const double len = std::sqrt((n.x * n.x + n.y * n.y) + n.z * n.z);
            if (len != 0.0) {
                const double inv = 1.0 / len;
                n.x *= inv;
                n.y *= inv;
                n.z *= inv;
            }
        }
        const double overlap = (radii - std::sqrt(dist2)) + 1.0;
        const double total = massB + massA;
        const double shareA = (massB / total) * overlap;
        const double shareB = (massA / total) * overlap;

        if (a.isFree and b.isFree) {
            if (depth < 2) {
                // see whether they would still collide once separated
                CollisionBall a2(a), b2(b);
                a2.pos.x = a.pos.x - n.x * shareA;
                a2.pos.y = a.pos.y - n.y * shareA;
                a2.pos.z = a.pos.z - n.z * shareA;
                b2.pos.x = n.x * shareB + b.pos.x;
                b2.pos.y = n.y * shareB + b.pos.y;
                b2.pos.z = n.z * shareB + b.pos.z;
                Collide(a2, b2, depth + 1, result);

                if ((massB / massA) > 25.0) {
                    // much heavier than us.  keep the direction, but limit the push to the overlap
                    GVector dir(result.accel);
                    const double dlen = std::sqrt((dir.x * dir.x + dir.y * dir.y) + dir.z * dir.z);
                    if (dlen != 0.0) {
                        const double inv = 1.0 / dlen;
                        dir.x *= inv;
                        dir.y *= inv;
                        dir.z *= inv;
                    }
                    result.accel = GVector(dir.x * overlap, dir.y * overlap, dir.z * overlap);
                }

                const double rx = a.vel.x - b.vel.x, ry = a.vel.y - b.vel.y, rz = a.vel.z - b.vel.z;
                if (((rx * rx + ry * ry) + rz * rz) > 0.0001)
                    return;
            } else {
                _log(DESTINY__ERROR, "Destiny::Collide() - ball %lli is stuck inside ball %lli.", a.id, b.id);
            }
        }

        // push out of the overlap within one tick, cancelling our velocity and thrust towards the other ball
        const double tn = (a.thrust.y * n.y + n.x * a.thrust.x) + a.thrust.z * n.z;
        const double tx = n.x * tn, ty = n.y * tn, tz = n.z * tn;
        const double d = ((1.0 / (k * dt + massA)) * dt) * massA;
        const double f = -(shareA / d);
        const double ootick = 1.0 / dt;
        accel = GVector(((f * n.x) - a.vel.x) * ootick - tx,
                        ((n.y * f) - a.vel.y) * ootick - ty,
                        ootick * ((n.z * f) - a.vel.z) - tz);
    }

    // a missile isn't deflected by its own target
    if ((a.mode == Ball::Mode::MISSILE) and (b.id == a.followID)) {
        result.hitFollowTarget = true;
        return;
    }

    if (time >= result.time) {
        accel.x *= 0.85;
        accel.y *= 0.85;
        accel.z *= 0.85;
        if (time != result.time) {
            result.time = time;
            result.accel = accel;
        } else {
            const double mag2 = (accel.x * accel.x + accel.y * accel.y) + accel.z * accel.z;
            const double old2 = (result.accel.x * result.accel.x + result.accel.y * result.accel.y) + result.accel.z * result.accel.z;
            if (mag2 > old2)
                result.accel = accel;
        }
    }
}

/* ======================================================================================
 *  BallparkSnapshot
 * ====================================================================================== */

BallparkSnapshot::BallparkSnapshot(SystemManager* pSystem)
: m_system(pSystem)
{
}

namespace {
    // reads the ball as the client gets it, so flags, radius, mass and agility are exactly what the client has
    bool ReadBall(SystemEntity* pSE, CollisionBall& ball) {
        Buffer buf;
        pSE->EncodeDestiny(buf);
        size_t size(buf.size());
        if (size < sizeof(BallHeader))
            return false;
        const uint8* data(&buf[0]);

        BallHeader head;
        std::memcpy(&head, data, sizeof(BallHeader));
        size_t offset(sizeof(BallHeader));

        ball.id = head.entityID;
        ball.mode = head.mode;
        ball.radius = static_cast<double>(head.radius);
        ball.pos = GPoint(head.posX, head.posY, head.posZ);
        ball.isFree = ((head.flags & Ball::Flag::IsFree) != 0);
        ball.isMassive = ((head.flags & Ball::Flag::IsMassive) != 0);
        ball.mass = FIXED_MASS;     // the client's default for rigid balls, which have no MassSector

        if (head.mode != Ball::Mode::RIGID) {
            if (size < offset + sizeof(MassSector))
                return false;
            MassSector mass;
            std::memcpy(&mass, data + offset, sizeof(MassSector));
            offset += sizeof(MassSector);
            ball.mass = (mass.mass < 0.0 ? 0.0 : mass.mass);
            ball.isCloaked = (mass.cloak != 0);
        }
        if (ball.isFree) {
            if (size < offset + sizeof(DataSector))
                return false;
            DataSector ds;
            std::memcpy(&ds, data + offset, sizeof(DataSector));
            ball.maxVelocity = static_cast<double>(ds.maxSpeed < 0.0f ? 0.0f : ds.maxSpeed);
            ball.vel = GVector(ds.velX, ds.velY, ds.velZ);
            ball.agility = static_cast<double>(ds.inertia <= 0.0f ? 1.0f : ds.inertia);
        }
        ball.pSE = pSE;
        return true;
    }
}

const CollisionBall* BallparkSnapshot::Get(SystemEntity* pSE) {
    if (pSE == nullptr)
        return nullptr;
    std::map<uint32, CollisionBall>::iterator itr(m_balls.find(pSE->GetID()));
    if (itr != m_balls.end())
        return &itr->second;

    CollisionBall ball;
    if (!ReadBall(pSE, ball))
        return nullptr;

    // moving balls: take the simulation state directly
    DestinyManager* pDestiny(pSE->DestinyMgr());
    if (pDestiny != nullptr)
        pDestiny->GetCollisionState(ball);

    return &(m_balls.emplace(pSE->GetID(), ball).first->second);
}

const std::vector<const CollisionBall*>& BallparkSnapshot::GetMassiveBalls(SystemBubble* pBubble) {
    std::map<SystemBubble*, std::vector<const CollisionBall*>>::iterator itr(m_massive.find(pBubble));
    if (itr != m_massive.end())
        return itr->second;

    std::vector<const CollisionBall*>& list(m_massive[pBubble]);
    std::map<uint32, SystemEntity*> entities;
    m_system->GetBallparkEntities(pBubble, entities);
    for (auto cur : entities) {
        const CollisionBall* pBall(Get(cur.second));
        if (pBall == nullptr)
            continue;
        if (!pBall->isMassive or pBall->isCloaked or (pBall->mode == Ball::Mode::MISSILE))
            continue;
        list.push_back(pBall);
    }
    return list;
}

}   // namespace Destiny

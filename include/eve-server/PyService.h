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


#ifndef __PYSERVICE_H_INCL__
#define __PYSERVICE_H_INCL__

#include "PyCallable.h"
#include "PyServiceMgr.h"

class PyPacket;
class PyRep;
class PySubStream;
class PyTuple;
class PyObject;
class Client;
class PyCallStream;
class PyBoundObject;
class EntityList;

//convenience macro, you do not HAVE to use this
#define PyCallable_DECL_CALL(n) PyResult Handle_##n(PyCallArgs &call);

class PyService : public PyCallable {
public:
    PyService(PyServiceMgr *mgr, const char *serviceName);
    virtual ~PyService();

    //overload Callable for binding:
    virtual PyResult Call(const std::string &method, PyCallArgs &args);

    const char *GetName() const { return(m_name.c_str()); }
    EntityList &entityList() const { return(m_manager->entity_list); }

protected:
    typedef enum {    //enum to make sure the caller uses legit values.
        check_Always = 0,
        check_Never,
        check_in_year,
        check_in_6_months,
        check_in_3_months,
        check_in_1_month,
        check_in_1_week,
        check_in_1_day,
        check_in_12_hours,
        check_in_6_hours,
        check_in_3_hours,
        check_in_2_hours,
        check_in_1_hour,
        check_in_30_minutes,
        check_in_15_minutes,
        check_in_5_minutes,
        check_in_1_minute,
        check_in_30_seconds,
        check_in_15_seconds,
        check_in_5_seconds,
        check_in_1_second,
        _checkCount
    } CacheCheckTime;
    static const char *const s_checkTimeStrings[_checkCount];
    PyObject *_BuildCachedReturn(PySubStream **result, const char *sessionInfo, CacheCheckTime check);

    //you MUST overload this factory method if you offer any bound services:
    virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);

    //some service-level remote calls, need to be reworked:
    virtual PyResult Handle_MachoResolveObject(PyCallArgs &call);
    virtual PyResult Handle_MachoBindObject(PyCallArgs &call);

    PyServiceMgr *const m_manager;

private:
    std::string m_name;
};

#if 0
class FixedEngine {
public:

};

class VelocityManager {
public:

    void Go(Ga::GaFloat step) {
        const Ga::GaVec3 LinearVelocity = m_body->getLinearVelocity();

        Ga::GaFloat velocityMag = LinearVelocity.length();

        if(velocityMag == m_maxLinearVelocity)    //do nothing...
            return;

        Ga::GaVec3 heading = m_body->getOrientation() * (1, 0, 0);

        Ga::GaFloat maxAccel = m_maxThrust / m_mass;    // m/s^2
        Ga::GaFloat deltaVelocity = maxAccel * step;    // m/s

        if(velocityMag > m_maxLinearVelocity) {
            //slow down a little...
            Ga::GaFloat newVelocity = velocityMag - deltaVelocity;
            if(newVelocity >= m_maxLinearVelocity) {
                //applying our full force for the entire timeslice will not reach our goal
                heading *= -m_maxThrust;
                m_body->applyForce(heading);
                return;
            }

            //else, we need to throttle our thrust...
            Ga::GaFloat overage = m_maxLinearVelocity - newVelocity;
            Ga::GaFloat factor = 1 - (overage / deltaVelocity);

            heading *= -m_maxThrust * factor;
            m_body->applyForce(heading);
        } else {
            //speed up.
            Ga::GaFloat newVelocity = velocityMag + deltaVelocity;
            if(newVelocity >= m_maxLinearVelocity) {
                //applying our full force for the entire timeslice will not reach our goal
                heading *= m_maxThrust;
                m_body->applyForce(heading);
                return;
            }

            //else, we need to throttle our thrust...
            Ga::GaFloat overage = m_maxLinearVelocity - newVelocity;
            Ga::GaFloat factor = 1 - (overage / deltaVelocity);

            heading *= m_maxThrust * factor;
            m_body->applyForce(heading);
        }
    }

protected:
    Ga::GaFloat m_mass;                    // kg
    Ga::GaFloat m_maxThrust;            // N
    Ga::GaFloat m_maxLinearVelocity;    // m/s
    Ga::GaPtr<Ga::Body> m_body;
};

class BodyFollower {
public:

    void Go(Ga::GaFloat step) {
    }

protected:
    VelocityManager *const m_velocity;
    DirectionManager *const m_direction;
    Ga::GaPtr<Ga::Body> m_bodyToFollow;
};


#endif

#endif




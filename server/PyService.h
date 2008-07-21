/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __PYSERVICE_H_INCL__
#define __PYSERVICE_H_INCL__

#include <string>
#include <map>
#include "PyCallable.h"
#include "PyServiceMgr.h"

class PyPacket;
class PyRep;
class PyRepSubStream;
class PyRepTuple;
class PyRepObject;
class Client;
class PyCallStream;
class PyBoundObject;
class EntityList;

//convenience macro, you do not HAVE to use this
#define PyCallable_DECL_CALL(n) PyCallResult Handle_##n(PyCallArgs &call);


class PyService : public PyCallable {
public:
	PyService(PyServiceMgr *mgr, const char *serviceName);
	virtual ~PyService();
	
	bool IsPacketFor(const PyPacket *packet) const;
	
	//overload Callable for binding:
	virtual PyCallResult Call(PyCallStream &call, PyCallArgs &args);
	
	EntityList *GetEntityList() const { return(m_manager->entity_list); }
	
protected:
	typedef enum {	//enum to make sure the caller uses legit values.
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
	PyRepObject *_BuildCachedReturn(PyRepSubStream **result, const char *sessionInfo, CacheCheckTime check);
	
	//you MUST overload this factory method if you offer any bound services:
	virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
	
	//some service-level remote calls, need to be reworked:
	virtual PyCallResult Handle_MachoResolveObject(PyCallArgs &call);
	virtual PyCallResult Handle_MachoBindObject(PyCallArgs &call);
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
		
		if(velocityMag == m_maxLinearVelocity)	//do nothing...
			return;
			
		Ga::GaVec3 heading = m_body->getOrientation() * (1, 0, 0);
		
		Ga::GaFloat maxAccel = m_maxThrust / m_mass;	// m/s^2
		Ga::GaFloat deltaVelocity = maxAccel * step;	// m/s
		
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
	Ga::GaFloat m_mass;					// kg
	Ga::GaFloat m_maxThrust;			// N
	Ga::GaFloat m_maxLinearVelocity;	// m/s
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




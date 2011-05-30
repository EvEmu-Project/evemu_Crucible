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

#ifndef __ASTEROID_H_INCL__
#define __ASTEROID_H_INCL__

#include "system/SystemEntity.h"

#if 0

CREATE TABLE sysAsteroidBelts (
	beltID INTEGER NOT NULL,
	nextGrowTime INTEGER UNSIGNED NOT NULL,
	CONSTRAINT FOREIGN KEY (beltID)
		REFERENCES mapDenormalize (`itemID`),
	PRIMARY KEY(beltID)
);

CREATE TABLE sysAsteroids 
	(
	asteroidID INTEGER UNSIGNED NOT NULL auto_increment,
	locationID INTEGER NOT NULL,
	typeID INTEGER NOT NULL,
	oreVolume REAL NOT NULL,
	x REAL NOT NULL,
	y REAL NOT NULL,
	z REAL NOT NULL,
	CONSTRAINT FOREIGN KEY (locationID)
	 REFERENCES `sysAsteroidBelts` (`beltID`),
	CONSTRAINT FOREIGN KEY (itemID)
	 REFERENCES `entity` (`itemID`),
	PRIMARY KEY(asteroidID)
);
#endif



/*
class SystemAsteroidEntity : public InanimateSystemEntity {
public:
	SystemAsteroidEntity(const GPoint &pos);
	virtual ~SystemAsteroidEntity() { targets.DoDestruction(); }
	
	virtual void Process();
	
	virtual PyDict *MakeSlimItem() const;
	//return ownership of a new damage state tuple.
	virtual PyList *MakeDamageState() const;
	virtual void MakeAddBall(DoDestiny_AddBall &into, uint32 updateID) const;
	
	//overload the parts of SystemEntity which we care about
	//virtual uint32 GetID() const;
	//virtual double GetRadius() const;
	//virtual const GPoint &GetPosition() const;
	virtual void EncodeDestiny(std::vector<byte> &into) const;
	
protected:
	const GPoint m_position;
};*/

/**
 * DynamicSystemEntity which represents structure object in space
 */
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;

class AsteroidEntity
: public DynamicSystemEntity
{
public:
	AsteroidEntity(
		InventoryItemRef asteroid,
		SystemManager *system,
		PyServiceMgr &services,
		const GPoint &position);

    /*
	 * Primary public interface:
	 */
    InventoryItemRef GetStructureObject() { return _asteroidRef; }
    DestinyManager * GetDestiny() { return m_destiny; }
    SystemManager * GetSystem() { return m_system; }

	/*
	 * Public fields:
	 */
	
	inline double x() const { return(GetPosition().x); }
	inline double y() const { return(GetPosition().y); }
	inline double z() const { return(GetPosition().z); }

	//SystemEntity interface:
	virtual EntityClass GetClass() const { return(ecAsteroidEntity); }
	virtual bool IsAsteroidEntity() const { return true; }
	virtual AsteroidEntity *CastToAsteroidEntity() { return(this); }
	virtual const AsteroidEntity *CastToAsteroidEntity() const { return(this); }
	virtual void Process();
	virtual void EncodeDestiny( Buffer& into ) const;
	virtual void TargetAdded(SystemEntity *who) {}
    virtual void TargetLost(SystemEntity *who) {}
    virtual void TargetedAdd(SystemEntity *who) {}
    virtual void TargetedLost(SystemEntity *who) {}
	virtual void TargetsCleared() {}
	virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
	virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}
	virtual uint32 GetCorporationID() const { return(1); }
	virtual uint32 GetAllianceID() const { return(0); }
	virtual void Killed(Damage &fatal_blow);
	virtual SystemManager *System() const { return(m_system); }
	
	void ForcedSetPosition(const GPoint &pt);
	
	virtual bool ApplyDamage(Damage &d);
	virtual void MakeDamageState(DoDestinyDamageState &into) const;

	void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
	void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

	//inline const GPoint &GetPosition() const { return(m_position); }
	//uint32 GetID() const { return(m_asteroidID); }
	//virtual const char *GetName() const { return("Asteroid"); }
	//virtual double GetRadius() const;	// { return(m_radius); }
	
	void Grow();

protected:
	/*
	 * Member functions
	 */

	/*
	 * Member fields:
	 */
	SystemManager *const m_system;	//we do not own this
	PyServiceMgr &m_services;	//we do not own this
    InventoryItemRef _asteroidRef;   // We don't own this
};

// OLD Asteroid class:
/*
class Asteroid : public ItemSystemEntity {
public:
	Asteroid(SystemManager *system, InventoryItemRef self);
	virtual ~Asteroid();

	//SystemEntity interface:
	virtual void QueueDestinyUpdate(PyTuple **du) {}
	virtual void QueueDestinyEvent(PyTuple **multiEvent) {}
	virtual void TargetAdded(SystemEntity *who) {}
	virtual void TargetLost(SystemEntity *who) {}
	virtual void TargetedAdd(SystemEntity *who) {}
	virtual void TargetedLost(SystemEntity *who) {}
	virtual void TargetsCleared() {}
	virtual void ProcessDestiny() {}
	virtual SystemManager *System() const { return(m_system); }
	virtual bool IsStaticEntity() const { return(true); }
	virtual bool ApplyDamage(Damage &d);
	
	//inline const GPoint &GetPosition() const { return(m_position); }
	//uint32 GetID() const { return(m_asteroidID); }
	//virtual const char *GetName() const { return("Asteroid"); }
	//virtual double GetRadius() const;	// { return(m_radius); }
	
	void Grow();
	
	virtual void EncodeDestiny( Buffer& into ) const;
	//virtual PyDict *MakeSlimItem() const;
	
protected:
	//InventoryItem *const m_self;
	SystemManager *const m_system;
	//const uint32 m_asteroidID;
	//const uint32 m_typeID;
	//const GPoint m_position;
	//asteroid owner: 500021
	//double m_radius;	//radius of 91-95 makes for reasonable asteroids.
};
*/
#endif /* !__ASTEROID__H__INCL__ */

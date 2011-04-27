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

#ifndef __SYSTEMENTITIES_H_INCL__
#define __SYSTEMENTITIES_H_INCL__

#include "system/SystemEntity.h"
#include "system/SystemDB.h"	//for DBSystemEntity

class SystemManager;

//simple intermediate base class which eliminates a lot of the SystemEntity interface
//for inanimate entities which do not care.
class InanimateSystemEntity : public SystemEntity {
public:
	InanimateSystemEntity(SystemManager *system) : SystemEntity(), m_system(system) {}
	
	virtual EntityClass GetClass() const { return(ecCelestial); }
	virtual bool IsCelestial() const { return true; }
	
	//I really dislike having this here...
	virtual InventoryItemRef Item() const { return InventoryItemRef(); }
	
	virtual void TargetAdded(SystemEntity *who) {}
	virtual void TargetLost(SystemEntity *who) {}
	virtual void TargetedAdd(SystemEntity *who) {}
	virtual void TargetedLost(SystemEntity *who) {}
	virtual void TargetsCleared() {}
	virtual void ProcessDestiny() {}
	virtual void QueueDestinyUpdate(PyTuple **du);
	virtual void QueueDestinyEvent(PyTuple **multiEvent);
	//virtual const GPoint &GetPosition() const;
	virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target) {}
	virtual bool ApplyDamage(Damage &d) { return(false); }
	virtual void Killed(Damage &fatal_blow) {}
	virtual SystemManager *System() const { return(m_system); }
	
protected:
	SystemManager *const m_system;	//we do not own this
};

class SimpleSystemEntity : public InanimateSystemEntity {
public:
	SimpleSystemEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SimpleSystemEntity() {}

	//for now, all simple entities are celestials, which are always visible.
	virtual bool IsVisibleSystemWide() const { return true; }

	static SimpleSystemEntity *MakeEntity(SystemManager *system, const DBSystemEntity &entity);
	
	virtual bool LoadExtras(SystemDB *db);

	//some of these are generic enough..
	virtual PyDict *MakeSlimItem() const;
	virtual void MakeDamageState(DoDestinyDamageState &into) const;
	
	const DBSystemEntity data;
	uint32 GetID() const { return(data.itemID); }
	virtual const char *GetName() const { return(data.itemName.c_str()); }
	virtual double GetRadius() const { return(data.radius); }
	virtual const GPoint &GetPosition() const { return( data.position ); }
    virtual const GVector &GetVelocity() const;
};

class SystemPlanetEntity : public SimpleSystemEntity {
public:
	SystemPlanetEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemPlanetEntity() { targets.DoDestruction(); }

	virtual void EncodeDestiny( Buffer& into ) const;
};

class SystemStationEntity : public SimpleSystemEntity {
public:
	SystemStationEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemStationEntity() { targets.DoDestruction(); }
	
	virtual void EncodeDestiny( Buffer& into ) const;
	virtual PyDict *MakeSlimItem() const;
};

//for now...
class SystemStargateEntity : public SystemStationEntity {
public:
	SystemStargateEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemStargateEntity();
	
	virtual bool LoadExtras(SystemDB *db);
	virtual PyDict *MakeSlimItem() const;
	
protected:
	PyRep *m_jumps;
};


class SystemSimpleEntity : public SystemPlanetEntity {
public:
	//"fallback" entity... just treat as a planet for now...
	SystemSimpleEntity(SystemManager *system, const DBSystemEntity &entity) : SystemPlanetEntity(system, entity) {}
	virtual ~SystemSimpleEntity() { targets.DoDestruction(); }
};


class SystemDungeonEntranceEntity : public ItemSystemEntity {
public:
	SystemDungeonEntranceEntity(SystemManager *system, InventoryItemRef self);
	virtual ~SystemDungeonEntranceEntity() { targets.DoDestruction(); }
	
	virtual SystemManager *System() const { return(m_system); }
	virtual bool IsVisibleSystemWide() const { return true; }

	virtual PyDict *MakeSlimItem() const;
	virtual void EncodeDestiny( Buffer& into ) const;

protected:
	SystemManager *const m_system;	//we do not own this
};

/**/




#endif





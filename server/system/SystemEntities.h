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

#ifndef __SYSTEMENTITIES_H_INCL__
#define __SYSTEMENTITIES_H_INCL__

#include "SystemEntity.h"
#include "SystemDB.h"	//for DBSystemEntity

class SystemManager;

//simple intermediate base class which eliminates a lot of the SystemEntity interface
//for inanimate entities which do not care.
class InanimateSystemEntity : public SystemEntity {
public:
	InanimateSystemEntity(SystemManager *system) : SystemEntity(), m_system(system) {}
	
	virtual EntityClass GetClass() const { return(ecCelestial); }
	virtual bool IsCelestial() const { return(true); }
	
	//I really dislike having this here...
	virtual InventoryItem *Item() const { return(NULL); }
	
	virtual void TargetAdded(SystemEntity *who) {}
	virtual void TargetLost(SystemEntity *who) {}
	virtual void TargetedAdd(SystemEntity *who) {}
	virtual void TargetedLost(SystemEntity *who) {}
	virtual void TargetsCleared() {}
	virtual void ProcessDestiny(uint32 stamp) {}
	virtual void QueueDestinyUpdate(PyRepTuple **du);
	virtual void QueueDestinyEvent(PyRepTuple **multiEvent);
	//virtual const GPoint &GetPosition() const;
	virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target) {}
	virtual void ApplyDamage(Damage &d) {}
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
	virtual bool IsVisibleSystemWide() const { return(true); }

	static SimpleSystemEntity *MakeEntity(SystemManager *system, const DBSystemEntity &entity);
	
	virtual bool LoadExtras(SystemDB *db);

	//some of these are generic enough..
	virtual PyRepDict *MakeSlimItem() const;
	virtual void MakeDamageState(DoDestinyDamageState &into) const;
	
	const DBSystemEntity data;
	uint32 GetID() const { return(data.itemID); }
	virtual const char *GetName() const { return(data.itemName.c_str()); }
	virtual double GetRadius() const { return(data.radius); }
	virtual const GPoint &GetPosition() const { return( data.position ); }
};

class SystemPlanetEntity : public SimpleSystemEntity {
public:
	SystemPlanetEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemPlanetEntity() { targets.DoDestruction(); }

	virtual void EncodeDestiny(std::vector<byte> &into) const;
};

class SystemStationEntity : public SimpleSystemEntity {
public:
	SystemStationEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemStationEntity() { targets.DoDestruction(); }
	
	virtual void EncodeDestiny(std::vector<byte> &into) const;
	virtual PyRepDict *MakeSlimItem() const;
};

//for now...
class SystemStargateEntity : public SystemStationEntity {
public:
	SystemStargateEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemStargateEntity();
	
	virtual bool LoadExtras(SystemDB *db);
	virtual PyRepDict *MakeSlimItem() const;
	
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
	SystemDungeonEntranceEntity(SystemManager *system, InventoryItem *self);
	virtual ~SystemDungeonEntranceEntity() { targets.DoDestruction(); }
	
	virtual SystemManager *System() const { return(m_system); }
	virtual bool IsVisibleSystemWide() const { return(true); }

	virtual PyRepDict *MakeSlimItem() const;
	virtual void EncodeDestiny(std::vector<byte> &into) const;
protected:
	SystemManager *const m_system;	//we do not own this
};

/**/




#endif





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

#ifndef __SYSTEMENTITY_H_INCL__
#define __SYSTEMENTITY_H_INCL__

#include "../ship/TargetManager.h"
#include <vector>


class PyRepDict;
class PyRepList;
class PyRepTuple;
class DoDestiny_AddBall;
class DoDestinyDamageState;
class DBSystemEntity;
class InventoryItem;
class SystemDB;
class GPoint;
class Damage;
class SystemBubble;
class SystemManager;

class Client;
class NPC;


class SystemEntity {
public:
	typedef enum {
		ecClient,
		ecNPC,
		ecCelestial,
		ecOther
	} EntityClass;
	
	SystemEntity();
	virtual ~SystemEntity() {}
	
	TargetManager targets;
	
	virtual void Process();
	virtual void ProcessDestiny(uint32 stamp) = 0;

	//this is a bit crude, but I prefer this over RTTI.
	virtual EntityClass GetClass() const { return(ecOther); }
	virtual bool IsClient() const { return(false); }
	virtual bool IsNPC() const { return(false); }
	virtual bool IsCelestial() const { return(false); }
	virtual Client *CastToClient() { return(NULL); }
	virtual const Client *CastToClient() const { return(NULL); }
	virtual NPC *CastToNPC() { return(NULL); }
	virtual const NPC *CastToNPC() const { return(NULL); }
	
	virtual bool IsStaticEntity() const { return(true); }	//will this entity's position never change?
	virtual bool IsVisibleSystemWide() const { return(false); }
	
	inline SystemBubble *Bubble() const { return(m_bubble); }	//may be NULL
	
	//may consume the arguments, or not.
	virtual void QueueDestinyUpdate(PyRepTuple **du) = 0;
	virtual void QueueDestinyEvent(PyRepTuple **multiEvent) = 0;
	
	//I am not sure if I want this here...
	virtual InventoryItem *Item() const = 0;
	
	virtual SystemManager *System() const = 0;	//may yeild NULL
	
	//expand the vector as needed, and encode the destiny update into it.
	virtual void EncodeDestiny(std::vector<byte> &into) const = 0;
	//return ownership of a new foo.SlimItem dict
	virtual PyRepDict *MakeSlimItem() const = 0;
	//fill in the supplied damage state object.
	virtual void MakeDamageState(DoDestinyDamageState &into) const = 0;
	//return ownership of a new damage state tuple (calls MakeDamageState)
	PyRepTuple *MakeDamageState() const;
	//get the item ID of this entity
	virtual uint32 GetID() const = 0;
	//get the position of this entity in space.
	virtual const GPoint &GetPosition() const = 0;
	//get other attributes of the entity:
	virtual const char *GetName() const = 0;
	virtual double GetRadius() const = 0;
	
	//Im not happy with these being here..
	virtual void TargetAdded(SystemEntity *who) = 0;
	virtual void TargetLost(SystemEntity *who) = 0;
	virtual void TargetedAdd(SystemEntity *who) = 0;
	virtual void TargetedLost(SystemEntity *who) = 0;
	virtual void TargetsCleared() = 0;

	//process outgoing damage, applying any bonuses...
	virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target) = 0;
	//process incoming damage.
	virtual void ApplyDamage(Damage &d) = 0;
	//handles death.
	virtual void Killed(Damage &fatal_blow);
	
	
	virtual void MakeAddBall(DoDestiny_AddBall &into, uint32 updateID) const;

	//helpers:
	double DistanceTo2(const SystemEntity *other) const;
	
protected:
	
private:
	friend class SystemBubble;	//only to update m_bubble
	SystemBubble *m_bubble;	//we do not own this, may be NULL. Only changed by SystemBubble
};

class ItemSystemEntity : public SystemEntity {
public:
	ItemSystemEntity(InventoryItem *self=NULL);
	virtual ~ItemSystemEntity();
	
	//Default implementations fall to m_self.
	virtual uint32 GetID() const;
	virtual InventoryItem *Item() const { return(m_self); }
	virtual const char *GetName() const;
	virtual const GPoint &GetPosition() const;
	virtual double GetRadius() const;
	virtual PyRepDict *MakeSlimItem() const;
	virtual void MakeDamageState(DoDestinyDamageState &into) const;
	
	//process outgoing damage, applying any bonuses...
	virtual void ApplyDamageModifiers(Damage &d, SystemEntity *target);
	//process incoming damage.
	virtual void ApplyDamage(Damage &d);
	
protected:
	InventoryItem *m_self;	// we have a ref of this
	
	void _SendDamageStateChanged() const;
	void _SetSelf(InventoryItem *);
};


class DestinyManager;

class DynamicSystemEntity : public ItemSystemEntity {
public:
	DynamicSystemEntity(DestinyManager *mgr=NULL /*ownership taken*/, InventoryItem *self=NULL /* takes ref */);
	virtual ~DynamicSystemEntity();

	//partial implementation of SystemEntity interface:
	virtual void ProcessDestiny(uint32 stamp);
	virtual const GPoint &GetPosition() const;
	virtual void EncodeDestiny(std::vector<byte> &into) const;
	virtual bool IsStaticEntity() const { return(false); }

	//Added interface:
	//get the corporation of this entity
	virtual uint32 GetCorporationID() const = 0;
	virtual double GetMass() const = 0;
	virtual double GetMaxVelocity() const = 0;
	virtual double GetAgility() const = 0;
	
	inline DestinyManager *Destiny() const { return(m_destiny); }
	
	virtual void Killed(Damage &fatal_blow);
	
protected:
	DestinyManager *m_destiny;	//we own this! NULL if we are not in a system
};


#endif





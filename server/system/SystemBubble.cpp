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

#include "SystemBubble.h"
#include "SystemEntity.h"
#include "SystemManager.h"

#include "../common/PyRep.h"
#include "../common/DestinyStructs.h"
#include "../common/DestinyBinDump.h"
#include "../common/logsys.h"

#include "../packets/Destiny.h"


SystemBubble::SystemBubble(const GPoint &center, double radius)
: m_center(center),
  m_radius(radius),
  m_radius2(radius*radius)
{
	_log(DESTINY__BUBBLE_DEBUG, "Created new bubble %p at (%.2f,%.2f,%.2f) with radius %.2f", this, m_center.x, m_center.y, m_center.z, m_radius);
}

//send a set of destiny events and updates to everybody in the bubble.
void SystemBubble::BubblecastDestiny(std::vector<PyRepTuple *> &updates, std::vector<PyRepTuple *> &events, const char *desc) const {
	//this could be done more efficiently....
	{
		std::vector<PyRepTuple *>::iterator cur, end;
		cur = updates.begin();
		end = updates.end();
		for(; cur != end; cur++) {
			PyRepTuple *up = *cur;
			BubblecastDestinyUpdate(&up, desc);	//update is consumed.
		}
		updates.clear();
	}
	{
		std::vector<PyRepTuple *>::iterator cur, end;
		cur = events.begin();
		end = events.end();
		for(; cur != end; cur++) {
			PyRepTuple *up = *cur;
			BubblecastDestinyUpdate(&up, desc);	//update is consumed.
		}
		events.clear();
	}
}

//send a destiny update to everybody in the bubble.
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyUpdate(PyRepTuple **payload, const char *desc) const {
	PyRepTuple *up = *payload;
	*payload = NULL;	//could optimize out one of the Clones in here...
	
	std::set<SystemEntity *>::const_iterator cur, end, tmp;
	cur = m_dynamicEntities.begin();
	end = m_dynamicEntities.end();
	PyRepTuple *up_dup = NULL;
	for(; cur != end; ++cur) {
		if(up_dup == NULL)
			up_dup = up->TypedClone();
		_log(DESTINY__BUBBLE_TRACE, "Bubblecast %s update to %s (%lu)", desc, (*cur)->GetName(), (*cur)->GetID());
		(*cur)->QueueDestinyUpdate(&up_dup);
		//they may not have consumed it (NPCs for example), so dont re-dup it in that case.
	}
	
	delete up_dup;
	delete up;
}

//send a destiny event to everybody in the bubble.
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyEvent(PyRepTuple **payload, const char *desc) const {
	PyRepTuple *up = *payload;
	*payload = NULL;	//could optimize out one of the Clones in here...
	
	std::set<SystemEntity *>::const_iterator cur, end, tmp;
	cur = m_dynamicEntities.begin();
	end = m_dynamicEntities.end();
	PyRepTuple *up_dup = NULL;
	for(; cur != end; ++cur) {
		if(up_dup == NULL)
			up_dup = up->TypedClone();
		_log(DESTINY__BUBBLE_TRACE, "Bubblecast %s event to %s (%lu)", desc, (*cur)->GetName(), (*cur)->GetID());
		(*cur)->QueueDestinyEvent(&up_dup);
		//they may not have consumed it (NPCs for example), so dont re-dup it in that case.
	}
	
	delete up_dup;
	delete up;
}

//called at some regular interval from the bubble manager.
//verifies that each entity is still in this bubble.
//if any entity is no longer in the bubble, they are removed
//from the bubble and stuck into the vector for re-classification.
bool SystemBubble::ProcessWander(std::vector<SystemEntity *> &wanderers) {
	//check to see if any of our dynamic entities are no longer in our bubble...
	std::set<SystemEntity *>::const_iterator cur, end;
	cur = m_dynamicEntities.begin();
	end = m_dynamicEntities.end();
	std::vector<SystemEntity *> found_wandering;	//the wanderers array may have other stuff in it, so use a local first.
	for(; cur != end; ++cur) {
		if(!InBubble((*cur)->GetPosition())) {
			//we cannot use Remove directly here because it will invalidate
			//our iterator, so store them away for now.
			found_wandering.push_back(*cur);
			wanderers.push_back(*cur);
		}
	}
	std::vector<SystemEntity *>::const_iterator curw, endw;
	curw = found_wandering.begin();
	endw = found_wandering.end();
	for(; curw != endw; ++curw) {
		Remove(*curw);
	}
	return(false);
}

void SystemBubble::Add(SystemEntity *ent, bool notify) {
	//notify before addition so we do not include ourself.
	if(notify) {
		_SendAddBalls(ent);
	}
	//if they are already in this bubble, do not continue.
	if(m_entities.find(ent->GetID()) != m_entities.end()) {
		_log(DESTINY__BUBBLE_TRACE, "Tried to add entity %lu to bubble %p, but it is already in here.", ent->GetID(), this);
		return;
	}
	//regardless, notify everybody else in the bubble of the add.
	_BubblecastAddBall(ent);
	
	_log(DESTINY__BUBBLE_DEBUG, "Adding entity %lu to bubble %p at (%.2f,%.2f,%.2f) with radius %.2f", ent->GetID(), this, m_center.x, m_center.y, m_center.z, m_radius);
	m_entities[ent->GetID()] = ent;
	ent->m_bubble = this;
	if(ent->IsStaticEntity() == false) {
		m_dynamicEntities.insert(ent);
	}
}

void SystemBubble::Remove(SystemEntity *ent, bool notify) {
	//assume that the entity is properly registered for its ID, and that
	//we do not need to search other values.
	_log(DESTINY__BUBBLE_DEBUG, "Removing entity %lu from bubble %p at (%.2f,%.2f,%.2f) with radius %.2f", ent->GetID(), this, m_center.x, m_center.y, m_center.z, m_radius);
	ent->m_bubble = NULL;
	m_entities.erase(ent->GetID());
	m_dynamicEntities.erase(ent);
	//notify after removal so we do not remove ourself.
	if(notify) {
		_SendRemoveBalls(ent);
	}
	//regardless, notify everybody else in the bubble of the removal.
	_BubblecastRemoveBall(ent);
}

void SystemBubble::clear() {
	m_entities.clear();
	m_dynamicEntities.clear();
}

void SystemBubble::GetEntities(std::set<SystemEntity *> &into) const {
	std::map<uint32, SystemEntity *>::const_iterator cur, end;
	cur = m_entities.begin();
	end = m_entities.end();
	for(; cur != end; cur++) {
		into.insert(cur->second);
	}
}

//NOTE: not used right now. May never be used... see SystemManager::MakeSetState
//this is called as a part of the SetState routine for initial enter space.
//it appends information for all entities contained within the bubble.
#if 0
void SystemBubble::AppendBalls(DoDestiny_SetState &ss, std::vector<byte> &setstate_buffer) const {
	if(m_entities.empty()) {
		return;
	}
	
	//go through all entities and gather the info we need...
	std::map<uint32, SystemEntity *>::const_iterator cur, end;
	cur = m_entities.begin();
	end = m_entities.end();
	for(; cur != end; cur++) {
		//ss.damageState
		ss.damageState[ cur->second->GetID() ] = cur->second->MakeDamageState();
		
		//ss.slims
		PyRepDict *slim_dict = cur->second->MakeSlimItem();
		ss.slims.add(new PyRepObject("foo.SlimItem", slim_dict));

		//append the destiny binary data...
		cur->second->EncodeDestiny(setstate_buffer);
	}
}
#endif

void SystemBubble::_SendAddBalls(SystemEntity *to_who) {
	if(m_entities.empty()) {
		_log(DESTINY__TRACE, "Add Balls: Nothing to send.");
		return;
	}
	
	//this is pretty crappy...
	uint32 updateID = to_who->System()->GetNextDestinyStamp();
		
	std::vector<byte> destiny_buffer(sizeof(Destiny::AddBall_header));
	destiny_buffer.reserve(1024);
	
	Destiny::AddBall_header *head = (Destiny::AddBall_header *) &destiny_buffer[0];
	head->more = 0;
	head->sequence = updateID;

	DoDestiny_AddBalls addballs;
	
	std::map<uint32, SystemEntity *>::const_iterator cur, end;
	cur = m_entities.begin();
	end = m_entities.end();
	for(; cur != end; ++cur) {
		if(cur->second->IsVisibleSystemWide())
			continue;	//it is already in their destiny state
		//damageState
		addballs.damages[ cur->second->GetID() ] = cur->second->MakeDamageState();
		//slim item
		PyRepDict *slim_dict = cur->second->MakeSlimItem();
		addballs.slims.add(new PyRepObject("foo.SlimItem", slim_dict));
		//append the destiny binary data...
		cur->second->EncodeDestiny(destiny_buffer);
	}
	
	addballs.destiny_binary = new PyRepBuffer(&destiny_buffer[0], destiny_buffer.size());
	destiny_buffer.clear();

	_log(DESTINY__TRACE, "Add Balls:");
	addballs.Dump(DESTINY__TRACE, "    ");
	_log(DESTINY__TRACE, "    Ball Binary:");
	_hex(DESTINY__TRACE, addballs.destiny_binary->GetBuffer(), addballs.destiny_binary->GetLength());
	_log(DESTINY__TRACE, "    Ball Decoded:");
	Destiny::DumpUpdate(DESTINY__TRACE, addballs.destiny_binary->GetBuffer(), addballs.destiny_binary->GetLength());

	PyRepTuple *tmp = addballs.FastEncode();
	to_who->QueueDestinyUpdate(&tmp);	//may consume, but may not.
	delete tmp;	//may not have been consumed.
}

void SystemBubble::_SendRemoveBalls(SystemEntity *to_who) {
	if(m_entities.empty()) {
		_log(DESTINY__TRACE, "Remove Balls: Nothing to send.");
		return;
	}
	
	DoDestiny_RemoveBalls remove_balls;
	
	std::map<uint32, SystemEntity *>::const_iterator cur, end;
	cur = m_entities.begin();
	end = m_entities.end();
	for(; cur != end; ++cur) {
		if(cur->second->IsVisibleSystemWide())
			continue;	//do not remove these from their state!
		remove_balls.balls.push_back(cur->second->GetID());
	}
	
	_log(DESTINY__TRACE, "Remove Balls:");
	remove_balls.Dump(DESTINY__TRACE, "    ");
	
	PyRepTuple *tmp = remove_balls.FastEncode();
	to_who->QueueDestinyUpdate(&tmp);	//may consume, but may not.
	delete tmp;	//may not have been consumed.
}

void SystemBubble::_BubblecastAddBall(SystemEntity *about_who) {
	if(m_dynamicEntities.empty()) {
		_log(DESTINY__TRACE, "Add Ball: Nobody to receive.");
		return;
	}
	
	DoDestiny_AddBall addball;
	about_who->MakeAddBall(addball, 
		//this is pretty crappy... but it kinda makes sense too...
		about_who->System()->GetNextDestinyStamp()
		);
	
	PyRepTuple *tmp = addball.FastEncode();
	BubblecastDestinyUpdate(&tmp, "AddBall");	//consumed
}

void SystemBubble::_BubblecastRemoveBall(SystemEntity *about_who) {
	if(m_dynamicEntities.empty()) {
		_log(DESTINY__TRACE, "Remove Ball: Nobody to receive.");
		return;
	}
	
	DoDestiny_RemoveBall removeball;
	removeball.entityID = about_who->GetID();
	
	_log(DESTINY__TRACE, "Remove Ball:");
	removeball.Dump(DESTINY__TRACE, "    ");
	
	PyRepTuple *tmp = removeball.FastEncode();
	BubblecastDestinyUpdate(&tmp, "RemoveBall");	//consumed
}





















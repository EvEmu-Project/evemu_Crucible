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

#include "EvemuPCH.h"

TargetManager::TargetManager(SystemEntity *self)
: m_destroyed(false),
  m_self(self)
{
}

TargetManager::~TargetManager() {
	//DO NOT call DoDestruction here! it calls virtuals!
}

//I am not happy with this:
//this function exists to deal with a specific problem with the 
// destruction chain where we reference a SystemEntity (m_self), which
// also contains their TargetManager. The TargetManager object is
// not destroyed until the base SystemEntity is destroyed, but
// the SystemEntity pointer itself becomes invalid as soon as the
// first child class in its hierarchy (such as Client or NPC) are
// destroyed. Thus, all terminal children of SystemEntity must call
// this from their destructor.
void TargetManager::DoDestruction() {
	if(!m_destroyed) {
		ClearAllTargets(false);
	}
}

void TargetManager::Process() {
	//process outgoing targeting
	{
		std::map<SystemEntity *, TargetEntry *>::iterator cur, end;
		cur = m_targets.begin();
		end = m_targets.end();
		for(; cur != end; cur++) {
			switch(cur->second->state) {
			case TargetEntry::Locking:
				//see if we are finished locking...
				if(cur->second->timer.Check()) {
					cur->second->timer.Disable();
					//yay, they are locked..
					cur->second->state = TargetEntry::Locked;
					_log(TARGET__TRACE, "%lu has finished locking %lu", m_self->GetID(), cur->first->GetID());
					m_self->TargetAdded(cur->first);
					cur->first->targets.TargetedByLocked(m_self);
				}
				break;
			case TargetEntry::Idle:
			case TargetEntry::Locked:
			case TargetEntry::PassiveLocking:
				//nothing to do right now...
				break;
			}
		}
	}
	//now incoming...?
	//nothing to do right now...
	/*{
		std::map<SystemEntity *, TargetedByEntry *>::iterator cur, end;
		cur = m_targetedBy.begin();
		end = m_targetedBy.end();
		for(; cur != end; cur++) {
			cur->first->targets.TargetLost(m_self);
		}
	}*/
}

void TargetManager::ClearTargets(bool notify_self) {
	_log(TARGET__TRACE, "%lu is clearing all targets", m_self->GetID());
	{
		std::map<SystemEntity *, TargetEntry *>::iterator cur, end;
		cur = m_targets.begin();
		end = m_targets.end();
		for(; cur != end; cur++) {
			_log(TARGET__TRACE, "%lu has cleared target %lu during clear all.", m_self->GetID(), cur->first->GetID());
			cur->first->targets.TargetedByLost(m_self);
			delete cur->second;
		}
		m_targets.clear();
	}
	if(notify_self)
		m_self->TargetsCleared();
}

void TargetManager::ClearAllTargets(bool notify_self) {
	ClearFromTargets();
	ClearTargets(notify_self);
	_log(TARGET__TRACE, "%lu has cleared all targeting information.", m_self->GetID());
}

void TargetManager::ClearFromTargets() {
	std::vector<SystemEntity *> ToNotify;
	ToNotify.reserve(m_targetedBy.size());
	
	//first, clean up our internal structure.
	{
		std::map<SystemEntity *, TargetedByEntry *>::iterator cur, end;
		cur = m_targetedBy.begin();
		end = m_targetedBy.end();
		for(; cur != end; cur++) {
			//do not notify until we clear our target list! otherwise bad things happen.
			ToNotify.push_back(cur->first);
			delete cur->second;
		}
		m_targetedBy.clear();
	}

	{
		std::vector<SystemEntity *>::iterator curn, endn;
		curn = ToNotify.begin();
		endn = ToNotify.end();
		for(; curn != endn; curn++) {
			
			(*curn)->targets.TargetLost(m_self);
		}
	}
}

void TargetManager::ClearTarget(SystemEntity *who) {
	//let the other entity know they are no longer targeted.
	who->targets.TargetedByLost(m_self);
	//clear it from our own state
	TargetLost(who);
}

//called directly when a 
void TargetManager::TargetLost(SystemEntity *who) {
	std::map<SystemEntity *, TargetEntry *>::iterator res;
	res = m_targets.find(who);
	if(res == m_targets.end()) {
		//not found...
		return;
	}
	//clear our internal state for this target (BEFORE the callback!)
	m_targets.erase(res);
	
	_log(TARGET__TRACE, "%lu has lost target %lu", m_self->GetID(), who->GetID());
	
	m_self->TargetLost(who);
}

bool TargetManager::StartTargeting(SystemEntity *who, uint32 lock_time) {
	//first make sure they are not already in the list
	std::map<SystemEntity *, TargetEntry *>::iterator res;
	res = m_targets.find(who);
	if(res != m_targets.end()) {
		//what to do?
		_log(TARGET__TRACE, "Told to start targeting %lu, but we are already processing them. Ignoring request.", who->GetID());
		return(false);
	}

	//TODO: check against max locked target.
	
	TargetEntry *te = new TargetEntry(who);
	te->state = TargetEntry::Locking;
	te->timer.Start(lock_time);
	m_targets[who] = te;
	
	_log(TARGET__TRACE, "%lu started targeting %lu (%lu ms lock time)", m_self->GetID(), who->GetID(), lock_time);
	return(true);
}

void TargetManager::TargetEntry::Dump() const {
	const char *sname = "Unknown State";
	switch(state) {
	case Idle:
		sname = "Idle Entry";
		break;
	case PassiveLocking:
		sname = "Passive Locking";
		break;
	case Locking:
		sname = "Locking";
		break;
	case Locked:
		sname = "Locked";
		break;
	//no default on purpose.
	}
	_log(TARGET__TRACE, "	Targeted %s (%lu): %s (%s %d ms)",
		who->GetName(),
		who->GetID(),
		sname,
		timer.Enabled()?"Running":"Disabled",
		timer.GetRemainingTime()
		);
}

void TargetManager::TargetedByEntry::Dump() const {
	const char *sname = "Unknown State";
	switch(state) {
	case Idle:
		sname = "Idle Entry";
		break;
	case Locking:
		sname = "Locking";
		break;
	case Locked:
		sname = "Locked";
		break;
	//no default on purpose.
	}
	_log(TARGET__TRACE, "	Targeted By %s (%lu): %s",
		who->GetName(),
		who->GetID(),
		sname
		);
}

void TargetManager::Dump() const {
	_log(TARGET__TRACE, "Target Dump for %lu:", m_self->GetID());
	{
		std::map<SystemEntity *, TargetEntry *>::const_iterator cur, end;
		cur = m_targets.begin();
		end = m_targets.end();
		for(; cur != end; cur++) {
			cur->second->Dump();
		}
	}
	{
		std::map<SystemEntity *, TargetedByEntry *>::const_iterator cur, end;
		cur = m_targetedBy.begin();
		end = m_targetedBy.end();
		for(; cur != end; ++cur) {
			cur->second->Dump();
		}
	}
}

SystemEntity *TargetManager::GetTarget(uint32 targetID, bool need_locked) const {
	std::map<SystemEntity *, TargetEntry *>::const_iterator cur, end;
	cur = m_targets.begin();
	end = m_targets.end();
	for(; cur != end; cur++) {
		if(cur->first->GetID() != targetID)
			continue;
		//found it...
		if(need_locked && cur->second->state != TargetEntry::Locked) {
			_log(TARGET__TRACE, "Found target %lu, but it is not locked.", targetID);
			continue;
		}
		//_log(TARGET__TRACE, "Found target %lu: %s (nl? %s)", targetID, cur->first->GetName(), need_locked?"yes":"no");
		return(cur->first);
	}
	//_log(TARGET__TRACE, "Unable to find target %lu (nl? %s)", targetID, need_locked?"yes":"no");
	return(NULL);	//not found.
}

void TargetManager::QueueTBDestinyEvent(PyRepTuple **up_in) const {
	PyRepTuple *up = *up_in;
	*up_in = NULL;	//could optimize out one of the Clones in here...
	
	std::map<SystemEntity *, TargetedByEntry *>::const_iterator cur, end;
	cur = m_targetedBy.begin();
	end = m_targetedBy.end();
	PyRepTuple *up_dup = NULL;
	for(; cur != end; ++cur) {
		if(up_dup == NULL)
			up_dup = up->TypedClone();
		cur->first->QueueDestinyEvent(&up_dup);
		//they may not have consumed it (NPCs for example), so dont re-dup it in that case.
	}
	
	delete up_dup;
	delete up;
}

void TargetManager::QueueTBDestinyUpdate(PyRepTuple **up_in) const {
	PyRepTuple *up = *up_in;
	*up_in = NULL;	//could optimize out one of the Clones in here...
	
	std::map<SystemEntity *, TargetedByEntry *>::const_iterator cur, end;
	cur = m_targetedBy.begin();
	end = m_targetedBy.end();
	PyRepTuple *up_dup = NULL;
	for(; cur != end; ++cur) {
		if(up_dup == NULL)
			up_dup = up->TypedClone();
		cur->first->QueueDestinyUpdate(&up_dup);
		//they may not have consumed it (NPCs for example), so dont re-dup it in that case.
	}
	
	delete up_dup;
	delete up;
}

/*void TargetManager::TargetedByLocking(SystemEntity *from_who) {
	//first make sure they are not already in the list
	std::map<SystemEntity *, TargetedByEntry *>::iterator res;
	res = m_targetedBy.find(from_who);
	if(res != m_targetedBy.end()) {
		//just re-use the old entry...
	} else {
		//new entry.
		TargetedByEntry *te = new TargetedByEntry(from_who);
		te->state = TargetedByEntry::Locking;
		m_targetedBy[from_who] = te;
	}
	//no notification, should we even track this event??
}*/

void TargetManager::TargetedByLocked(SystemEntity *from_who) {
	//first make sure they are not already in the list
	std::map<SystemEntity *, TargetedByEntry *>::iterator res;
	res = m_targetedBy.find(from_who);
	if(res != m_targetedBy.end()) {
		//just re-use the old entry...
		res->second->state = TargetedByEntry::Locked;
	} else {
		//new entry.
		TargetedByEntry *te = new TargetedByEntry(from_who);
		te->state = TargetedByEntry::Locking;
		m_targetedBy[from_who] = te;
	}
	_log(TARGET__TRACE, "%lu has been locked by %lu", m_self->GetID(), from_who->GetID());
	m_self->TargetedAdd(from_who);
}

void TargetManager::TargetedByLost(SystemEntity *from_who) {
	//first make sure they are not already in the list
	std::map<SystemEntity *, TargetedByEntry *>::iterator res;
	res = m_targetedBy.find(from_who);
	if(res != m_targetedBy.end()) {
		delete res->second;
		m_targetedBy.erase(res);
		m_self->TargetedLost(from_who);
		_log(TARGET__TRACE, "%lu is no longer locked by %lu", m_self->GetID(), from_who->GetID());
	} else {
		//not found.. do nothing to our state, no notification?
		_log(TARGET__TRACE, "%lu was notified of targeted lost by %lu, but they did not have us targeted in the first place.", m_self->GetID(), from_who->GetID());
	}
}

SystemEntity *TargetManager::GetFirstTarget(bool need_locked) {
	if(m_targets.empty())
		return(NULL);
	if(!need_locked) {
		//we know there is at least one entry here...
		return(m_targets.begin()->first);
	}
	
	std::map<SystemEntity *, TargetEntry *>::const_iterator cur, end;
	cur = m_targets.begin();
	end = m_targets.end();
	for(; cur != end; cur++) {
		if(cur->second->state == TargetEntry::Locked)
			return(cur->first);
	}
	return(NULL);
}

PyRep *TargetManager::Encode_GetTargets() const {
	PyRepList *result = new PyRepList();
	
	std::map<SystemEntity *, TargetEntry *>::const_iterator cur, end;
	cur = m_targets.begin();
	end = m_targets.end();
	for(; cur != end; cur++) {
		result->addInt(cur->first->GetID());
	}

	return(result);
}

PyRep *TargetManager::Encode_GetTargeters() const {
	PyRepList *result = new PyRepList();
	
	std::map<SystemEntity *, TargetedByEntry *>::const_iterator cur, end;
	cur = m_targetedBy.begin();
	end = m_targetedBy.end();
	for(; cur != end; cur++) {
		result->addInt(cur->first->GetID());
	}

	return(result);
}



















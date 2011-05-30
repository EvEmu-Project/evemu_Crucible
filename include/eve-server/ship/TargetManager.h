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


#ifndef __TARGETMANAGER_H_INCL__
#define __TARGETMANAGER_H_INCL__

class SystemEntity;
class PyRep;
class PyTuple;

class TargetManager {
public:
	TargetManager(SystemEntity *self);
	virtual ~TargetManager();

	void DoDestruction();
	
	void Process();

	//clear out our targeting information (incoming and outgoing)
	void ClearTargets(bool notify_self=true);
	void ClearTarget(SystemEntity *who);
	bool StartTargeting(SystemEntity *who, uint32 lock_time);
	void ClearAllTargets(bool notify_self=true);
	
	//Methods for AI:
	SystemEntity *GetFirstTarget(bool need_locked);
	bool HasNoTargets() const { return(m_targets.empty()); }
	bool IsTargetedBySomething() const { return(!m_targetedBy.empty()); }
	uint32 GetTotalTargets() const { return m_targets.size(); }
	
	SystemEntity *GetTarget(uint32 targetID, bool need_locked=true) const;
	void QueueTBDestinyEvent(PyTuple **up) const;	//queue a destiny event to all people targeting me.
	void QueueTBDestinyUpdate(PyTuple **up) const;	//queue a destiny update to all people targeting me.

	void Dump() const;
	uint32 TimeToLock(ShipRef ship, SystemEntity *target) const; 
	
	//Packet builders:
	PyList *GetTargets() const;
	PyList *GetTargeters() const;
	
protected:
	void ClearFromTargets();
	//called by other target managers when they are clearing their targeting out.
	void TargetLost(SystemEntity *who);

	//called in reaction to outgoing targeting events in other target managers.
	//void TargetedByLocking(SystemEntity *from_who);
	void TargetedByLocked(SystemEntity *from_who);
	void TargetedByLost(SystemEntity *from_who);

	
	class TargetedByEntry {
	public:
		TargetedByEntry(SystemEntity *_who)
			: state(Idle), who(_who) {}

		void Dump() const;

		enum {
			Idle,
			Locking,
			Locked
		} state;
		SystemEntity *const who;
	};
	
	class TargetEntry {
	public:
		TargetEntry(SystemEntity *_who)
			: state(Idle), who(_who), timer(0) {}

		void Dump() const;

		enum {
			Idle,
			PassiveLocking,
			Locking,
			Locked
		} state;
		SystemEntity *const who;
		Timer timer;
	};
	
	bool m_destroyed;	//true if we have already taken care of destruction logic.
	SystemEntity *const m_self;	//we do not own this.
	std::map<SystemEntity *, TargetedByEntry *> m_targetedBy;	//we own these values, not the keys
	std::map<SystemEntity *, TargetEntry *> m_targets;	//we own these values, not the keys
};





#endif



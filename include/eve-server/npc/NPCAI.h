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

#ifndef __NPCAI_H_INCL__
#define __NPCAI_H_INCL__

class NPC;
class SystemEntity;

class NPCAIMgr {
public:
	NPCAIMgr(NPC *who);
	
	void Process();

	void Targeted(SystemEntity *by_who);
	void TargetLost(SystemEntity *by_who);
	
protected:
	void CheckAttacks(SystemEntity *target);
	void _EnterIdle();
	void _EnterChasing(SystemEntity *target);
	void _EnterFollowing(SystemEntity *target);
	void _EnterEngaged(SystemEntity *target);
	void _SendWeaponEffect(const char *effect, SystemEntity *target);
	
	typedef enum {
		Idle,
		Chasing,	//using MWD or the like.
		Following,
		Engaged
	} State;
	State m_state;

	//cached to reduce access times.
	EvilNumber m_entityFlyRange2;
	EvilNumber m_entityChaseMaxDistance2;
	EvilNumber m_entityAttackRange2;
	
	NPC *const m_npc;
	
	Timer m_processTimer;
	Timer m_mainAttackTimer;

	Timer m_shieldBoosterTimer;
	Timer m_armorRepairTimer;

	//Timer m_warpScramblerTimer;
	//Timer m_webifierTimer;

};

#endif





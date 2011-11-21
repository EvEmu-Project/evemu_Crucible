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

#include "EVEServerPCH.h"

AsteroidBeltManager::AsteroidBeltManager(MiningDB *db, uint32 belt_id)
: m_beltID(belt_id),
  m_db(db),
  m_growthTimer(ASTEROID_GROWTH_INTERVAL_MS)	//default this to worst case. This will almost certainly be changed with a LoadState()
{
	m_growthTimer.Start();
}

AsteroidBeltManager::~AsteroidBeltManager() {
	delete m_db;
	_Clear();
}

void AsteroidBeltManager::_Clear() {
	std::vector<AsteroidEntity*>::const_iterator cur, end;
	cur = m_asteroids.begin();
	end = m_asteroids.end();
	for(; cur != end; cur++) {
		delete *cur;
	}
}

void AsteroidBeltManager::Process() {
	if(m_growthTimer.Check()) {
		_TriggerGrowth();
	}
}

void AsteroidBeltManager::_TriggerGrowth() {
	std::vector<AsteroidEntity*>::const_iterator cur, end;
	cur = m_asteroids.begin();
	end = m_asteroids.end();
	for(; cur != end; cur++) {
		_log(SERVICE__ERROR, "Asteroid Growth not hooked in yet.");
		//(*cur)->Grow();
	}
}


bool AsteroidBeltManager::LoadState() {
	//load list of asteroids.
	//load next grow time
	return false;	//until this is written.
}

bool AsteroidBeltManager::SaveState() {
	std::vector<AsteroidEntity*>::const_iterator cur, end;
	cur = m_asteroids.begin();
	end = m_asteroids.end();
	for(; cur != end; cur++) {
		//TODO: something useful.
	}
	return false;	//until this is written.
}

void AsteroidBeltManager::ForceGrowth() {
	_TriggerGrowth();
	m_growthTimer.Start(ASTEROID_GROWTH_INTERVAL_MS);
}



















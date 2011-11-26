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

#ifndef __ASTEROIDBELTMANAGER_H_INCL__
#define __ASTEROIDBELTMANAGER_H_INCL__

#include "system/SystemEntities.h"

class AsteroidEntity;
class MiningDB;

static const uint32 ASTEROID_GROWTH_INTERVAL_MS = 3600000;	//RuleI(Mining, AsteroidGrowthInterval_ms)

//There is no solid reason for having this as a different object
//than the SystemEntity subclass, it is just the way the code came out.
//It is very likely that this will need to be merged into the entity object
//once the system matures more.
class AsteroidBeltManager {
public:
	AsteroidBeltManager(MiningDB *db, uint32 belt_id);
	virtual ~AsteroidBeltManager();

	//Database operations.
	virtual bool LoadState();
	virtual bool SaveState();

	virtual void Process();
	virtual void ForceGrowth();

protected:
	const uint32 m_beltID;
	MiningDB *const m_db;	//we own this.

	//runtime state:
	Timer m_growthTimer;
	std::vector<AsteroidEntity*> m_asteroids;	//we own these

	void _TriggerGrowth();

	void _Clear();
};

class SystemAsteroidBeltEntity : public SimpleSystemEntity {
public:
	SystemAsteroidBeltEntity(SystemManager *system, const DBSystemEntity &entity);
	virtual ~SystemAsteroidBeltEntity();

	//SystemEntity:
	virtual void EncodeDestiny( Buffer& into ) const;
	virtual void Process();
	//SimpleSystemEntity:
	virtual bool LoadExtras(SystemDB *db);

protected:
	AsteroidBeltManager *m_manager;	//dynamic to simplify dependancy issues.
};




#endif



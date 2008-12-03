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
	std::vector<Asteroid *>::const_iterator cur, end;
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
	std::vector<Asteroid *>::const_iterator cur, end;
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
	std::vector<Asteroid *>::const_iterator cur, end;
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



















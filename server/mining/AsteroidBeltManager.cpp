
#include "AsteroidBeltManager.h"
#include "MiningDB.h"
#include "Asteroid.h"

#define ASTEROID_GROWTH_INTERVAL_MS 3600000 //RuleI(Mining, AsteroidGrowthInterval_ms)

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
	return(false);	//until this is written.
}

bool AsteroidBeltManager::SaveState() {
	std::vector<Asteroid *>::const_iterator cur, end;
	cur = m_asteroids.begin();
	end = m_asteroids.end();
	for(; cur != end; cur++) {
		//TODO: something useful.
	}
	return(false);	//until this is written.
}

void AsteroidBeltManager::ForceGrowth() {
	_TriggerGrowth();
	m_growthTimer.Start(ASTEROID_GROWTH_INTERVAL_MS);
}



















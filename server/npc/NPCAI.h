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

#ifndef __NPCAI_H_INCL__
#define __NPCAI_H_INCL__

#include "../common/timer.h"

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
	const double m_entityFlyRange2;
	const double m_entityChaseMaxDistance2;
	const double m_entityAttackRange2;
	
	NPC *const m_npc;
	
	Timer m_processTimer;
	Timer m_mainAttackTimer;
};



#endif





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
#ifndef __SYSTEMBUBBLE_H_INCL__
#define __SYSTEMBUBBLE_H_INCL__

#include "../common/gpoint.h"
#include <map>
#include <set>
#include <vector>

class SystemEntity;
class PyRepTuple;
class DoDestiny_SetState;

class SystemBubble {
public:
	SystemBubble(const GPoint &center, double radius);
	
	const GPoint m_center;
	const double m_radius;
	
	void BubblecastDestiny(std::vector<PyRepTuple *> &updates, std::vector<PyRepTuple *> &events, const char *desc) const;
	void BubblecastDestinyUpdate(PyRepTuple **payload, const char *desc) const;
	void BubblecastDestinyEvent(PyRepTuple **payload, const char *desc) const;
	
	bool ProcessWander(std::vector<SystemEntity *> &wanderers);
	
	void Add(SystemEntity *ent, bool notify=true);
	void Remove(SystemEntity *ent, bool notify=true);
	void clear();
	bool IsEmpty() const { return(m_entities.empty()); }
	void GetEntities(std::set<SystemEntity *> &into) const;
	
	void AppendBalls(DoDestiny_SetState &ss, std::vector<byte> &setstate_buffer) const;
	
	bool InBubble(const GPoint &pt) const { return(GVector(m_center, pt).lengthSquared() < m_radius2); }
	
protected:
	void _SendAddBalls(SystemEntity *to_who);
	void _SendRemoveBalls(SystemEntity *to_who);
	void _BubblecastAddBall(SystemEntity *about_who);
	void _BubblecastRemoveBall(SystemEntity *about_who);
	
	const double m_radius2;	//radius squared.
	std::map<uint32, SystemEntity *> m_entities;	//we do not own these.
	std::set<SystemEntity *> m_dynamicEntities;	//entities which may move. we do not own these.
};




#endif



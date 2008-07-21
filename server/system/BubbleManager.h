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
#ifndef __BUBBLEMANAGER_H_INCL__
#define __BUBBLEMANAGER_H_INCL__

#include "../common/timer.h"
#include <vector>

class SystemEntity;
class SystemBubble;
class GPoint;

//the purpose of this object is to make a nice container for
//any of the optimized space searching algorithms which we
// may develop based on bubbles.
// 
// Right now it is stupid and searches for a bubble using a
// linear search. obviously a much better spacial partitioning
// scheme could be written later (octtree or quadtree maybe?)
// and this would be the place to put it.
class BubbleManager {
public:
	BubbleManager();
	~BubbleManager();
	
	void Process();
	
	//call whenever an entity may have left its bubble.
	void UpdateBubble(SystemEntity *ent, bool notify=true);
	//call when an entity is added to the system.
	void Add(SystemEntity *ent, bool notify);
	//call when an entity is removed from the system.
	void Remove(SystemEntity *ent, bool notify);
	void clear();
	
protected:
	SystemBubble * _FindBubble(const GPoint &pos) const;
	
	Timer m_wanderTimer;
	
	//dumb storage for now:
	std::vector<SystemBubble *> m_bubbles;	//we own these. Dynamic only because I am afraid of copy activities.
};





#endif



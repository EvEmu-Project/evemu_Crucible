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

//upon this interval, check for entities which may have wandered out of their bubble without a major event happening.
static const uint32 BubbleWanderTimer_S = 30;
static const double BubbleRadius_m = 1e6;	//made up...

BubbleManager::BubbleManager()
: m_wanderTimer(BubbleWanderTimer_S *1000)
{
	m_wanderTimer.Start();
}

BubbleManager::~BubbleManager() {
	clear();
}

void BubbleManager::clear() {
	std::vector<SystemBubble *>::const_iterator cur, end;
	cur = m_bubbles.begin();
	end = m_bubbles.end();
	for(; cur != end; cur++) {
		delete *cur;
	}
	m_bubbles.clear();
}

void BubbleManager::Process() {
	if(m_wanderTimer.Check()) {
		std::vector<SystemEntity *> wanderers;
		
		{
			std::vector<SystemBubble *>::const_iterator cur, end;
			cur = m_bubbles.begin();
			end = m_bubbles.end();
			for(; cur != end; ++cur) {
				if((*cur)->IsEmpty()) {
					//TODO: nuke this bubble.
				}
				//if wanderers are found, they are 
				(*cur)->ProcessWander(wanderers);
			}
		}
		if(!wanderers.empty()) {
			std::vector<SystemEntity *>::const_iterator cur, end;
			cur = wanderers.begin();
			end = wanderers.end();
			for(; cur != end; cur++) {
				Add(*cur, true);
			}
		}
	}
}

void BubbleManager::UpdateBubble(SystemEntity *ent, bool notify) {
	SystemBubble *b = ent->Bubble();
	if(b != NULL) {
		if(b->InBubble(ent->GetPosition())) {
			_log(DESTINY__BUBBLE_TRACE, "Entity %lu is still located in bubble %p", ent->GetID(), b);
			//still in bubble...
			return;
		}
		_log(DESTINY__BUBBLE_TRACE, "Entity %lu is no longer located in bubble %p", ent->GetID(), b);
		b->Remove(ent, notify);
	}
	Add(ent, notify);
}

void BubbleManager::Add(SystemEntity *ent, bool notify) {

	SystemBubble *in_bubble = _FindBubble(ent->GetPosition());
	if(in_bubble != NULL) {
		in_bubble->Add(ent, notify);
		return;
	}
	//not in any existing bubble.
	
	in_bubble = new SystemBubble(ent->GetPosition(), BubbleRadius_m);
	//TODO: think about bubble colission. should we merge them?
	m_bubbles.push_back(in_bubble);
	in_bubble->Add(ent, notify);
}

void BubbleManager::Remove(SystemEntity *ent, bool notify) {
	SystemBubble *b = ent->Bubble();
	if(b == NULL) {
		//not in any bubble.
		_log(DESTINY__BUBBLE_TRACE, "Entity %lu is not located in any bubble. Nothing to remove.", ent->GetID());
		return;
	}
	b->Remove(ent, notify);
}

//NOTE: this should prolly eventually be optimized to use a
//spacial partitioning scheme to speed up this search.
SystemBubble * BubbleManager::_FindBubble(const GPoint &pos) const {
	std::vector<SystemBubble *>::const_iterator cur, end;
	cur = m_bubbles.begin();
	end = m_bubbles.end();
	for(; cur != end; ++cur) {
		SystemBubble *b = *cur;
		if(b->InBubble(pos)) {
			return(b);
		}
	}
	//not in any existing bubble.
	return(NULL);
}






















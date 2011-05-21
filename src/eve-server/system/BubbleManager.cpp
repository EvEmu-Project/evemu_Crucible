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

//upon this interval, check for entities which may have wandered out of their bubble without a major event happening.
static const uint32 BubbleWanderTimer_S = 30;
static const double BubbleRadius_m = 500000;    // EVE retail uses 250km and allows grid manipulation, for simplicity we dont and have our grid much larger

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
					// Remove this bubble now that it is empty of ALL system entities
                    //delete *cur;
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
			_log(DESTINY__BUBBLE_TRACE, "Entity %u is still located in bubble %p", ent->GetID(), b);
			//still in bubble...
			return;
		}
		_log(DESTINY__BUBBLE_TRACE, "Entity %u is no longer located in bubble %p", ent->GetID(), b);
		b->Remove(ent, notify);
        sLog.Debug( "BubbleManager::UpdateBubble()", "SystemEntity '%s' being removed from Bubble %u", ent->GetName(), b->GetBubbleID() );
	}
	Add(ent, notify);
}

void BubbleManager::Add(SystemEntity *ent, bool notify) {

	SystemBubble *in_bubble = _FindBubble(ent->GetPosition());
	if(in_bubble != NULL) {
		in_bubble->Add(ent, notify);
        sLog.Debug( "BubbleManager::Add()", "SystemEntity '%s' being added to existing Bubble %u", ent->GetName(), in_bubble->GetBubbleID() );
		return;
	}
	// this System Entity is not in any existing bubble, so let's make a new bubble
    // using the current position of this System Entity, however, we want to create this
    // new bubble's center 99.2% of the bubble radius further along the direction
    // of travel from the position of this System Entity
    GPoint newBubbleCenter(ent->GetPosition());
    GVector shipVelocity(ent->GetVelocity());
    shipVelocity.normalize();
    newBubbleCenter.x += shipVelocity.x * (0.96 * BubbleRadius_m);
    newBubbleCenter.y += shipVelocity.y * (0.96 * BubbleRadius_m);
    newBubbleCenter.z += shipVelocity.z * (0.96 * BubbleRadius_m);

	in_bubble = new SystemBubble(newBubbleCenter, BubbleRadius_m);
    sLog.Debug( "BubbleManager::Add()", "SystemEntity '%s' being added to NEW Bubble %u", ent->GetName(), in_bubble->GetBubbleID() );
	//TODO: think about bubble colission. should we merge them?
	m_bubbles.push_back(in_bubble);
	in_bubble->Add(ent, notify);
}

void BubbleManager::Remove(SystemEntity *ent, bool notify) {
	SystemBubble *b = ent->Bubble();
	if(b == NULL) {
		//not in any bubble.
		_log(DESTINY__BUBBLE_TRACE, "Entity %u is not located in any bubble. Nothing to remove.", ent->GetID());
		return;
	}
	b->Remove(ent, notify);
    sLog.Debug( "BubbleManager::Remove()", "SystemEntity '%s' being removed from Bubble %u", ent->GetName(), b->GetBubbleID() );

	std::vector<SystemBubble *>::iterator cur, end;
	cur = m_bubbles.begin();
	end = m_bubbles.end();
	for(; cur != end; ++cur) {
		SystemBubble *b = *cur;
        if(b->IsEmpty()) {
            sLog.Debug( "BubbleManager::Remove()", "Bubble %u is empty and is therefore being deleted from the system right now.", b->GetBubbleID() );
			m_bubbles.erase(cur,cur);
		}
	}
}

//NOTE: this should probably eventually be optimized to use a
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
	return NULL;
}

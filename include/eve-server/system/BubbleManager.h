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
#ifndef __BUBBLEMANAGER_H_INCL__
#define __BUBBLEMANAGER_H_INCL__

#define BUBBLE_RADIUS_METERS 500000.0       // EVE retail uses 250km and allows grid manipulation, for simplicity we dont and have our grid much larger
#define BUBBLE_HYSTERESIS_METERS 5000.0     // How far out of the existing bubble a ship needs to fly before being placed into a new or different bubble

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
	void UpdateBubble(SystemEntity *ent, bool notify=true, bool isWarping=false, bool isPostWarp=false);
	//call when an entity is added to the system.
	void Add(SystemEntity *ent, bool notify, bool isPostWarp=false);
    //call to calculate new bubble's center from entity's velocity:
    void NewBubbleCenter(GVector shipVelocity, GPoint & newBubbleCenter);
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



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
#ifndef __SYSTEMBUBBLE_H_INCL__
#define __SYSTEMBUBBLE_H_INCL__

class SystemEntity;
class PyTuple;
class DoDestiny_SetState;

class SystemBubble {
public:
	SystemBubble(const GPoint &center, double radius);
    ~SystemBubble() { m_bubbleID--; };

	
	const GPoint m_center;
	const double m_radius;
	
	void BubblecastDestiny(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, const char *desc) const;
	void BubblecastDestinyUpdate(PyTuple **payload, const char *desc) const;
	void BubblecastDestinyEvent(PyTuple **payload, const char *desc) const;
	
	bool ProcessWander(std::vector<SystemEntity *> &wanderers);
	
	void Add(SystemEntity *ent, bool notify=true);
	void Remove(SystemEntity *ent, bool notify=true);
	void clear();
	bool IsEmpty() const { return(m_entities.empty()); }
	void GetEntities(std::set<SystemEntity *> &into) const;
    uint32 GetBubbleID() { return m_bubbleID; };

    //void AppendBalls(DoDestiny_SetState &ss, std::vector<uint8> &setstate_buffer) const;

    bool InBubble(const GPoint &pt) const { return(GVector(m_center, pt).lengthSquared() < m_radius2); }
	
protected:
	void _SendAddBalls(SystemEntity *to_who);
	void _SendRemoveBalls(SystemEntity *to_who);
	void _BubblecastAddBall(SystemEntity *about_who);
	void _BubblecastRemoveBall(SystemEntity *about_who);
	
	const double m_radius2;	//radius squared.
    static uint32 m_bubbleIncrementer;
    uint32 m_bubbleID;
	std::map<uint32, SystemEntity *> m_entities;	//we do not own these.
	std::set<SystemEntity *> m_dynamicEntities;	//entities which may move. we do not own these.
};




#endif



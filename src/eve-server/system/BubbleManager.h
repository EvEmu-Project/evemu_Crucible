/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
*/
#ifndef __BUBBLEMANAGER_H_INCL__
#define __BUBBLEMANAGER_H_INCL__


#include <unordered_map>
#include "system/SystemEntity.h"

static const float BUBBLE_RADIUS_METERS = 300000.0f;       // EVE retail uses 250km and allows grid manipulation  NOTE:  this is based on testing for best results.  -allan
static const float BUBBLE_HYSTERESIS_METERS = 5000.0f;     // How far out of the existing bubble a ship needs to fly before being placed into a new or different bubble

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
class BubbleManager
: public Singleton<BubbleManager>
{
public:
    BubbleManager();
    ~BubbleManager();

    int Initialize();
    void Process();

    // call to check for and remove empty bubbles from bubble vector
    void RemoveEmpty();
    //call whenever an entity may have left its bubble.
    void CheckBubble(SystemEntity* ent);
    //call when an entity is added to the system.
    void Add(SystemEntity* pSE, bool isPostWarp = false);
    //call to find the bubble containing the SystemEntity specified, if no bubble does, return NULL
    SystemBubble* FindBubble(SystemEntity *ent) const;
    //call to find the bubble containing the GPoint specified, if no bubble does, return NULL
    SystemBubble* FindBubble(uint32 systemID, const GPoint &pos) const;
    SystemBubble* FindBubbleByID(uint16 bubbleID);
    //find the bubble containing the GPoint specified.  will call create to make new bubble if none found.
    //  this is preferred method to create new bubble.
    SystemBubble* GetBubble(SystemManager* sysMgr, const GPoint &pos);
    //call to calculate new bubble's center from entity's velocity:
    void NewBubbleCenter(GVector shipVelocity, GPoint& newBubbleCenter);
    //call when an entity is removed from the system.
    void Remove(SystemEntity* ent);
    void clear();
    void ClearSystemBubbles(uint32 systemID);
    void RemoveBubble(uint32 systemID, SystemBubble* pSB);

    uint32 Count()                                      { return static_cast<uint32>(m_bubbles.size()); }
    uint32 GetBubbleID()                                { return ++m_bubbleID; }

    // for spawn system     -allan 15April16
    void AddSpawnID(uint16 bubbleID, uint32 spawnID);
    void RemoveSpawnID(uint16 bubbleID, uint32 spawnID);
    uint32 GetBeltID(uint16 bubbleID);

    // for .list command
    uint32 GetBubbleCount(uint32 systemID);

    // for .bubbletrack command
    void MarkCenters(); // for all bubbles, across all systems
    void RemoveMarkers();// for all bubbles, across all systems
    void MarkCenters(uint32 systemID); // for bubbles in given system
    void RemoveMarkers(uint32 systemID);// for bubbles in given system

    // for showing bubble centers in scan window (using .showall)
    void GetBubbleCenterMarkers(std::vector<CosmicSignature>& anom);
    void GetBubbleCenterMarkers(uint32 systemID, std::vector<CosmicSignature>& anom);

protected:
    SystemBubble* MakeBubble(SystemManager* sysMgr, GPoint pos);

private:
    Timer m_emptyTimer;
    Timer m_wanderTimer;

    uint32 m_bubbleID;

    /* map of bubbleID, spawnID */
    std::map<uint16, uint32> m_spawnIDs;

    std::list<SystemBubble*> m_bubbles;                 //for proc only.
    std::vector<SystemEntity*> m_wanderers;             //entities that are no longer in their bubble, but not removed

    std::map<uint32, SystemBubble*> m_bubbleIDMap;     // bubbleID/bubble*

    std::unordered_multimap<uint32, SystemBubble*> m_sysBubbleMap;  // systemID/bubble*
};

//Singleton
#define sBubbleMgr \
( BubbleManager::get() )


#endif

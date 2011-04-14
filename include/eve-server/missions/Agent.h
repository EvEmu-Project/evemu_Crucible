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
#ifndef __AGENT_H_INCL__
#define __AGENT_H_INCL__

class MissionDB;
class Agent;

#if 0

typedef enum {
	//totally made up right now:
	MissionUnseen,
	MissionUnaccepted,
	MissionAccepted
} MissionState;

class AgentMissionSpec {
public:

	uint32		missionID;
	uint8		missionLevel;
	bool		importantMission;
	uint32		missionTypeID;
	std::string	missionTypeName;
	std::string	missionName;
	//bookmarks
};

class AgentMission {
public:
	AgentMission(Agent *_agent, const AgentMissionSpec *_spec, MissionState state, uint64 expirationTime);
	
	PyRep *MakeDetailsRep();

	Agent *agent;	//we do not own this.
	const AgentMissionSpec *spec;	//we do not own this.
	MissionState missionState;
	uint64 expirationTime;
};




class AgentOfferSpec {
public:
	std::string	offerName;
	uint32		loyaltyPoints;
	uint32		requiredISK;
	uint32		rewardISK;
	//this may be too limiting...
	std::map<uint32, uint32> requiredItems;
	std::map<uint32, uint32> offeredItems;
};

class AgentOffer {
public:
	AgentOffer(Agent *_agent, const AgentOfferSpec *_spec, uint64 expirationTime);
	
	PyRep *MakeDetailsRep();

	Agent *agent;	//we do not own this.
	const AgentOfferSpec *spec;	//we do not own this.
	uint64 expirationTime;
};




class AgentResearchSpec {
public:
	uint32		typeID;
	double		ppd;
};

class AgentResearch {
public:
	AgentResearch(Agent *_agent, const AgentResearchSpec *_spec, double points);
	
	PyRep *MakeDetailsRep();

	Agent *agent;	//we do not own this.
	const AgentResearchSpec *spec;	//we do not own this
	double		points;
};







class MissionDB;

class ClientJournal {
public:
	ClientJournal();
	~ClientJournal();
	PyRep *MakeJournalDetails();

	bool Load(MissionDB *from);
	
	std::vector<AgentMission *> missions;	//we own these
	std::vector<AgentOffer *> offers;		//we own these
	std::vector<AgentResearch *> research;	//we own these
};







class AgentMissionSet {
public:
	AgentMissionSet();
	~AgentMissionSet();
	
	bool Load(MissionDB *from);
	
	std::vector<const AgentMissionSpec *> missions;	//we own these
	std::vector<const AgentOfferSpec *> offers;		//we own these
	std::vector<const AgentResearchSpec *> research;	//we own these
};

#endif

class Client;

class AgentActions {
public:
	uint32 actionID;
	uint32 loyaltyPoints;
	std::string agentSays;
	std::map<uint32, std::string> actions;
};

class Agent {
public:
	Agent(uint32 id);
	~Agent();
	
	bool Load(MissionDB *from);

	uint32 GetLoyaltyPoints(Client *who);
	void DoAction(Client *who, uint32 actionID, std::string &say, std::map<uint32, std::string> &choices);

protected:
	const uint32 m_agentID;
	std::map<uint32, AgentActions *> m_actions;	//we own these.
//	AgentLevel *m_agentLevel;
};





#endif



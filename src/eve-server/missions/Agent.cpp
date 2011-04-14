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


Agent::Agent(uint32 id)
: m_agentID(id)
{
}

Agent::~Agent() {
	std::map<uint32, AgentActions *>::iterator cur, end;
	cur = m_actions.begin();
	end = m_actions.end();
	for(; cur != end; cur++) {
		delete cur->second;
	}
}

bool Agent::Load(MissionDB *from) {
	return true;
}

uint32 Agent::GetLoyaltyPoints(Client *who) {
	codelog(AGENT__ERROR, "Unimplemented.");
	return(0);
}

/* It seems as though actionIDs are dynamically assigned at runtime, as they
 * always appear to be sequential, and repeat visits to the same agent will
 * yield different actionIDs for seemingly the same action. No idea if there is
 * any perceived benefit to actually doing that for our case... it could be a
 * mechanism for them to track event ordering (to make sure people do not do
 * missions out of order by only accepting the actions which were actually
 * "allocated" previously.)
 *
 *
 *
*/
void Agent::DoAction(
	Client *who, uint32 actionID,
	std::string &say, std::map<uint32, std::string> &choices
) {
	/*
	if(actionID == 0) {
		//default dialog...
		choices[] = "I need something to do.";	//TODO: randomize this like they do on live
		
	} else {
		
	}
	
	
	
	std::map<uint32, AgentActions *>::iterator res;
	res = m_actions.find(actionID);
	if(res == m_actions.end()) {
		_log(AGENT__ERROR, "Agent %d: Unable to find action %u for '%s'", m_agentID, actionID, c->GetName());
		say = "Invalid Action";
		return;
	}
	AgentActions *action = res->second;
	
	say = action->agentSays;
	choices = action->actions;
	loyaltyPoints = action->loyaltyPoints;
	*/

	say = "What do you want? Spit it out, stooge.";
	choices[1] = "I want work, do you have anything?";
	choices[2] = "I need to find somebody.  Can you help me?";
}




















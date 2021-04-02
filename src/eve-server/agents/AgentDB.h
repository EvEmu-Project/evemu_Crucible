
 /**
  * @name AgentDB.h
  *   Agent DB operations
  *    original agent code by zhur, this was completely rewritten based on new data.
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#ifndef _EVE_SERVER_AGENT_DATABASE_H__
#define _EVE_SERVER_AGENT_DATABASE_H__


#include "../eve-server.h"

#include "packets/Missions.h"
#include "../../eve-common/EVE_Agent.h"

class AgentDB
{
public:
    static void LoadAgentData(uint32 agentID, AgentData& data);
    static void LoadAgentSkills(uint32 agentID, std::map<uint16, uint8>& data);

};

#endif  // _EVE_SERVER_AGENT_DATABASE_H__

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


#ifndef __LSC_SERVICE_H_INCL__
#define __LSC_SERVICE_H_INCL__

#include "../PyService.h"
#include "LSCDB.h"
#include "LSCChannel.h"

#include <map>

/*                                                                              
 *
 * LSC stands for Large Scale Chat
 *
 *
CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
CHTMODE_OPERATOR = ((4 + 2) + 1)
CHTMODE_CONVERSATIONALIST = (2 + 1)
CHTMODE_SPEAKER = 2
CHTMODE_LISTENER = 1
CHTMODE_NOTSPECIFIED = -1
CHTMODE_DISALLOWED = -2 *
 *
 *
CHTERR_NOSUCHCHANNEL = -3
CHTERR_ACCESSDENIED = -4
CHTERR_INCORRECTPASSWORD = -5
CHTERR_ALREADYEXISTS = -6
CHTERR_TOOMANYCHANNELS = -7
CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 *
 *
 *
 *
 *
*/

class CommandDispatcher;

class LSCService : public PyService {
public:
	LSCService(PyServiceMgr *mgr, DBcore *db, CommandDispatcher *cd);
	virtual ~LSCService();

	PyResult ExecuteCommand(Client *from, const char *msg);
	void CreateSystemChannel(uint32 systemID);
	void CharacterLogout(uint32 charID, OnLSC_SenderInfo * si);

	void SendMail(uint32 sender, uint32 recipient, const std::string &subject, const std::string &content) {
		std::vector<uint32> recs(1, recipient);
		SendMail(sender, recs, subject, content);
	}
	void SendMail(uint32 sender, const std::vector<uint32> &recipients, const std::string &subject, const std::string &content);

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	CommandDispatcher *const m_commandDispatch;

	LSCDB m_db;

	std::map<uint32, LSCChannel *> m_channels;	//we own these pointers
	
	//make sure you add things to the constructor too
	PyCallable_DECL_CALL(GetChannels)
	PyCallable_DECL_CALL(GetRookieHelpChannel)
	PyCallable_DECL_CALL(JoinChannels)
	PyCallable_DECL_CALL(LeaveChannels)
	PyCallable_DECL_CALL(LeaveChannel)
	PyCallable_DECL_CALL(CreateChannel)
	PyCallable_DECL_CALL(DestroyChannel)
	PyCallable_DECL_CALL(SendMessage)

	PyCallable_DECL_CALL(GetMyMessages)
	PyCallable_DECL_CALL(GetMessageDetails)
	PyCallable_DECL_CALL(Page)
	PyCallable_DECL_CALL(MarkMessagesRead)
	PyCallable_DECL_CALL(DeleteMessages)

private:
	uint32 nextFreeChannelID;

	LSCChannel *CreateChannel(uint32 channelID, const char * name, const char * motd, LSCChannel::Type type, bool maillist = false);
	LSCChannel *CreateChannel(uint32 channelID, const char * name, LSCChannel::Type type, bool maillist = false);
	LSCChannel *CreateChannel(uint32 channelID, LSCChannel::Type type);
	LSCChannel *CreateChannel(uint32 channelID);
	LSCChannel *CreateChannel(const char * name, bool maillist = false);
	void InitiateStaticChannels();
};




#endif



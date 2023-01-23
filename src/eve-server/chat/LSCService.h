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
    Author:     Zhur, Aknor Jaden
*/


#ifndef __LSC_SERVICE_H_INCL__
#define __LSC_SERVICE_H_INCL__

#include <vector>

//#include "utils/Singleton.h"

#include "chat/LSCDB.h"
#include "chat/LSCChannel.h"

#include "services/Service.h"
#include "admin/SlashService.h"


class CommandDispatcher;
class EVEServiceManager;

class LSCService : public Service <LSCService>
{
public:
    // All user-created chat channels are created with IDs that are in this set:
    //     [baseChannelID,maxChannelID]  (note the inclusivity in that set)
    static const int32 BASE_CHANNEL_ID;
    static const uint32 MAX_CHANNEL_ID;

    LSCService(EVEServiceManager& mgr, CommandDispatcher *cd);
    ~LSCService();

    void Init(CommandDispatcher *cd);

    PyResult ExecuteCommand(Client *from, const char *msg);

    void CharacterLogin(Client *pClient);
    void SendServerMOTD(Client* pClient);

    void CreateSystemChannel(int32 channelID);
    void SystemUnload(uint32 systemID, uint32 constID, uint32 regionID);

    void SendMail(uint32 sender, uint32 recipient, const std::string &subject, const std::string &content) {
        std::vector<int32> recs(1, recipient);
        SendMail(sender, recs, subject, content);
    }
    void SendMail(uint32 sender, const std::vector<int32> &recipients, const std::string &subject, const std::string &content);

protected:
    CommandDispatcher *const m_commandDispatch;

    LSCDB m_db;

    std::map<int32, LSCChannel*> m_channels;  //we own these pointers

    PyResult GetChannels(PyCallArgs& call);
    PyResult GetRookieHelpChannel(PyCallArgs& call);
    PyResult JoinChannels(PyCallArgs& call, PyList* channelIDs, PyLong* role);
    PyResult LeaveChannels(PyCallArgs& call, PyList* channels, PyInt* usubscribe, PyLong* role);
    PyResult LeaveChannels(PyCallArgs& call, PyList* channels, PyBool* usubscribe, PyLong* role);
    PyResult LeaveChannel(PyCallArgs& call, PyRep* channelInfo, PyInt* unsubscribe);
    PyResult CreateChannel(PyCallArgs& call, PyRep* channelName);
    PyResult Configure(PyCallArgs& call, PyInt* channelID);
    PyResult DestroyChannel(PyCallArgs& call, PyInt* channelID);
    PyResult GetMembers(PyCallArgs& call, PyRep* channelInfo);
    PyResult GetMember(PyCallArgs& call);
    PyResult SendMessage(PyCallArgs& call, PyRep* channelInfo, PyWString* cMessage);
    PyResult Invite(PyCallArgs& call, PyInt* characterID, PyInt* channelID);
    PyResult AccessControl(PyCallArgs& call, PyRep* channelInfo, PyInt* characterID, PyInt* role);

    PyResult GetMyMessages(PyCallArgs& call);
    PyResult GetMessageDetails(PyCallArgs& call, PyInt* readerID, PyInt* messageID);
    PyResult Page(PyCallArgs& call, PyList* recipientIDs, PyRep* subject, PyRep* body);
    PyResult MarkMessagesRead(PyCallArgs& call, PyList* messageIDs);
    PyResult DeleteMessages(PyCallArgs& call, PyInt* channelID, PyList* messageIDs);

private:
    SlashService* m_slash;

    void CreateStaticChannels();
    LSCChannel *GetChannelByID(int32 channelID);
    LSCChannel *GetChannelByName(std::string  channelName);
    LSCChannel *CreateChannel(int32 channelID, uint32 ownerID, const char* name, std::string motd, const char* password, const char* compkey,
                              LSC::Type type = LSC::Type::normal, uint32 cspa = 0, int32 groupMessageID = 0, int32 channelMessageID = 0,
                              bool memberless = false, bool maillist = false, bool temporary = false, bool languageRestriction = false);

};

/*  to finish later...
//Singleton
#define sLscSvc \
( LSCService::get() )
*/

#endif

/*{'FullPath': u'UI/Messages', 'messageID': 259365, 'label': u'LSCCannotAccessControlBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259367, 'label': u'LSCCannotCreateBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259369, 'label': u'LSCCannotDestroyTitle'}(u'Cannot Destroy Channel', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259370, 'label': u'LSCCannotDestroyBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259371, 'label': u'LSCCannotJoinTitle'}(u'Cannot Join Channel', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259372, 'label': u'LSCCannotJoinBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259374, 'label': u'LSCCannotRenameBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259376, 'label': u'LSCCannotSendMessageBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259377, 'label': u'LSCCannotSetCSPATitle'}(u'Cannot Configure CSPA', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259378, 'label': u'LSCCannotSetCSPABody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259381, 'label': u'LSCCannotSetCreatorTitle'}(u'Cannot Set Creator', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259382, 'label': u'LSCCannotSetCreatorBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259387, 'label': u'LSCCannotSetMOTDTitle'}(u'Cannot Set Channel MOTD', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259388, 'label': u'LSCCannotSetMOTDBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259389, 'label': u'LSCCannotSetMemberlessTitle'}(u'Cannot Configure Channel', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259390, 'label': u'LSCCannotSetMemberlessBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259391, 'label': u'LSCCannotSetPasswordTitle'}(u'Cannot Set Channel Password', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259392, 'label': u'LSCCannotSetPasswordBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
 */
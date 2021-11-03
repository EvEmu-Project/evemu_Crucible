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
    Rewrite:    Allan  (incomplete)
*/

#include <boost/algorithm/string.hpp>

#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/CommandDispatcher.h"
#include "admin/SlashService.h"
#include "chat/LSCService.h"
#include "fleet/FleetService.h"


/** @todo
 * LSC system todo list...
 * to implement...
 *   channel password
 *   private chat invite
 *   *more*
 *
 * to fix...
 *   channelID system
 *   *more*
 *
 * fleet shit...
 *BOSS/FC/WC/SC should be able to moderate text in their respective fleet chat as needed.
 * They should have the power to dictate text color of their type, and silence people who shouldn't be talking..
 *
 */

// Set the base (minimum) and maximum numbers for any user-created chat channel.
const int32 LSCService::BASE_CHANNEL_ID = 2100000000;      //trial accts are spam-restricted to 1m input buffer when channelID < 2100000000
const uint32 LSCService::MAX_CHANNEL_ID = 0xFFFFFFFF;

PyCallable_Make_InnerDispatcher(LSCService)

LSCService::LSCService(PyServiceMgr *mgr, CommandDispatcher* cd)
: PyService(mgr, "LSC"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd)
{
    _SetCallDispatcher(m_dispatch);

    //make sure you edit the header file too
    PyCallable_REG_CALL(LSCService, GetChannels);
    PyCallable_REG_CALL(LSCService, GetRookieHelpChannel);
    PyCallable_REG_CALL(LSCService, JoinChannels);
    PyCallable_REG_CALL(LSCService, LeaveChannels);
    PyCallable_REG_CALL(LSCService, LeaveChannel);
    PyCallable_REG_CALL(LSCService, CreateChannel);
    PyCallable_REG_CALL(LSCService, Configure);
    PyCallable_REG_CALL(LSCService, DestroyChannel);
    PyCallable_REG_CALL(LSCService, GetMembers);
    PyCallable_REG_CALL(LSCService, GetMember);
    PyCallable_REG_CALL(LSCService, SendMessage);
    PyCallable_REG_CALL(LSCService, Invite);
    PyCallable_REG_CALL(LSCService, AccessControl);

    PyCallable_REG_CALL(LSCService, GetMyMessages);
    PyCallable_REG_CALL(LSCService, GetMessageDetails);
    PyCallable_REG_CALL(LSCService, Page);
    PyCallable_REG_CALL(LSCService, MarkMessagesRead);
    PyCallable_REG_CALL(LSCService, DeleteMessages);

	// sm.RemoteSvc('LSC').VoiceStatus(eveChannelID, 2)  gag
	// sm.RemoteSvc('LSC').VoiceStatus(eveChannelID, 1)  ungag
	// sm.RemoteSvc('LSC').VoiceStatus(eveChannelName, 0) leave channel
    m_db = new LSCDB();

    CreateStaticChannels();

    // make startup msg with # of static channels created
}


LSCService::~LSCService() {
    delete m_dispatch;
    std::map<int32, LSCChannel* >::iterator cur = m_channels.begin();
    for(; cur != m_channels.end(); cur++) {
        SafeDelete(cur->second);
    }
    SafeDelete(m_db);
}

void LSCService::Init(CommandDispatcher* cd )
{
    // not used yet
}

/*
LSC__ERROR=1
LSC__WARNING=0
LSC__MESSAGE=0
LSC__INFO=0
LSC__CHANNELS=0
LSC__CALL_DUMP=0
LSC__RSP_DUMP=0
*/

//  there is no reason to hit the db for everyfuckingthing in this class!!

///////////////////////////////////////////////////////////////////////////////
//
// Eve Chat calls
//
///////////////////////////////////////////////////////////////////////////////

const int cspa = 2950; // CONCORD Spam Prevention Act


PyResult LSCService::Handle_GetChannels(PyCallArgs &call)
{
    ChannelInfo info;
    info.lines = new PyList();

    uint32 regionID = call.client->GetRegionID(), constID = call.client->GetConstellationID(), systemID = call.client->GetSystemID();
    uint32 corpID = call.client->GetCorporationID();

    std::map<int32, LSCChannel*>::iterator cur = m_channels.begin();
    for (; cur != m_channels.end(); cur++) {
        switch (cur->second->GetType()) {
            case LSC::Type::corp: {
                if (cur->first != corpID)
                    continue;
            } break;
            case LSC::Type::region:
            case LSC::Type::constellation:
            case LSC::Type::solarsystem:
            case LSC::Type::solarsystem2:
            case LSC::Type::fleet:
            case LSC::Type::wing:
            case LSC::Type::squad:
            case LSC::Type::warfaction:
            case LSC::Type::incursion:
            case LSC::Type::custom: {
                continue;
            } break;
        }

        info.lines->AddItem(cur->second->EncodeStaticChannel(call.client->GetCharacterID()));
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        info.Dump(LSC__RSP_DUMP);
    return info.Encode();
}


PyResult LSCService::Handle_GetRookieHelpChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_GetRookieHelpChannel()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    return PyStatic.NewOne();
}

PyResult LSCService::Handle_CreateChannel(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_CreateChannel()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    Call_SingleStringArg name;
    if (!name.Decode(&call.tuple))  {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    bool create = false, temporary = false, joinExisting = false, noCallThrottling = false, memberless = false;
    if (call.byname.find("create") != call.byname.end())
        create =  call.byname.find("create")->second->AsBool()->value();
    if (call.byname.find("temporary") != call.byname.end())
        temporary = call.byname.find("temporary")->second->AsBool()->value();
    if (call.byname.find("joinExisting") != call.byname.end())
        joinExisting = call.byname.find("joinExisting")->second->AsBool()->value();
    if (call.byname.find("noCallThrottling") != call.byname.end())
        noCallThrottling = call.byname.find("noCallThrottling")->second->AsBool()->value();
    if (call.byname.find("memberless") != call.byname.end())
        memberless = (call.byname.find("memberless")->second->AsInt()->value() ? true : false);

    Client* pClient = call.client;
    ChannelCreateReply reply;

    // create/get channel info
    LSCChannel* channel(nullptr);
    if (joinExisting) {
        channel = GetChannelByName(name.arg);
    } else if (create)  {
        // figure out how to determine owner of this channel.....corp, ally, char.  for now, use charID
        int32 nextID = m_db->GetNextAvailableChannelID();
        if (nextID != 0) {
            std::string comStr = name.arg;
            boost::algorithm::to_lower(comStr);
            comStr.erase(std::remove(comStr.begin(), comStr.end(), ' '), comStr.end());
            channel = CreateChannel(nextID, pClient->GetCharacterID(), name.arg.c_str(), "motd", nullptr, comStr.c_str(), LSC::Type::normal, cspa,\
            nextID, nextID, memberless, false, temporary, false);
        }
        // save non-temp channel info for new channels
        if ((channel) and (!temporary))
            m_db->UpdateChannelInfo(channel);
    } else {
        // make error here for !join and !create  (should never hit)
    }

    // test for channel info
    if (!channel) {
        if (joinExisting) {
            _log(LSC__ERROR, "%s: Channel not found - %s", pClient->GetName(), name.arg.c_str());
            reply.ChannelInfo = new PyInt(LSC::Error::errNoSuchChannel);
        } else if (create)  {
            _log(LSC__ERROR, "%s: Channel not created - %s", pClient->GetName(), name.arg.c_str());
            reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
        } else {
            // make error here for !join and !create  (should never hit)
        }
        return reply.Encode();
    }

    // join channel and send response
    if (!channel->IsJoined(pClient->GetCharacterID()))  {
        if (channel->JoinChannel(pClient)) {
            //if ((channel->GetChannelID() < 0) or (channel->GetChannelID() > maxStaticChannel)) {
                reply.ChannelInfo = channel->EncodeDynamicChannel(pClient->GetCharacterID());
            //} else {
            //    reply.ChannelInfo = channel->EncodeStaticChannel(pClient->GetCharacterID());
            //}
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelMods = channel->EncodeChannelMods();
        } else {
            reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
        }
        if (!temporary)
            m_db->UpdateSubscription(channel->GetChannelID(), pClient);

        return reply.Encode();
    } else {
        _log(LSC__ERROR, "%s: Already joined Channel %i \"%s\".", pClient->GetName(), channel->GetChannelID(), channel->GetDisplayName().c_str());
        reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
        return reply.Encode();
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        reply.Dump(LSC__RSP_DUMP);

    /*
     *            ret = sm.RemoteSvc('LSC').CreateChannel(displayName, joinExisting=False, memberless=0, create=True)
     *            if ret:
     *                info, acl, memberList = ret
     *        // on fail,  ChannelInfo is pyint(lsc::type::error), others are null
     *            if info == CHTERR_ALREADYEXISTS:
     *            if info == CHTERR_NOSUCHCHANNEL:
     */
    return nullptr;
}

PyResult LSCService::Handle_JoinChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_JoinChannels()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallJoinChannels args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    std::set<int32> toJoin;

    PyList::const_iterator cur = args.channels->begin();
    for (; cur != args.channels->end(); cur++) {
        if ((*cur)->IsInt()) {
            toJoin.insert((*cur)->AsInt()->value());
        } else if ((*cur)->IsTuple()) {
            PyTuple* prt = (*cur)->AsTuple();
            if (prt->items.size() != 1 or !prt->items[0]->IsTuple()) {
                _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                continue;
            }
            prt = prt->items[0]->AsTuple();

            if (prt->items.size() != 2 or /* !prt->items[0]->IsString() or unnessecary */ !prt->items[1]->IsInt()) {
                _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                continue;
            }
            toJoin.insert(prt->items[1]->AsInt()->value());
        } else {
            codelog(SERVICE__ERROR, "%s: Failed to decode argument ", call.client->GetName());
            return nullptr;
        }
    }

    PyList *ml = new PyList();
    LSCChannel* channel(nullptr);
    uint32 charID = call.client->GetCharacterID();

    std::set<int32>::iterator curs = toJoin.begin();
    for (; curs != toJoin.end(); curs++) {
        int32 channelID = *curs;
        if (channelID == 0)
            continue;
        if (sConfig.chat.EnforceRookieInHelp)
            if ((channelID == 1) or (channelID == 2))
                if (((args.role & Acct::Role::NEWBIE) != Acct::Role::NEWBIE) or ((args.role & Acct::Role::EPLAYER) != Acct::Role::EPLAYER))
                    continue;

        channel = GetChannelByID(channelID);
        if (channel == nullptr)
            continue;
        ChannelJoinReply chjr;
            chjr.ChannelID = channel->EncodeID();

        /** @todo  query/check password and other stipulations */
        if (!channel->IsJoined(charID)) {
            if (channel->JoinChannel(call.client)) {
                ChannelJoinOK cjok;
                //if ((channelID < 0) or (channelID > maxStaticChannel)) {
                    cjok.ChannelInfo = channel->EncodeDynamicChannel(charID);
                //} else {
                //    cjok.ChannelInfo = channel->EncodeStaticChannel(charID);
                //}
                cjok.ChannelMods = channel->EncodeChannelMods();
                cjok.ChannelChars = channel->EncodeChannelChars();
                chjr.JoinRsp = cjok.Encode();
                chjr.ok = 1;
            } else {
                ChannelJoinNotOK cjnok;
                    cjnok.Error = "LSCCannotJoin";
                    cjnok.rspDict = new PyDict();   // dunno what goes here...
                chjr.JoinRsp = cjnok.Encode();
                chjr.ok = 0;
            }
        } else {
            ChannelJoinNotOK cjnok;
            //{'FullPath': u'UI/Messages', 'messageID': 256739, 'label': u'LSCChannelIsJoinedBody'}(u'You are already in the channel ({displayName})', None, {u'{displayName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'displayName'}})
                cjnok.Error = "LSCChannelIsJoined";
                cjnok.rspDict = new PyDict();   // dunno what goes here...
            chjr.JoinRsp = cjnok.Encode();
            chjr.ok = 0;
        }
        ml->AddItem(chjr.Encode());
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        ml->Dump(LSC__RSP_DUMP, "   ");
    return ml;
}

PyResult LSCService::Handle_SendMessage(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_SendMessage()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    int32 channel_id = 0;
    std::string message = "";

    if ((call.tuple->IsTuple()) and (call.tuple->AsTuple()->items[0]->IsInt())) {
        // Decode All User-created chat channel messages here:
        if (!call.tuple->IsTuple()) {
            _log(LSC__ERROR, "LSCService::Handle_SendMessage failed: tuple0 is the wrong type: %s", call.tuple->TypeString());
            return PyStatic.NewNone();
        }
        PyTuple* tuple0 = call.tuple->AsTuple();

        if (tuple0->size() != 2) {
            _log(LSC__ERROR, "LSCService::Handle_SendMessage failed: tuple0 is the wrong size: expected 2, but got %u", tuple0->size());
            return PyStatic.NewNone();
        }

        channel_id = call.tuple->AsTuple()->items[0]->AsInt()->value();
        message = call.tuple->AsTuple()->items[1]->AsWString()->content();
        _log(LSC__INFO, "Handle_SendMessage: call is player channel chat.");
    } else {
        Call_SendMessage args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewNone();
        }
        channel_id = args.channel.id;
        message = args.message;
        _log(LSC__INFO, "Handle_SendMessage: call is system channel chat.");
    }

    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channel_id);
    if (itr == m_channels.end()) {
        _log(LSC__ERROR, "%s: Couldn't find channel %u", call.client->GetName(), channel_id);
        return PyStatic.NewNone();
    }

    if (message.at(0) == '.') {
        _log(LSC__INFO, "SlashService->SlashCmd() called via LSC Service");
        static_cast<SlashService *>(m_manager->LookupService("slash"))->SlashCommand(call.client, message);
        itr->second->SendMessage(call.client, message.c_str(), true);
    } else {
        itr->second->SendMessage(call.client, message.c_str());
    }

    return PyStatic.NewNone();
}

PyResult LSCService::Handle_AccessControl(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_AccessControl()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }
    /*  args passed as channelID, charID, mode
     *
     * 20:43:44 W LSCService::Handle_AccessControl(): size=3
     * 20:43:44 [LSC_CDump]   Call Arguments:
     * 20:43:44 [LSC_CDump]       Tuple: 3 elements
     * 20:43:44 [LSC_CDump]         [ 0] Integer field: 2100000000
     * 20:43:44 [LSC_CDump]         [ 1] (None)
     * 20:43:44 [LSC_CDump]         [ 2] Integer field: 1
     * 20:43:44 [LSC_CDump]   Call Named Arguments:
     * 20:43:44 [LSC_CDump]     Argument 'machoVersion':
     * 20:43:44 [LSC_CDump]         Integer field: 1
     */

    // WARNING: This call contains manual packet decoding to handle Access Control since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    int32 channel_id = 0;

    //m_db->UpdateChannelInfo(channel);

    // BIG TODO:  The whole reason why normal players cannot post chats in other channels has to do with the Access Mode
    // in the channel settings dialog in the client.  Now, I don't know why chatting in the Help/Rookie Help is not allowed
    // for ANY normal players, however, at the very least, implementing the change in Access Mode to 3 = Allowed may fix
    // the issue.  The only thing to figure out is how to format a packet to be sent to the client(s) to inform of the change
    // in access mode.  Is this really needed though, since the owner will change the mode and the owner's client will know
    // immediately, and anyone wanting to join will get that mode value when the JoinChannel has been called.

    // call.tuple->GetItem(0)->AsInt()->value() = channel ID
    // call.tuple->GetItem(1)->IsNone() == true  <---- change made to "" field
    // call.tuple->GetItem(2)->AsInt()->value() =
    //     0 = ??
    //     1 = Moderated
    //     2 = ??
    //     3 = Allowed

    // call.tuple->GetItem(1)->IsInt() == true  <---- character ID for character add to one of the lists specified by GetItem(2):
    // call.tuple->GetItem(2)->AsInt()->value() =
    //     3 = Add to Allowed List
    //     -2 = Add to Blocked List
    //     7 = Add to Moderators List

    //channel->UpdateConfig();

    return PyStatic.NewOne();
}

PyResult LSCService::Handle_Invite(PyCallArgs &call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_Invite()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle chat messages sent inside user-created
    // chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-19)

    LSCChannel* channel(nullptr);

    uint32 channel_ID;
    uint32 char_ID = call.client->GetCharacterID();
    uint32 invited_char_ID;

    // Decode the call:
    if (call.tuple->IsTuple()) {
        if (call.tuple->GetItem(1)->IsInt()) {
            channel_ID = call.tuple->GetItem(1)->AsInt()->value();
        } else {
            _log(LSC__ERROR, "%s: call.tuple->GetItem(1) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->TypeString());
            return nullptr;
        }

        if (call.tuple->GetItem(0)->IsInt()) {
            invited_char_ID = call.tuple->GetItem(0)->AsInt()->value();
        } else {
            _log(LSC__ERROR, "%s: call.tuple->GetItem(0) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->TypeString());
            return nullptr;
        }
    } else {
        _log(LSC__ERROR, "%s: call.tuple is of the wrong type: '%s'.  Expected PyTuple type.", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    if (m_channels.find(channel_ID) != m_channels.end()) {
        channel = m_channels[channel_ID];

        if (!channel->IsJoined(invited_char_ID)) {
            // SOMEHOW SEND A JOIN COMMAND/REQUEST TO THE TARGET CLIENT FOR invited_char_ID
            /*    OnLSC_JoinChannel join;
             *            join.sender = channel->_MakeSenderInfo(call.client);
             *            join.member_count = 1;
             *            join.channelID = channel->EncodeID();
             *            PyTuple *answer = join.Encode();
             *            MulticastTarget mct;
             *            //LSCChannelChar *invitor;
             *            //LSCChannelChar *invitee;
             *            if (!channel->IsJoined(char_ID))
             *            {
             *                //invitor = new LSCChannelChar(channel,0,char_ID,call.client->GetCharName(),0,0,0,0);
             *                mct.characters.insert(char_ID);
        }
        //invitee = new LSCChannelChar(channel,0,invited_char_ID,entityList().FindCharacter(invited_char_ID)->GetCharName(),0,0,0,0);
        mct.characters.insert(invited_char_ID);
        entityList().Multicast("OnLSC", channel->GetTypeString(), &answer, mct);
        //entityList().Unicast(invited_char_ID,"OnLSC",channel->GetTypeString(),&answer,false);
        */

            // ********** TODO **********
            // Figure out how to send the ChatInvite packet to the client running the character with id = 'invited_char_ID'
            // in order for that character's client to then issue the JoinChannels call to the server with the chat channel
            // ID equal to that of this channel, be it either a private convo (temporary==1) or an existing user-created chat.
            // **************************

            //ChatInvite chatInvitePacket;
            //chatInvitePacket.integer1 = 1;
            //chatInvitePacket.integer2 = invited_char_ID;
            //chatInvitePacket.boolean = true;
            //chatInvitePacket.displayName = call.tuple->GetItem(2)->AsString()->content();
            //chatInvitePacket.integer3 = 1;
            //chatInvitePacket.integer4 = 0;
            //chatInvitePacket.integer5 = 1;
            //PyTuple *tuple = chatInvitePacket.Encode();
            //entityList().Unicast(invited_char_ID, "", "", &tuple, false);
        } else {
            _log(LSC__ERROR, "%s: Character %u is already joined to channel %u.", call.client->GetName(), invited_char_ID, channel_ID);
            return nullptr;
        }
    } else {
        _log(LSC__ERROR, "%s: Cannot find channel %u.", call.client->GetName(), channel_ID);
        return nullptr;
    }

    return PyStatic.NewOne();
}

PyResult LSCService::Handle_Configure(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_Configure()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }
    /*
     * 20:43:44 W LSCService::Handle_Configure(): size=1
     * 20:43:44 [LSC_CDump]   Call Arguments:
     * 20:43:44 [LSC_CDump]       Tuple: 1 elements
     * 20:43:44 [LSC_CDump]         [ 0] Integer field: 2100000000
     * 20:43:44 [LSC_CDump]   Call Named Arguments:
     * 20:43:44 [LSC_CDump]     Argument 'machoVersion':
     * 20:43:44 [LSC_CDump]         Integer field: 1
     * 20:43:44 [LSC_CDump]     Argument 'motd':
     * 20:43:44 [LSC_CDump]         WString: 'test MOTD'
     *
     * 21:13:02 W LSCService::Handle_Configure(): size=1
     * 21:13:02 [LSC_CDump]   Call Arguments:
     * 21:13:02 [LSC_CDump]       Tuple: 1 elements
     * 21:13:02 [LSC_CDump]         [ 0] Integer field: 2100000000
     * 21:13:02 [LSC_CDump]   Call Named Arguments:
     * 21:13:02 [LSC_CDump]     Argument 'creator':
     * 21:13:02 [LSC_CDump]         Integer field: 140000130
     * 21:13:02 [LSC_CDump]     Argument 'machoVersion':
     * 21:13:02 [LSC_CDump]         Integer field: 1
     *
     */

    // WARNING: This call contains manual packet decoding to handle configuring parameters for
    // user-created chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    LSCChannel* channel(nullptr);
    int32 channel_id = 0;

    //ChannelInfo args;
    //if (!args.Decode(&call.tuple) {
    //codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
    //    return nullptr;
    //}

    // Get Tuple which contains channel number to modify:
    if (call.tuple->AsTuple()->GetItem(0)->IsInt()) {
        channel_id = call.tuple->AsTuple()->GetItem(0)->AsInt()->value();
    } else {
        _log(LSC__ERROR, "%s: Tuple contained wrong type: '%s'", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    // Get count of parameters or just loop through the std::map until you've reached the end
    if (call.byname.size() == 0) {
        _log(LSC__ERROR, "%s: byname std::map contained zero elements, expected at least one.", call.client->GetName());
        return nullptr;
    }

    // Find channel in existing channels:
    std::map<int32, LSCChannel*>::iterator res = m_channels.find(channel_id);
    if (m_channels.end() == res)  {
        _log(LSC__ERROR, "%s: Handle_Configure Couldn't find channel %u", call.client->GetName(), channel_id);
        return nullptr;
    }

    channel = m_channels.find(channel_id)->second;
    if (call.byname.find("displayName") != call.byname.end()) {
        if (call.byname.find("displayName")->second->IsWString()) {
            channel->SetDisplayName(call.byname.find("displayName")->second->AsWString()->content());
        } else {
            _log(LSC__ERROR, "%s: displayName contained wrong type: '%s'", call.client->GetName(), call.byname.find("displayName")->second->TypeString());
            return nullptr;
        }
    }

    if (call.byname.find("memberless") != call.byname.end()) {
        if (call.byname.find("memberless")->second->IsInt()) {
            channel->SetMemberless(call.byname.find("memberless")->second->AsInt()->value() ? true : false);
        } else {
            _log(LSC__ERROR, "%s: memberless contained wrong type: '%s'", call.client->GetName(), call.byname.find("memberless")->second->TypeString());
            return nullptr;
        }
    }

    if (call.byname.find("motd") != call.byname.end()) {
        if (call.byname.find("motd")->second->IsWString()) {
            channel->SetMOTD(call.byname.find("motd")->second->AsWString()->content());
        } else {
            _log(LSC__ERROR, "%s: motd contained wrong type: '%s'", call.client->GetName(), call.byname.find("motd")->second->TypeString());
            return nullptr;
        }
    }

    if (call.byname.find("oldPassword") != call.byname.end()) {
        if (call.byname.find("oldPassword")->second->IsWString()) {
            if (channel->GetPassword() == call.byname.find("oldPassword")->second->AsWString()->content()) {
                if (call.byname.find("newPassword") != call.byname.end()) {
                    if (call.byname.find("newPassword")->second->IsWString()) {
                        channel->SetPassword(call.byname.find("newPassword")->second->AsWString()->content());
                    } else {
                        _log(LSC__ERROR, "%s: newPassword contained wrong type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString());
                        return nullptr;
                    }
                }
            } else {
                _log(LSC__ERROR, "%s: incorrect oldPassword supplied. Password NOT changed.", call.client->GetName());
                return nullptr;
            }
        } else if (call.byname.find("oldPassword")->second->IsNone()) {
            if (call.byname.find("newPassword") != call.byname.end()) {
                if (call.byname.find("newPassword")->second->IsWString()) {
                    channel->SetPassword(call.byname.find("newPassword")->second->AsWString()->content());
                } else {
                    _log(LSC__ERROR, "%s: newPassword contained wrong type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString());
                    return nullptr;
                }
            }
        } else {
            _log(LSC__ERROR, "%s: oldPassword is of an unexpected type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString());
            return nullptr;
        }
    }

    m_db->UpdateChannelInfo(channel);

    channel->UpdateConfig();

    return PyStatic.NewNone();
}


PyResult LSCService::Handle_LeaveChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_LeaveChannel()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannel arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    uint32 toLeave = 0;

    if (arg.channel->IsInt()) {
        toLeave = arg.channel->AsInt()->value();
    } else if (arg.channel->IsTuple()) {
        PyTuple* prt = arg.channel->AsTuple();

        if (prt->GetItem(0)->IsInt()) {
            toLeave = prt->GetItem(0)->AsInt()->value();
        } else if (prt->GetItem(0)->IsTuple()) {
            prt = prt->GetItem(0)->AsTuple();

            if (prt->items.size() != 2 or !prt->GetItem(1)->IsInt()) {
                _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                return PyStatic.NewNone();
            }

            toLeave = prt->GetItem(1)->AsInt()->value();
        } else {
            _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return PyStatic.NewNone();
        }
    } else {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    if (arg.unsubscribe)
        m_db->DeleteSubscription(toLeave,call.client->GetCharacterID());

    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(toLeave);
    if (itr != m_channels.end()) {
        itr->second->LeaveChannel(call.client);
        if ((itr->second->GetMemberCount() < 1) and (itr->second->GetTemporary())) {
            m_db->DeleteChannel(toLeave);
            SafeDelete(itr->second);
            m_channels.erase(itr);
        }
    }

    return PyStatic.NewNone();
}


PyResult LSCService::Handle_LeaveChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_LeaveChannels()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannels args;

    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    std::set<uint32> toLeave;
    toLeave.clear();

    PyList::const_iterator cur = args.channels->begin();
    for (; cur != args.channels->end(); cur++) {
        if ((*cur)->IsInt()) {
            toLeave.insert((*cur)->AsInt()->value());
        } else if ((*cur)->IsTuple()) {
            PyTuple* prt = (*cur)->AsTuple();

            if (prt->GetItem(0)->IsInt()) {
                toLeave.insert(prt->GetItem(0)->AsInt()->value());
                continue;
            }

            if (!prt->GetItem(0)->IsTuple()) {
                _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                continue;
            }
            prt = prt->GetItem(0)->AsTuple();

            if (prt->GetItem(0)->IsTuple())
                prt = prt->GetItem(0)->AsTuple();

            if (prt->size() != 2 or !prt->GetItem(1)->IsInt()) {
                _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                continue;
            }

            toLeave.insert(prt->GetItem(1)->AsInt()->value());
        } else {
            _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            continue;
        }
    }

    std::set<uint32>::iterator itr = toLeave.begin();
    std::map<int32, LSCChannel*>::iterator itr2;
    for (; itr!=toLeave.end(); itr++) {
        itr2 = m_channels.find(*itr);
        if (itr2 != m_channels.end()) {
            itr2->second->LeaveChannel(call.client);
            if (args.unsubscribe)
                m_db->DeleteSubscription(*itr, call.client->GetCharacterID());

            if ((itr2->second->GetMemberCount() < 1) and (itr2->second->GetTemporary())) {
                m_db->DeleteChannel(*itr);
                SafeDelete(itr2->second);
                m_channels.erase(itr2);
            }
        }
    }

    return PyStatic.NewNone();
}

PyResult LSCService::Handle_DestroyChannel(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_DestroyChannel()", "size=%lu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(arg.arg);
    if (itr == m_channels.end()) {
        _log(LSC__ERROR, "%s: Couldn't find channel %u", call.client->GetName(), arg.arg);
        return PyStatic.NewNone();
    }

    // ********** TODO **********
    // Figure out how to validate whether this character (call.client->GetCharacterID()) is allowed
    // to destroy this chat channel, and proceed if they are, otherwise, do not.  And, is there an error
    // packet sent back to the client?
    // **************************

    // Now, remove the channel from the database:
    m_db->DeleteChannel(itr->second->GetChannelID());

    // Finally, remove the channel from the server dynamic objects:
    itr->second->Evacuate(call.client);
    SafeDelete(itr->second);
    m_channels.erase(itr);

    return PyStatic.NewNone();
}


PyResult LSCService::Handle_GetMembers(PyCallArgs &call) {
    CallGetMembers arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 channelID;
    if (arg.channel->IsInt()) {
        channelID = arg.channel->AsInt()->value();
    } else if (arg.channel->IsTuple()) {
        PyTuple* prt = arg.channel->AsTuple();

        if (prt->GetItem(0)->IsInt()) {
            channelID = prt->GetItem(0)->AsInt()->value();
        } else if (prt->GetItem(0)->IsTuple()) {
            prt = prt->GetItem(0)->AsTuple();

            if (prt->items.size() != 2 or !prt->GetItem(1)->IsInt())
            {
                _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
                return nullptr;
            }

            channelID = prt->GetItem(1)->AsInt()->value();
        } else {
            _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return nullptr;
        }
    } else {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (m_channels.find(channelID) != m_channels.end())
        return m_channels[channelID]->EncodeChannelChars();

    return nullptr;
}


PyResult LSCService::Handle_GetMember(PyCallArgs &call) {
    return nullptr;
}


void LSCService::CharacterLogin(Client* pClient)
{
    CreateSystemChannel(pClient->GetCorporationID());
    CreateSystemChannel(pClient->GetAllianceID());
}

void LSCService::SystemUnload(uint32 systemID, uint32 constID, uint32 regionID)
{
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(systemID);
    if (itr != m_channels.end()) {
        SafeDelete(itr->second);
        m_channels.erase(itr);
    }
    /** @todo  find a way to track usages of region and const channels to delete when no longer used */
    //  is this needed?
}

LSCChannel* LSCService::CreateChannel(int32 channelID, uint32 ownerID, const char *name, std::string motd, const char *password, const char *compkey,
                                      LSC::Type type/*LSC::Type::normal*/, uint32 cspa/*0*/, int32 groupMessageID/*0*/, int32 channelMessageID/*0*/, bool memberless/*false*/,
                                      bool maillist/*false*/, bool temporary/*false*/, bool languageRestriction/*false*/) {
    // test for invalid channelID (alliance not implemented yet)
    if (channelID == 0)
        return nullptr;

    LSCChannel* channel = new LSCChannel(this, channelID, type, ownerID, name, compkey, motd, memberless, password, maillist, cspa, temporary, languageRestriction, groupMessageID, channelMessageID);
    return (m_channels[channelID] = channel);
}

LSCChannel* LSCService::GetChannelByID(int32 channelID)
{
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return nullptr;
}

LSCChannel* LSCService::GetChannelByName(std::string channelName)
{
    for (auto cur : m_channels)
        if (cur.second->GetDisplayName() == channelName)
            return cur.second;
    return nullptr;
}

void LSCService::CreateSystemChannel(int32 channelID)
{
    if (channelID == 0)
        return;
    if (m_channels.find(channelID) != m_channels.end())
        return;

    LSC::Type type = LSC::Type::normal;
    std::string name= "", motd = "";
    int32 messageID = -1, grpMsgID = 0;
    uint32 ownerID = channelID;

    /** @todo  i know this isnt used much, but wtf was i thinking hitting db for names, when they are in static data?? */
    if (IsRegionID(channelID)) {
        type = LSC::Type::region;
        name = "Region";
        motd = m_db->GetRegionName(channelID);
        grpMsgID = 1;
    } else if (IsConstellationID(channelID)) {
        type = LSC::Type::constellation;
        name = "Constellation";
        motd = m_db->GetConstellationName(channelID);
        grpMsgID = 2;
    } else if (IsKSpaceID(channelID)) {
        type = LSC::Type::solarsystem2;
        name = "Local";
        motd = m_db->GetSolarSystemName(channelID);
        grpMsgID = 3;
    } else if (IsWSpaceID(channelID)) {
        type = LSC::Type::solarsystem;
        name = "System";
        motd = m_db->GetSolarSystemName(channelID);
        grpMsgID = 3;
    } else if (IsNPCCorp(channelID)) {
        type = LSC::Type::corp;
        name = "Corp";
        motd = m_db->GetCorporationName(channelID);
        grpMsgID = 263235;
    } else if (IsPlayerCorp(channelID)) {
        type = LSC::Type::corp;
        name = "Corp";
        motd = m_db->GetCorporationName(channelID);
        grpMsgID = 263235;
        messageID = 0;
    } else if (IsAlliance(channelID)) {
        type = LSC::Type::alliance;
        name = "Alliance";
        motd = m_db->GetAllianceName(channelID);
        grpMsgID = 5;
        messageID = 0;
    } else if (IsFleetID(channelID)) {
        type = LSC::Type::fleet;
        name = sFltSvc.GetFleetName(channelID);
        motd = sFltSvc.GetFleetDescription(channelID);
        messageID = 0;
        ownerID = sFltSvc.GetFleetLeaderID(channelID);
    } else if (IsWingID(channelID)) {
        type = LSC::Type::wing;
        name = sFltSvc.GetWingName(channelID);
        motd = name;
        motd += "<br>";
        motd += sFltSvc.GetFleetDescription(channelID);
        messageID = 0;
        //ownerID = sFltSvc.GetWingLeaderID(channelID);
        ownerID = sFltSvc.GetFleetLeaderID(channelID);
    } else if (IsSquadID(channelID)) {
        type = LSC::Type::squad;
        name = sFltSvc.GetSquadName(channelID);
        motd = name;
        motd += "<br>";
        motd += sFltSvc.GetFleetDescription(channelID);
        messageID = 0;
        //ownerID = sFltSvc.GetSquadLeaderID(channelID);
        ownerID = sFltSvc.GetFleetLeaderID(channelID);
    } else {
        // not sure what to do here....should never hit
    }
    std::string comStr = motd;
    boost::algorithm::to_lower(comStr);
    comStr.erase(std::remove(comStr.begin(), comStr.end(), ' '), comStr.end());
    CreateChannel(channelID, ownerID, name.c_str(), motd, nullptr, comStr.c_str(), type, cspa, grpMsgID, messageID);
}

void LSCService::CreateStaticChannels() {
    // hardcode creating server static channels during server startup
    // these are all set to memberless to avoid constant updates
    std::ostringstream str;

// basic template
    //CreateChannel(channelID, ownerID, "name", "motd", "password" or nullptr, "*title*", LSC::Type::normal, cspa, 0, 0, true);

//Help
// Incursion Rookie Help MOTD
    str.str("");
    str << "<br><color=0xff007fff><b>Welcome to the EVEmu <u>EVE Online: Crucible</u> Emulator</b></color><br><br>";
    str << "<color=0xff00ff00><b>Topic:</b></color>  ";
    str << "<color=0xffffffff>EVE-Online related rookie help.</color><br><br>";
    str << "<color=0xff00ff00><b>Rules:</b></color>  ";
    str << "<color=0xffffffff>No WTB, WTS, WTT, PC, advertising, recruiting, scamming, offering private help in any form or begging in this channel.<br>";
    str << "No CAPS or text-decoration.</color><br><br>";
    str << "<color=0xff00ff00><b>Language:</b></color>  ";
    str << "<color=0xffffffff>This channel is ENGLISH ONLY.</color><br><br>";
    str << "<color=0xff00ff00><b>How to contact a GM:</b> </color>  ";
    str << "<color=0xffffffff>Right now, our petition service isnt operational, so to contact a GM, post on the forums.</color><br><br>";
    //str << "<color=0xffffffff>File a petition (F12 - Petitions - New Petition)</color><br><br>";
    str << "<color=0xff00ff00><b>ISK Advertising:</b></color>  ";
    str << "<color=0xffff0000>Contrary to what they spam, EVEmu does not authorize any person or site to sell ISK for RL cash.</color><br><br>";
    str << "<color=0xff00ff00><b>Please Note:</b></color><br>";
    str << "<color=0xffffffff>There are no third party applications (.exe) that will magically give you any type of ship you wish or hack your wallet. Please report characters advertising these types of links IMMEDIATELY via petition and DO NOT download and try them. <br><br>";
    // str << "Sister of EVE storyline starts in <url=showinfo:5//30005001><u>Arnon</url></b></u> <b>with <url=showinfo:1378//3019356><u>Sister Alitura</url></b></u>. <br><br></color>";
    CreateChannel(1, 1, "Rookie Help", str.str().c_str(), nullptr, "help", LSC::Type::normal, cspa, 263238, -1, true);
// Incursion Help MOTD
    str.str("");
    str << "<br><color=0xff007fff><b>Welcome to the EVEmu <u>EVE Online: Crucible</u> Emulator</color><br><br>";
    str << "<color=0xff00ff00><b>Topic:</b></color>  ";
    str << "<color=0xffffffff>EVE-Online related help.</color><br><br>";
    str << "<color=0xff00ff00><b>Rules:</b></color>  ";
    str << "<color=0xffffffff>No WTB, WTS, WTT, PC, advertising, recruiting, scamming, offering private help in any form or begging in this channel.<br>";
    str << "No CAPS or text-decoration.</color><br><br>";
    str << "<color=0xff00ff00><b>Language:</b></color>  ";
    str << "<color=0xffffffff>This channel is ENGLISH ONLY.</color><br><br>";
    str << "<color=0xff00ff00><b>How to contact a GM:</b> </color>  ";
    str << "<color=0xffffffff>Right now, our petition service isnt operational, so to contact a GM, post on the forums.</color><br><br>";
    //str << "<color=0xffffffff>File a petition (F12 - Petitions - New Petition)</color><br><br>";
    str << "<color=0xff00ff00><b>ISK Advertising:</b></color>  ";
    str << "<color=0xffff0000>Contrary to what they spam, EVEmu does not authorize any person or site to sell ISK for RL cash.</color><br><br>";
    CreateChannel(2, 1, "Help", str.str().c_str(), nullptr, "help", LSC::Type::normal, cspa, 263238, -1/*263262*/, true);
//Empires/Factions
    CreateChannel(10, 1, "Caldari", "Caldari Faction", nullptr, "caldari", LSC::Type::normal, cspa, 263329, -1/*263268*/, true);
    CreateChannel(11, 1, "Amarr", "Amarr Faction", nullptr, "amarr", LSC::Type::normal, cspa, 263329, -1/*263246*/, true);
    CreateChannel(12, 1, "Minmatar", "Minmatar Faction", nullptr, "minmatar", LSC::Type::normal, cspa, 263329, -1/*263281*/, true);
    CreateChannel(13, 1, "Gallente", "Gallente Faction", nullptr, "gallente", LSC::Type::normal, cspa, 263329, -1/*263271*/, true);
    CreateChannel(14, 1, "Jove", "Jove Faction", nullptr, "jove", LSC::Type::normal, cspa, 263329, -1/*263258*/, true);
// Incursion recruitment MOTD...
    str.str("");
    str << "<br><color=0xffffffff>Welcome to the recruitment channel.<br><br>";
    str << "This channel is intended for those players looking to find a new corporation, as well as those looking to enlist new players.<br>";
    str << "Other activities, such as non-recruitment discussion and scamming, are not permitted in this channel.";
//Corporate
    CreateChannel(20, 9, "Recruitment", str.str().c_str(), nullptr, "recruitment", LSC::Type::normal, cspa, 263235, -1/*263286*/, true);
    str.str("");
    str << "<br><color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Corporate CEO</color><color=0xffffffff> channel.<br><br>";
    str << "This channel is intended for corp CEOs to discuss business as they see fit.</color>";
    CreateChannel(21, 9, "CEO", str.str().c_str(), nullptr, "ceo", LSC::Type::normal, cspa, 263235, -1/*263287*/, true);
//Alliance
    /*
    str.str("");
    str << "<color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Alliance</color><color=0xffffffff>  channel.  This channel is intended for corp CEOs to discuss business as they see fit.</color>";
    CreateChannel(31, 1, "Smacktalk", str.str().c_str(), nullptr, "smacktalk", LSC::Type::normal, cspa, 263330, -1, true);
    CreateChannel(32, 1, "Rumour Mill", str.str().c_str(), nullptr, "rumourmill", LSC::Type::normal, cspa, 263330, -1, true);
    CreateChannel(33, 1, "Freelancer", str.str().c_str(), nullptr, "freelancer", LSC::Type::normal, cspa, 263330, -1, true);
    */
//Trade
    str.str("");
    str << "<br><color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Trade</color><color=0xffffffff> channel.<br><br>";
    str << "This channel is intended for those players looking to trade the various items as referenced in the channel title.</color>";
    CreateChannel(40, 1, "Other", str.str().c_str(), nullptr, "other", LSC::Type::normal, cspa, 263240, -1/*263277*/, true);
    CreateChannel(41, 1, "Ships", str.str().c_str(), nullptr, "ships", LSC::Type::normal, cspa, 263240, -1/*263245*/, true);
    CreateChannel(42, 1, "Blueprints", str.str().c_str(), nullptr, "blueprints", LSC::Type::normal, cspa, 263240, -1/*263292*/, true);
    CreateChannel(43, 1, "Modules and Munitions", str.str().c_str(), nullptr, "modulesandmunitions", LSC::Type::normal, cspa, 263240, -1/*263254*/, true);
    CreateChannel(44, 1, "Minerals and Manufacturing", str.str().c_str(), nullptr, "mineralsandmanufacturing", LSC::Type::normal, cspa, 263240, -1/*263275*/, true);
//Science and Industry
    str.str("");
    str << "<br><color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Science and Industry</color><color=0xffffffff> channel.<br><br>";
    str << "This channel is intended for those players looking to discuss the various items as referenced in the channel title.</color>";
    CreateChannel(50, 1, "Boosters", str.str().c_str(), nullptr, "boosters", LSC::Type::normal, cspa, 263331, -1/*263365*/, true);
    CreateChannel(51, 1, "Invention", str.str().c_str(), nullptr, "invention", LSC::Type::normal, cspa, 263331, -1/*263366*/, true);
    CreateChannel(52, 1, "Manufacturing", str.str().c_str(), nullptr, "manufacturing", LSC::Type::normal, cspa, 263331, -1/*263367*/, true);
    CreateChannel(53, 1, "Mining", str.str().c_str(), nullptr, "mining", LSC::Type::normal, cspa, 263331, -1/*263368*/, true);
    CreateChannel(54, 1, "Planetary Interaction", str.str().c_str(), nullptr, "planetaryinteraction", LSC::Type::normal, cspa, 263331, -1/*263369*/, true);
    CreateChannel(55, 1, "Research", str.str().c_str(), nullptr, "research", LSC::Type::normal, cspa, 263331, -1/*263370*/, true);
//Groups/Content
    str.str("");
    str << "<br><color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Content</color><color=0xffffffff> channel.<br><br>";
    str << "This channel is intended for those players looking to discuss the various items as referenced in the channel title.</color>";
    CreateChannel(60, 1, "Incursions", str.str().c_str(), nullptr, "incursions", LSC::Type::normal, cspa, 263328, -1/*263289*/, true);
    CreateChannel(61, 1, "Ratting", str.str().c_str(), nullptr, "ratting", LSC::Type::normal, cspa, 263328, -1/*263338*/, true);
    CreateChannel(62, 1, "Scanning", str.str().c_str(), nullptr, "scanning", LSC::Type::normal, cspa, 263328, -1/*263339*/, true);
    CreateChannel(63, 1, "Wormholes", str.str().c_str(), nullptr, "wormholes", LSC::Type::normal, cspa, 263328, -1/*263340*/, true);
//Misc
    str.str("");
    str << "<br><color=0xffffffff>Welcome to the <url=https://evemu.dev>Free Wrecks</url> channel.<br>";
    str << "Here you can offer your abandoned mission wrecks to any willing freelance salvager in New Eden.</color><br><br>";
    str << "<b><u><color=0xff00ffff>Salvagers</color></u><br><color=0xff00ff00>Alphas</color></b>";
    str << "<color=0xffffffff> - Basic salvage fits by race: </color><br>";
    str << "<url=fitting:16236:31083;2:25861;4:8135;1:31370;1:1319;2:4435;1:5973;1:24348;4::>Amarr</url><color=0xffffffff> - </color>";
    str << "<url=fitting:16238:25861;4:1319;2:31370;1:31083;2:4435;2:5973;1:24348;4::>Caldari</url><color=0xffffffff> - </color>";
    str << "<url=fitting:16240:31083;2:25861;4:8135;1:31370;1:1319;2:4435;1:5973;1:24348;4::>Gallente</url><color=0xffffffff> - </color>";
    str << "<url=fitting:16242:25861;4:1319;2:31370;1:31083;2:6001;1:4435;2:24348;4::>Minmatar</url><color=0xffffffff> </color><br><br>";
    /*  this suggests t2 shit which isnt available yet
    str << "<b><color=0xff00ff00>Omegas</color></b><color=0xffffffff> - You should be able to use a </color><url=showinfo:2998>Noctis</url>";
    str << "<color=0xffffffff>, </color><url=showinfo:30836>Salvager II's</url><color=0xffffffff> &amp; </color><url=showinfo:4250>Tractor Beam II's</url><br>";
    */
    str << "<b><u><color=0xff00ffff>Mission Runners</color></b></u><br><color=0xffffffff>";
    str << "Please state where your bookmark will be traded, contracted or if you're hoping to fleet with the salvager looking for work. <br>";
    str << "If you are contracting or trading bookmarks dont forget to</color> <color=0xff007fff>Abandon all wrecks and containers</color><color=0xffffffff>.</color><br><br>";
    str << "<b><u><color=0xff00ff00>Useful Links.</color></u><br>";
    str << "<loc><url=http://evemaps.dotlan.net/>DOTLAN</url></loc></b><color=0xffffffff> - A database of everything you need to know about New Eden; maps, corporations, navigations and much more.</color><color=0xff0000ff>NOTE:</color><color=0xffffffff> While this site is specific for Tranquility, the maps and navigation are the same here on EVEmu.</color><br>";
    str << "<loc><url=http://o.smium.org>Osmium</url></loc><color=0xffffffff> - A site where pilots post their ship fittings to help players get the most out of their ship class.</color><br>";
    str << "<loc><url=http://eve-survival.org/wikka.php?wakka=MissionReports>EVE Survival</url></loc><color=0xffffffff> - A database of missions within New Eden. Here you can find information about gaining the upper hand on those sneaky NPCs and how to perfectly run the mission in question.</color><br>";
    str << "<loc><url=http://www.fuzzwork.co.uk/>Fuzz Work</url></loc><color=0xffffffff> - A brilliant site that has many awesome calculators for LP stores, Blueprints, Invention, Ore and much more!</color><br>";
    CreateChannel(100, 2, "Free Wrecks", str.str().c_str(), nullptr, "freewrecks", LSC::Type::normal, cspa, 0, 0, true); //256739 <-- this was messageID from error about "channel already joined"
    //CreateChannel(101, 1, "", "motd", nullptr, "*title*", LSC::Type::normal, cspa, 0, 0, true);

//GM Command channel
    str.str("");
    str << "<br><color=0xffffffff>Welcome to the</color> <color=0xff00ffff>GM Command</color><color=0xffffffff> channel.<br><br>";
    str << "This channel is intended for using dot commands.</color>";
    CreateChannel(2900000000, 1, "Command", str.str().c_str(), nullptr, "command", LSC::Type::custom, cspa, 0, 0, true);
}

void LSCService::SendServerMOTD(Client* pClient)
{
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(pClient->GetSystemID());
    if (itr != m_channels.end())
        itr->second->SendServerMOTD(pClient);
}

///////////////////////////////////////////////////////////////////////////////
//
// EveMail calls:
//
///////////////////////////////////////////////////////////////////////////////

PyResult LSCService::Handle_GetMyMessages(PyCallArgs &call) {
    return(m_db->GetMailHeaders(call.client->GetCharacterID()));
}


PyResult LSCService::Handle_GetMessageDetails(PyCallArgs &call) {
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    //TODO: verify ability to read this message...

    return(m_db->GetMailDetails(args.arg2, args.arg1));
}


PyResult LSCService::Handle_Page(PyCallArgs &call) {
    Call_Page args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    _log(SERVICE__MESSAGE, "%s: Received evemail msg with subject '%s': %s", call.client->GetName(), args.subject.c_str(), args.body.c_str());

    SendMail(call.client->GetCharacterID(), args.recipients, args.subject, args.body);

    return nullptr;
}


//stuck here to be close to related functionality
void LSCService::SendMail(uint32 sender, const std::vector<int32> &recipients, const std::string &subject, const std::string &content) {
    NotifyOnMessage notify;
    std::set<uint32> successful_recipients;

    notify.subject = subject;
    notify.sentTime = Win32TimeNow();
    notify.senderID = sender;

    // there's attachmentID and messageID... does this means a single message can contain multiple attachments?
    // eg. text/plain and text/html? we should be watching for this at reading mails...
    // created should be creation time. But Win32TimeNow returns int64, and is stored as bigint(20),
    // so change in the db is needed
    std::vector<int32>::const_iterator cur, end;
    cur = recipients.begin();
    end = recipients.end();

    for(; cur != end; cur++) {
        uint32 messageID = m_db->StoreMail(sender, *cur, subject.c_str(), content.c_str(), notify.sentTime);
        if (messageID == 0) {
            _log(SERVICE__ERROR, "Failed to store message from %u for recipient %u", sender, *cur);
            continue;
        }
        //TODO: supposed to have a different messageID in each notify I suspect..
        notify.messageID = messageID;

        _log(SERVICE__MESSAGE, "Delivered message from %u to recipient %u", sender, *cur);
        //record this person in the 'delivered to' list:
        notify.recipients.push_back(*cur);
        successful_recipients.insert(*cur);
    }

    //now, send a notification to each successful recipient
    PyTuple *answer = notify.Encode();
    sEntityList.Multicast(successful_recipients, "OnMessage", "*multicastID", &answer, false);
}


//stuck here to be close to related functionality
//theres a lot of duplicated crap in here...
//this could be replaced by the SendNewEveMail if it weren't in the Client
void Client::SelfEveMail(const char* subject, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);

    va_end(args);

    m_services.lsc_service->SendMail(GetCharacterID(), GetCharacterID(), subject, str);
    SafeFree(str);
}


PyResult LSCService::Handle_MarkMessagesRead(PyCallArgs &call) {
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    std::vector<int32>::iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();
    for(; cur != end; cur++) {
        m_db->MarkMessageRead(*cur);
    }
    return nullptr;
}


PyResult LSCService::Handle_DeleteMessages(PyCallArgs &call) {
    Call_DeleteMessages args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (args.channelID != (int32)call.client->GetCharacterID()) {
        _log(SERVICE__ERROR, "%s (%d) tried to delete messages in channel %u. Denied.", call.client->GetName(), call.client->GetCharacterID(), args.channelID);
        return nullptr;
    }

    std::vector<int32>::iterator cur, end;
    cur = args.messages.begin();
    end = args.messages.end();
    for(; cur != end; cur++) {
        m_db->DeleteMessage(*cur, args.channelID);
    }

    return nullptr;
}

PyResult LSCService::ExecuteCommand(Client *from, const char *msg) {
    return(m_commandDispatch->Execute(from, msg));
}

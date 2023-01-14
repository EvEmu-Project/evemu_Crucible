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
    Author:        caytchen
*/

#ifndef __MAILDB_H_INCL__
#define __MAILDB_H_INCL__

#include "ServiceDB.h"

class PyObject;
class PyString;
class Call_CreateLabel;
class Call_EditLabel;

class MailDB : public ServiceDB
{
public:
    PyRep* GetLabels(int characterID) const;

    PyString* GetMailBody(int id) const;

    void SetMailUnread(int id);
    void SetMailRead(int id);
    void SetMailsUnread(std::vector<int32> ids);
    void SetMailsRead(std::vector<int32> ids);
    void MarkAllAsRead(uint32 characterID);
    void MarkAllAsUnread(uint32 characterID);

    bool CreateLabel(int characterID, const std::string& name, int32 color, uint32& newID) const;
    void DeleteLabel(int characterID, int labelID) const;
    void EditLabel(int characterID, int labelID, const std::string& name, int32 color) const;
    void ApplyLabel(int32 messageID, int labelID);
    void ApplyLabels(std::vector<int32> messageIDs, int labelID);

    void MarkAllAsReadByLabel(uint32 characterID, int labelID);
    void MarkAllAsUnreadByLabel(uint32 characterID, int labelID);
    void RemoveLabels(std::vector<int32> messageIDs, int labelID);
    
    void DeleteMail(int32 messageID);

    void EmptyTrash(uint32 characterID);
    void MoveAllFromTrash(uint32 characterID);
    void MoveAllToTrash(uint32 characterID);
    void MoveFromTrash(int32 messageID);
    void MoveToTrash(int32 messageID);
    void MoveToTrashByLabel(int32 characterID, int32 labelID); 

    // Mailing list

    PyDict *GetJoinedMailingLists(uint32 characterID);
    uint32 CreateMailingList(uint32 creator, std::string name, int32 defaultAccess,
                           int32 defaultMemberAccess, int32 cost); 

    void JoinMailingList(uint32 characterID, std::string name);
    void LeaveMailingList(uint32 characterID, int32 listID);
    void DeleteMailingList(uint32 characterID, int32 listID);
    // @TODO(groove): KickMembers Figure out converting PyList member info into something usable..
    PyDict *GetMailingListMembers(int32 listID);
    void MailingListSetEntityAccess(int32 entity, int32 access, int32 listID);
    void MailingListClearEntityAccess(int32 entity, int32 listID);
    // @TODO(groove): SetMailingListMembersMuted
    // @TODO(groove): SetMailingListMembersOperator
    // @TODO(groove): SetMailingListMembersClear
    // @TODO(groove): SetMailingListDefaultAccesss
    void SetMailingListDefaultAccess(int32 listID, int32 defaultAccess,
                                     int32 defaultMemberAccess, int32 cost);
    //              : Make generic method for this shit^^^
    //
    // @TODO(groove): MailingListGetInfo
    PyObject *MailingListGetSettings(int32 listID);
    // @TODO(groove): MailingListGetWelcomeMail
    // @TODO(groove): MailingListSaveWelcomeMail
    // @TODO(groove): MailingListSendWelcomeMail
    // @TODO(groove): MailingListClearWelcomeMail
                               
    
    

    // Helpers

    void ApplyStatusMasks(std::vector<int32> messageIDs, int mask);
    void RemoveStatusMasks(std::vector<int32> messageIDs, int mask);
    void ApplyStatusMask(int32 messageID, int mask);
    void RemoveStatusMask(int32 messageID, int mask);

    void ApplyLabelMask(int32 messageID, int mask);
    void ApplyLabelMasks(std::vector<int32> messageIDs, int mask);
    void RemoveLabelMask(int32 messageID, int mask);
    void RemoveLabelMasks(std::vector<int32> messageIDs, int mask);

    int SendMail(int sender, std::vector<int>& toCharacterIDs, int toListID, int toCorpOrAllianceID, const std::string& title, const std::string& body, int isReplyTo, int isForwardedFrom);
    PyRep* GetNewMail(int charId);
    PyRep* GetMailStatus(int charId);

protected:
    static int BitFromLabelID(int id);
};

#endif

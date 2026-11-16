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
    Author:        caytchen, groove
*/

// @TODO(groove) check characterIDs while doing moves to prevent
// client trickery
#include "eve-server.h"

#include "mail/MailDB.h"
#include "EVE_Mail.h"


PyRep* MailDB::GetMailStatus(int charId)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            " SELECT messageID, statusMask, labelMask "
                            " FROM mailStatus"
                            " WHERE characterID = %u" , charId)) {
        codelog(DATABASE__ERROR, " Failed to get mail status" );
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* MailDB::GetNewMail(int charId)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                            " SELECT m.messageID, m.senderID, m.toCharacterIDs, m.toListID, "
                            "   m.toCorpOrAllianceID, m.title, m.sentDate FROM mailStatus AS s"
                            "  LEFT JOIN mailMessage AS m USING (messageID) "
                            "  WHERE s.characterID = %u" , charId))
        return nullptr;
    return DBResultToCRowset(res);
}

int MailDB::SendMail(int sender, std::vector<int>& toCharacterIDs, int toListID, int toCorpOrAllianceID, const std::string& title, const std::string& body, int isReplyTo, int isForwardedFrom)
{
    // build a string with ',' seperated char ids
    std::string toStr;
    for (size_t i = 0; i < toCharacterIDs.size(); i++)
    {
        toStr += std::to_string(toCharacterIDs[i]);
        // only add ',' when this isn't the last ID
        if (i != (toCharacterIDs.size() - 1))
            toStr += " ," ;
    }

    // sanitize these ids
    if (toListID == -1)
        toListID = 0;
    if (toCorpOrAllianceID == -1)
        toCorpOrAllianceID = 0;

    // compress the body
    Buffer bodyCompressed;
    Buffer bodyInput(body.begin(), body.end());
    DeflateData(bodyInput, bodyCompressed);

    // ugh - buffer doesn't give us the actual buffer.. what the?
    std::string bodyCompressedStr(bodyCompressed.begin<char>(), bodyCompressed.end<char>());

    // escape it to not break the query with special characters
    std::string bodyEscaped;
    std::string titleEscaped;
    sDatabase.DoEscapeString(bodyEscaped, bodyCompressedStr);
    sDatabase.DoEscapeString(titleEscaped, title);

    // default label is 1 = Inbox
    const int defaultLabel = 1;

    // TODO(groove): Maybe do checks to see if we're allowed to send mails to
    // whereever it's going
    DBerror err;
    uint32 messageID = 0;

    if (!sDatabase.RunQueryLID(err, messageID,
                               " INSERT INTO mailMessage "
                               " (senderID, toCharacterIDs, toListID, toCorpOrAllianceID, "
                               " title, body, sentDate) "
                               " VALUES (%u, '%s', %d, %d, '%s', '%s', %"  PRIu64 " )" ,
                               sender, toStr.c_str(), toListID, toCorpOrAllianceID, titleEscaped.c_str(),
                               bodyEscaped.c_str(), Win32TimeNow()))
    {
        codelog(DATABASE__ERROR, "Failed to insert mailMessage: %s", err.c_str());
        return 0; // never create mailStatus rows with an invalid ID
    }

    if (toCharacterIDs.size() > 0) {
        for (int i = 0; i < toCharacterIDs.size(); i++) {
            uint32 id = toCharacterIDs[i];
            if (!sDatabase.RunQuery(err,
                                    " INSERT INTO mailStatus "
                                    " (messageID, characterID, statusMask, labelMask)"
                                    " VALUES (%u, %u, %u, %u)" , messageID, id, 0, mailLabelInbox))
            {
                codelog(DATABASE__ERROR, " Failed to insert mailStatus for character ids" );
                return 0;
            }
        }
    }

    if (toListID > 0) {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res,
                                " SELECT characterID FROM mailListUsers "
                                " WHERE listID = %u" , toListID))
        {
            codelog(DATABASE__ERROR, " Failed to get mailing list members" );
            return 0;
        }
        DBResultRow row;
        while (res.GetRow(row))
        {
            uint32 id = row.GetInt(0);
            if (!sDatabase.RunQuery(err,
                                    " INSERT INTO mailStatus "
                                    " (messageID, characterID, statusMask, labelMask)"
                                    " VALUES (%u, %u, %u, %u)" , messageID, id, 0, mailLabelInbox))
            {
                codelog(DATABASE__ERROR, " Failed to insert mailStatus for character ids" );
                return 0;
            }
        }
    }
    // TODO(groove) corp/alliance mails

    return messageID;
}

bool MailDB::IsMailRecipient(int32 messageID, uint32 characterID) const
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT messageID FROM mailStatus WHERE messageID = %u AND characterID = %u LIMIT 1",
        messageID, characterID))
        return false;
    return res.GetRowCount() > 0;
}

PyString* MailDB::GetMailBody(int id) const
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, " SELECT body FROM mailMessage WHERE messageID = %u" , id))
        return nullptr;
    if (res.GetRowCount() <= 0)
        return nullptr;

    DBResultRow row;
    if (!res.GetRow(row) || row.IsNull(0))
        return nullptr;

    return new PyString(row.GetText(0), row.ColumnLength(0));
}

void MailDB::SetMailUnread(int id)
{
    RemoveStatusMask(id, mailStatusMaskRead);
}

void MailDB::SetMailRead(int id)
{
    ApplyStatusMask(id, mailStatusMaskRead);
}

void MailDB::SetMailsUnread(std::vector<int32> ids, uint32 characterID)
{
    RemoveStatusMasks(ids, mailStatusMaskRead, characterID);
}
void MailDB::SetMailsRead(std::vector<int32> ids, uint32 characterID)
{
    ApplyStatusMasks(ids, mailStatusMaskRead, characterID);
}
void MailDB::MarkAllAsUnread(uint32 characterID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mailStatus SET statusMask = statusMask & ~%u WHERE characterID = %u", mailStatusMaskRead, characterID))
        codelog(DATABASE__ERROR, "MailDB::MarkAllAsUnread failed: %s", err.c_str());
}

void MailDB::MarkAllAsRead(uint32 characterID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mailStatus SET statusMask = statusMask | %u WHERE characterID = %u", mailStatusMaskRead, characterID))
        codelog(DATABASE__ERROR, "MailDB::MarkAllAsRead failed: %s", err.c_str());
}
void MailDB::MarkAllAsUnreadByLabel(uint32 characterID, int labelID)
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE mailStatus SET statusMask = statusMask & ~%u "
        "WHERE characterID = %u AND (labelMask & (1 << %u)) > 0",
        mailStatusMaskRead, characterID, bit))
        codelog(DATABASE__ERROR, "MailDB::MarkAllAsUnreadByLabel failed: %s", err.c_str());
}

void MailDB::MarkAllAsReadByLabel(uint32 characterID, int labelID)
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE mailStatus SET statusMask = statusMask |%u "
        "WHERE characterID = %u AND (labelMask & (1 << %u)) > 0",
        mailStatusMaskRead, characterID, bit))
        codelog(DATABASE__ERROR, "MailDB::MarkAllAsReadByLabel failed: %s", err.c_str());
}
void MailDB::RemoveLabels(std::vector<int32> messageIDs, int labelID, uint32 characterID)
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    RemoveLabelMasks(messageIDs, (1 << bit), characterID);
}
void MailDB::ApplyLabels(std::vector<int32> messageIDs, int labelID, uint32 characterID)
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    ApplyLabelMasks(messageIDs, (1 << bit), characterID);
}
PyRep* MailDB::GetLabels(int characterID) const
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, " SELECT bit, name, color FROM mailLabel WHERE ownerID = %u" , characterID))
        return nullptr;

    PyDict* ret = new PyDict();

    DBResultRow row;
    while (res.GetRow(row))
    {
        MailLabel label;
        label.id = (int)pow((float)2, row.GetInt(0));
        label.name = row.GetText(1);
        label.color = row.GetInt(2);

        ret->SetItem(new PyInt(label.id), label.Encode());
    }

    return ret;
}

bool MailDB::CreateLabel(int characterID, const std::string& name, int32 color, uint32& newID) const
{
    // we need to get the next free bit index; can't avoid a SELECT
    DBQueryResult res;
    sDatabase.RunQuery(res, " SELECT bit FROM mailLabel WHERE ownerID = %u ORDER BY bit DESC LIMIT 1" , characterID);

    // 6 is a guessed default; there are some hardcoded labels that we don't have the details on yet
    int bit = 6;

    if (res.GetRowCount() > 0)
    {
        DBResultRow row;
        res.GetRow(row);
        // we want the next one, not the current one, so +1
        bit = row.GetInt(0) + 1;
    }

    DBerror error;
    std::string safeName;
    sDatabase.DoEscapeString(safeName, name);
    if (!sDatabase.RunQuery(error, " INSERT INTO mailLabel (bit, name, color, ownerID) VALUES (%u, '%s', %u, %u)" , bit, safeName.c_str(), color, characterID))
    {
        codelog(DATABASE__ERROR, " Failed to insert new mail label into database" );
        // since this is an out parameter, make sure we assign this even in case of an error
        newID = 0;
        return false;
    }

    // the client wants the power of 2, not the bitset index
    newID = (uint32)pow((float)2, bit);
    return true;
}

void MailDB::DeleteLabel(int characterID, int labelID) const
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE mailStatus SET labelMask = labelMask & ~(1 << %u) "
        "WHERE characterID = %u", bit, characterID)) {
        codelog(DATABASE__ERROR, "MailDB::DeleteLabel failed to remove mask: %s", err.c_str());
        return;
    }
    if (!sDatabase.RunQuery(err,
        "DELETE FROM mailLabel WHERE ownerID = %u AND bit = %u", characterID, bit))
        codelog(DATABASE__ERROR, "MailDB::DeleteLabel failed: %s", err.c_str());
}
void MailDB::EditLabel(int characterID, int labelID, const std::string& name, int32 color) const
{
    int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    std::string safeName;
    sDatabase.DoEscapeString(safeName, name);

    DBerror error;
    if (name.length() == 0) {
        sDatabase.RunQuery(error, " UPDATE mailLabel SET color = %u WHERE bit = %u AND ownerID = %u" , color, bit, characterID);
    } else if (color == -1) {
        sDatabase.RunQuery(error, " UPDATE mailLabel SET name = '%s' WHERE bit = %u AND ownerID = %u" , safeName.c_str(), bit, characterID);
    } else {
        sDatabase.RunQuery(error, " UPDATE mailLabel SET name = '%s', color = %u WHERE bit = %u AND ownerID = %u" , safeName.c_str(), color, bit, characterID);
    }
}

void MailDB::ApplyLabel(int32 messageID, int labelID)
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    ApplyLabelMask(messageID, (1 << bit));
}

void MailDB::DeleteMail(int32 messageID, uint32 characterID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "DELETE FROM mailStatus WHERE messageID = %u AND characterID = %u",
        messageID, characterID))
        codelog(DATABASE__ERROR, "MailDB::DeleteMail failed: %s", err.c_str());
}
void MailDB::EmptyTrash(uint32 characterID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM mailStatus WHERE characterID = %u AND (statusMask & %u) > 0", characterID, mailStatusMaskTrashed))
        codelog(DATABASE__ERROR, "MailDB::EmptyTrash failed: %s", err.c_str());
}
void MailDB::MoveAllFromTrash(uint32 characterID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mailStatus SET statusMask = statusMask & ~%u WHERE characterID = %u", mailStatusMaskTrashed, characterID))
        codelog(DATABASE__ERROR, "MailDB::MoveAllFromTrash failed: %s", err.c_str());
}
void MailDB::MoveAllToTrash(uint32 characterID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mailStatus SET statusMask = statusMask | %u WHERE characterID = %u", mailStatusMaskTrashed, characterID))
        codelog(DATABASE__ERROR, "MailDB::MoveAllToTrash failed: %s", err.c_str());
}
void MailDB::MoveToTrash(int32 messageID)
{
    ApplyStatusMask(messageID, mailStatusMaskTrashed);
}
void MailDB::MoveToTrashByLabel(int32 characterID, int32 labelID)
{
    const int bit = BitFromLabelID(labelID);
    if (bit < 0 || bit > 30)
        return;
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE mailStatus SET statusMask = statusMask | %u "
        "WHERE characterID = %u AND (labelMask & (1 << %u)) > 0",
        mailStatusMaskTrashed, characterID, bit))
        codelog(DATABASE__ERROR, "MailDB::MoveToTrashByLabel failed: %s", err.c_str());
}
void MailDB::ApplyStatusMasks(std::vector<int32> messageIDs, int mask, uint32 characterID)
{
    if (messageIDs.empty())
        return;
    std::ostringstream query;
    query << "UPDATE mailStatus SET statusMask = statusMask |" << mask
          << " WHERE characterID = " << characterID << " AND messageID IN (";
    for (size_t i = 0; i < messageIDs.size(); ++i) {
        if (i)
            query << ",";
        query << messageIDs[i];
    }
    query << ")";
    DBerror err;
    if (!sDatabase.RunQuery(err, "%s", query.str().c_str()))
        codelog(DATABASE__ERROR, "MailDB::ApplyStatusMasks failed: %s", err.c_str());
}
void MailDB::RemoveStatusMasks(std::vector<int32> messageIDs, int mask, uint32 characterID)
{
    if (messageIDs.empty())
        return;
    std::ostringstream query;
    query << "UPDATE mailStatus SET statusMask = statusMask & ~" << mask
          << " WHERE characterID = " << characterID << " AND messageID IN (";
    for (size_t i = 0; i < messageIDs.size(); ++i) {
        if (i)
            query << ",";
        query << messageIDs[i];
    }
    query << ")";
    DBerror err;
    if (!sDatabase.RunQuery(err, "%s", query.str().c_str()))
        codelog(DATABASE__ERROR, "MailDB::RemoveStatusMasks failed: %s", err.c_str());
}
void MailDB::ApplyStatusMask(int32 messageID, int mask)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            " UPDATE mailStatus "
                            " SET statusMask = statusMask | %u "
                            " WHERE messageID = %u" , mask, messageID)) {
        codelog(DATABASE__ERROR, " Failed to apply status mask" );
    }
}


void MailDB::RemoveStatusMask(int32 messageID, int mask)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            " UPDATE mailStatus "
                            " SET statusMask = statusMask & ~%u "
                            " WHERE messageID = %u" , mask, messageID)) {
        codelog(DATABASE__ERROR, " Failed to remove status mask" );
    }
}


void MailDB::ApplyLabelMask(int32 messageID, int mask)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            " UPDATE mailStatus "
                            " SET labelMask = (labelMask | %u) "
                            " WHERE messageID = %u;" , mask, messageID))
    {
        codelog(DATABASE__ERROR, " Failed to apply label mask to message" );
    }
}

void MailDB::RemoveLabelMask(int32 messageID, int mask)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
                            " UPDATE mailStatus "
                            " SET labelMask = (labelMask & ~%u) "
                            " WHERE messageID = %u;" , mask, messageID))
    {
        codelog(DATABASE__ERROR, " Failed to apply label mask to message" );
    }
}

void MailDB::ApplyLabelMasks(std::vector<int32> messageIDs, int mask, uint32 characterID)
{
    if (messageIDs.empty())
        return;
    std::ostringstream query;
    query << "UPDATE mailStatus SET labelMask = labelMask |" << mask
          << " WHERE characterID = " << characterID << " AND messageID IN (";
    for (size_t i = 0; i < messageIDs.size(); ++i) {
        if (i)
            query << ",";
        query << messageIDs[i];
    }
    query << ")";
    DBerror err;
    if (!sDatabase.RunQuery(err, "%s", query.str().c_str()))
        codelog(DATABASE__ERROR, "MailDB::ApplyLabelMasks failed: %s", err.c_str());
}
void MailDB::RemoveLabelMasks(std::vector<int32> messageIDs, int mask, uint32 characterID)
{
    if (messageIDs.empty())
        return;
    std::ostringstream query;
    query << "UPDATE mailStatus SET labelMask = labelMask & ~" << mask
          << " WHERE characterID = " << characterID << " AND messageID IN (";
    for (size_t i = 0; i < messageIDs.size(); ++i) {
        if (i)
            query << ",";
        query << messageIDs[i];
    }
    query << ")";
    DBerror err;
    if (!sDatabase.RunQuery(err, "%s", query.str().c_str()))
        codelog(DATABASE__ERROR, "MailDB::RemoveLabelMasks failed: %s", err.c_str());
}

int MailDB::BitFromLabelID(int id)
{
    // Label IDs are one-hot bit masks. Reject zero/negative or multi-bit
    // values so a malformed client request cannot affect the Inbox bit.
    if (id <= 0 || (id & (id - 1)) != 0)
        return -1;
    for (int i = 0; i <= 30; ++i)
        if (id == (1 << i))
            return i;
    return -1;
}

PyDict *MailDB::GetJoinedMailingLists(uint32 characterID)
{
    // @NOTE: May have to util.keyval this
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
                " SELECT l.id, l.displayName, l.defaultAccess, l.defaultMemberAccess, l.cost,"
                "   u.listID, u.characterID, u.role, u.access"
                "  FROM mailListUsers AS u "
                "   LEFT JOIN mailList AS l ON l.id = u.listID "
                "   WHERE u.characterID = %u" , characterID))
    {
        codelog(DATABASE__ERROR, " Failed to get joined mailing lists" );
        return nullptr;
    }

    DBResultRow row;
    PyDict *ret = new PyDict();
    while (res.GetRow(row))
    {
        PyDict *dict = new PyDict();

        dict->SetItemString("displayName" , new PyString(row.GetText(1)));

        int32 role = row.GetInt(7);
        switch (role)
        {
        case mailingListMemberMuted: {
            dict->SetItemString("isMuted" , new PyBool(true));
            dict->SetItemString("isOwner" , new PyBool(false));
            dict->SetItemString("isOperator" , new PyBool(false));
        } break;
        case mailingListMemberOperator: {
            dict->SetItemString("isMuted" , new PyBool(false));
            dict->SetItemString("isOwner" , new PyBool(false));
            dict->SetItemString("isOperator" , new PyBool(true));
        } break;
        case mailingListMemberOwner: {
            dict->SetItemString("isMuted" , new PyBool(false));
            dict->SetItemString("isOwner" , new PyBool(true));
            dict->SetItemString("isOperator" , new PyBool(false));
        } break;
        }

        ret->SetItem(new PyInt(row.GetInt(0)), new PyObject("util.KeyVal" , dict));
    }

    return ret;
}


uint32 MailDB::CreateMailingList(uint32 creator, std::string name, int32 defaultAccess, int32 defaultMemberAccess, int32 cost)
{
    DBerror err;
    uint32 id = 0;
    uint32& idr = id;

    std::string safeName;
    sDatabase.DoEscapeString(safeName, name);
    if (!sDatabase.RunQueryLID(err, idr,
                            " INSERT INTO mailList "
                            " (displayName, defaultAccess, defaultMemberAccess, cost) "
                            " VALUES( '%s', %u, %u, %u) " , safeName.c_str(), defaultAccess,
                               defaultMemberAccess, cost))
    {
        codelog(DATABASE__ERROR, " Failed to create mailing list" );
        return -1;
    }

    if (!sDatabase.RunQuery(err,
                            " INSERT INTO mailListUsers "
                            " (listID, characterID, role, access) "
                            " VALUES(%u, %u, %u, %u)" , id, creator, mailingListMemberOwner,
                            mailingListAllowed))
    {
        codelog(DATABASE__ERROR, " Failed to insert owner of mailing list" );
        return -1;
    }

    return id;
}

PyDict *MailDB::GetMailingListMembers(int32 listID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT listID, characterID, role, access FROM mailListUsers "
                            " WHERE listID = %u" , listID))
    {
        codelog(DATABASE__ERROR, " Failed to get mailing list members" );
        return nullptr;
    }

    DBResultRow row;
    PyDict *dict = new PyDict();

    while (res.GetRow(row))
    {
        dict->SetItem(new PyInt(row.GetInt(1)), new PyInt(row.GetInt(2)));
    }
    return dict;
}


PyObject *MailDB::MailingListGetSettings(int32 listID)
{
    PyDict *ret = new PyDict();

    // TODO(groove): Make this a join if possible

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT id, displayName, defaultAccess, defaultMemberAccess, cost FROM mailList "
                            "  WHERE id = %u" , listID))
    {
        codelog(DATABASE__ERROR, " Failed to get mailing list settings" );
        return nullptr;
    }
    DBResultRow row;

    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, " mailList didn't give us a row" );
        return nullptr;
    }


    ret->SetItemString("defaultAccess" , new PyInt(row.GetInt(2)));
    ret->SetItemString("defaultMemberAccess" , new PyInt(row.GetInt(3)));

    DBQueryResult res2;

    if (!sDatabase.RunQuery(res,
        " SELECT listID, characterID, role, access FROM mailListUsers "
                            " WHERE listID = %u" , listID))
    {
        codelog(DATABASE__ERROR, " Failed to get mailing list settings" );
        return nullptr;
    }

    PyDict *dict = new PyDict();

    ret->SetItemString("access" , dict);


    while (res.GetRow(row)) {
        dict->SetItem(new PyInt(row.GetInt(1)), new PyInt(row.GetInt(3)));
    }

    return new PyObject("util.KeyVal" , ret);
}


void MailDB::SetMailingListDefaultAccess(int32 listID, int32 defaultAccess, int32 defaultMemberAccess, int32 cost)
{
    DBerror err;

    if (!sDatabase.RunQuery(err,
                   " UPDATE mailList "
                   " SET defaultAccess = %u, defaultMemberAccess = %u, cost = %u "
                   " WHERE id = %u" , defaultAccess, defaultMemberAccess, cost, listID))
    {
        codelog(DATABASE__ERROR, " Failed to update mailing list defaults" );
        return;
    }
}

void MailDB::DeleteMailingList(uint32 characterID, int32 listID)
{
    // not used yet.
}

void MailDB::JoinMailingList(uint32 characterID, std::string name)
{
    // not used yet.
}

void MailDB::LeaveMailingList(uint32 characterID, int32 listID)
{
    // not used yet.
}

void MailDB::MailingListClearEntityAccess(int32 entity, int32 listID)
{
    // not used yet.
}

void MailDB::MailingListSetEntityAccess(int32 entity, int32 access, int32 listID)
{
    // not used yet.
}

void MailDB::MoveFromTrash(int32 messageID)
{
    // not used yet.
}

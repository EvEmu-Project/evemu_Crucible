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
    Author:        Luck, caytchen
*/

#ifndef MAIL_MGR_SERIVCE_H
#define MAIL_MGR_SERVICE_H

#include "services/Service.h"
#include "mail/MailDB.h"
#include "Client.h"

class MailDB;

class MailMgrService : public Service <MailMgrService>
{
public:
    MailMgrService();

protected:
    MailDB m_db;

    PyResult SendMail(PyCallArgs& call, PyList* toCharacterIDs, std::optional<PyInt*> listID, std::optional<PyInt*> toCorpOrAllianceID, PyWString* title, PyWString* body, PyInt* isReplyTo, PyInt* isForwardedFrom);
    PyResult PrimeOwners(PyCallArgs& call, PyList* ownerIDs);
    PyResult SyncMail(PyCallArgs& call, std::optional<PyInt*> first, std::optional<PyInt*> second);
    PyResult GetMailHeaders(PyCallArgs& call, PyList* messageIDs);
    PyResult MoveToTrash(PyCallArgs& call, PyList* messageIDs);
    PyResult MoveFromTrash(PyCallArgs& call, PyList* messageIDs);
    PyResult MarkAsUnread(PyCallArgs& call, PyList* messageIDs);
    PyResult MarkAsRead(PyCallArgs& call, PyList* messageIDs);
    PyResult MoveAllToTrash(PyCallArgs& call);
    PyResult MoveToTrashByLabel(PyCallArgs& call, PyInt* labelID);
    PyResult MoveToTrashByList(PyCallArgs& call, PyInt* listID);
    PyResult MarkAllAsUnread(PyCallArgs& call);
    PyResult MarkAsUnreadByLabel(PyCallArgs& call, PyInt* labelID);
    PyResult MarkAsUnreadByList(PyCallArgs& call, PyList* messageIDs);
    PyResult MarkAllAsRead(PyCallArgs& call);
    PyResult MarkAsReadByLabel(PyCallArgs& call, PyInt* labelID);
    PyResult MarkAsReadByList(PyCallArgs& call, PyInt* listID);
    PyResult MoveAllFromTrash(PyCallArgs& call);
    PyResult EmptyTrash(PyCallArgs& call);
    PyResult DeleteMail(PyCallArgs& call, PyList* messageIDs);
    PyResult GetBody(PyCallArgs& call, PyInt* messageID, PyInt* isUnread);
    PyResult AssignLabels(PyCallArgs& call, PyList* messageIDs, PyInt* labelID);
    PyResult RemoveLabels(PyCallArgs& call, PyList* messageIDs, PyInt* labelID);

    // implemented
    PyResult GetLabels(PyCallArgs& call);
    PyResult EditLabel(PyCallArgs& call, PyInt* labelID, PyWString* name, std::optional<PyInt*> color);
    PyResult CreateLabel(PyCallArgs& call, PyWString* name, std::optional<PyInt*> color);
    PyResult DeleteLabel(PyCallArgs& call, PyInt* labelID);
};


#endif /* MAIL_MGR_SERVICE_H */

/*{'messageKey': 'EveMailCanOnlySendToOwnMailinglists', 'dataID': 17875556, 'suppressable': False, 'bodyID': 256552, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256551, 'messageID': 3301}
 * {'messageKey': 'EveMailDiscardWelcomeMail', 'dataID': 17876727, 'suppressable': True, 'bodyID': 256987, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256986, 'messageID': 3236}
 * {'messageKey': 'EveMailNoCharFound', 'dataID': 17876940, 'suppressable': False, 'bodyID': 257064, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3172}
 * {'messageKey': 'EvemailCantFindRecipient', 'dataID': 17876069, 'suppressable': False, 'bodyID': 256740, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3451}
 * {'messageKey': 'EvemailCantSendToAllianceNoRole', 'dataID': 17876527, 'suppressable': False, 'bodyID': 256909, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3205}
 * {'messageKey': 'EvemailCantSendToNPCCorp', 'dataID': 17876524, 'suppressable': False, 'bodyID': 256908, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3204}
 * {'messageKey': 'EvemailCantSendToOtherAlliance', 'dataID': 17876530, 'suppressable': False, 'bodyID': 256910, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3206}
 * {'messageKey': 'EvemailCantSendToOtherCorp', 'dataID': 17876521, 'suppressable': False, 'bodyID': 256907, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3203}
 * {'messageKey': 'EvemailMailMarkAllRead', 'dataID': 17876757, 'suppressable': True, 'bodyID': 256999, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256998, 'messageID': 3226}
 * {'messageKey': 'EvemailMailReadLabelGroup', 'dataID': 17877035, 'suppressable': True, 'bodyID': 257099, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257098, 'messageID': 3227}
 * {'messageKey': 'EvemailMailReadMailinglistGroup', 'dataID': 17877045, 'suppressable': True, 'bodyID': 257103, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257102, 'messageID': 3228}
 * {'messageKey': 'EvemailMailRestoreAll', 'dataID': 17876732, 'suppressable': True, 'bodyID': 256989, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256988, 'messageID': 3231}
 * {'messageKey': 'EvemailMailTrashAll', 'dataID': 17876737, 'suppressable': True, 'bodyID': 256991, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256990, 'messageID': 3225}
 * {'messageKey': 'EvemailMailTrashLabelGroup', 'dataID': 17877040, 'suppressable': True, 'bodyID': 257101, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257100, 'messageID': 3229}
 * {'messageKey': 'EvemailMailTrashMailinglistGroup', 'dataID': 17877050, 'suppressable': True, 'bodyID': 257105, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257104, 'messageID': 3230}
 * {'messageKey': 'EvemailMaillDeleteAll', 'dataID': 17876762, 'suppressable': True, 'bodyID': 257001, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257000, 'messageID': 3232}
 * {'messageKey': 'EvemailMaxListOrCorp', 'dataID': 17876937, 'suppressable': False, 'bodyID': 257063, 'messageType': 'notify', 'urlAudio': 'wise:/msg_uiwarning03_play', 'urlIcon': '', 'titleID': None, 'messageID': 3110}
 * {'messageKey': 'EvemailMaxRecipients', 'dataID': 17876894, 'suppressable': False, 'bodyID': 257048, 'messageType': 'notify', 'urlAudio': 'wise:/msg_uiwarning03_play', 'urlIcon': '', 'titleID': None, 'messageID': 3109}
 * {'messageKey': 'EvemailNotificationsDeleteAll', 'dataID': 17876767, 'suppressable': True, 'bodyID': 257003, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257002, 'messageID': 3212}
 * {'messageKey': 'EvemailNotificationsDeleteGroup', 'dataID': 17876742, 'suppressable': True, 'bodyID': 256993, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256992, 'messageID': 3216}
 * {'messageKey': 'EvemailNotificationsMarkAllRead', 'dataID': 17876747, 'suppressable': True, 'bodyID': 256995, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256994, 'messageID': 3214}
 * {'messageKey': 'EvemailNotificationsMarkGroupRead', 'dataID': 17876752, 'suppressable': True, 'bodyID': 256997, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256996, 'messageID': 3215}
 * {'messageKey': 'EvemailSendingFailed', 'dataID': 17876535, 'suppressable': False, 'bodyID': 256912, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256911, 'messageID': 3208}
 */
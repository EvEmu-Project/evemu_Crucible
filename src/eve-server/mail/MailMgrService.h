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

#include "PyService.h"

class MailDB;

class MailMgrService : public PyService
{
public:
    MailMgrService(PyServiceMgr *mgr);
    virtual ~MailMgrService();

protected:
    MailDB* m_db;

    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(SendMail);
    PyCallable_DECL_CALL(PrimeOwners);
    PyCallable_DECL_CALL(SyncMail);
    PyCallable_DECL_CALL(GetMailHeaders);
    PyCallable_DECL_CALL(MoveToTrash);
    PyCallable_DECL_CALL(MoveFromTrash);
    PyCallable_DECL_CALL(MarkAsUnread);
    PyCallable_DECL_CALL(MarkAsRead);
    PyCallable_DECL_CALL(MoveAllToTrash);
    PyCallable_DECL_CALL(MoveToTrashByLabel);
    PyCallable_DECL_CALL(MoveToTrashByList);
    PyCallable_DECL_CALL(MarkAllAsUnread);
    PyCallable_DECL_CALL(MarkAsUnreadByLabel);
    PyCallable_DECL_CALL(MarkAsUnreadByList);
    PyCallable_DECL_CALL(MarkAllAsRead);
    PyCallable_DECL_CALL(MarkAsReadByLabel);
    PyCallable_DECL_CALL(MarkAsReadByList);
    PyCallable_DECL_CALL(MoveAllFromTrash);
    PyCallable_DECL_CALL(EmptyTrash);
    PyCallable_DECL_CALL(DeleteMail);
    PyCallable_DECL_CALL(GetBody);
    PyCallable_DECL_CALL(AssignLabels);
    PyCallable_DECL_CALL(RemoveLabels);

    // implemented
    PyCallable_DECL_CALL(GetLabels);
    PyCallable_DECL_CALL(EditLabel);
    PyCallable_DECL_CALL(CreateLabel);
    PyCallable_DECL_CALL(DeleteLabel);
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
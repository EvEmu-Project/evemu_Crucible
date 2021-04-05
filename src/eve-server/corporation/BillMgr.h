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
    Author:        Zhur
    Updates:    Allan
*/


#ifndef __BILLMGR_SERVICE_H_INCL__
#define __BILLMGR_SERVICE_H_INCL__

#include "PyService.h"
#include "corporation/CorporationDB.h"

class BillMgr : public PyService
{
public:
    BillMgr(PyServiceMgr *mgr);
    virtual ~BillMgr();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    CorporationDB m_db;

    PyCallable_DECL_CALL(CharPayBill);
    PyCallable_DECL_CALL(CharGetBills);
    PyCallable_DECL_CALL(GetBillTypes);
    PyCallable_DECL_CALL(CharGetBillsReceivable);
    PyCallable_DECL_CALL(PayCorporationBill);
    PyCallable_DECL_CALL(GetCorporationBills);
    PyCallable_DECL_CALL(GetCorporationBillsReceivable);
    PyCallable_DECL_CALL(SendAutomaticPaySettings);
    PyCallable_DECL_CALL(GetAutomaticPaySettings);
};

#endif


/*
 * BillFactoryOrOffice
 *
 */

/*{'FullPath': u'UI/Messages', 'messageID': 260456, 'label': u'BillMgrRentalBillMessageBody'}(u'This is an automatic message. A bill of {amount} ISK, due {dueDate} owed by you to {creditorsName} was issued {currentDate}. This bill is for extending the lease on your {itemType} at {locationName}.', None, {u'{amount}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'amount'}, u'{itemType}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'itemType'}, u'{locationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'locationName'}, u'{dueDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'dueDate'}, u'{currentDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentDate'}, u'{creditorsName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'creditorsName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 260457, 'label': u'BillMgrWarBillMessageBody'}(u'This is an automatic message. A bill of {amount} ISK, due {dueDate} owed by you to {creditorsName} was issued {currentDate}. This bill is for continuing the war against {against}.', None, {u'{amount}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'amount'}, u'{dueDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'dueDate'}, u'{currentDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentDate'}, u'{creditorsName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'creditorsName'}, u'{against}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'against'}})
 * {'FullPath': u'UI/Messages', 'messageID': 260458, 'label': u'BillMgrMarketFineMessageBody'}(u'This is an automatic message. A bill of {amount} ISK, due {dueDate} owed by you to {creditorsName} was issued {currentDate}. This fine is for attempting a purchase that you could not afford. Namely {itemType} at {locationName}.', None, {u'{amount}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'amount'}, u'{itemType}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'itemType'}, u'{locationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'locationName'}, u'{dueDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'dueDate'}, u'{currentDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentDate'}, u'{creditorsName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'creditorsName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 260459, 'label': u'BillMgrAllianceMaintainanceBillMessageBody'}(u'This is an automatic message. A bill of {amount} ISK, due {dueDate} owed by {allianceName} to {creditorsName} was issued {currentDate}. This bill is for the maintenance of {allianceName}.', None, {u'{amount}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'amount'}, u'{dueDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'dueDate'}, u'{currentDate}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentDate'}, u'{creditorsName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'creditorsName'}, u'{allianceName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'allianceName'}})
 *
 *
 */
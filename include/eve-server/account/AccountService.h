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


#ifndef __ACCOUNTSERVICE_H_INCL__
#define __ACCOUNTSERVICE_H_INCL__

#include "account/AccountDB.h"
#include "PyService.h"

class AccountService
: public PyService {
public:
	AccountService(PyServiceMgr *mgr);
	virtual ~AccountService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	AccountDB m_db;
	
	PyCallable_DECL_CALL(GetCashBalance)
	PyCallable_DECL_CALL(GetEntryTypes)
	PyCallable_DECL_CALL(GetKeyMap)
	PyCallable_DECL_CALL(GiveCash)
	PyCallable_DECL_CALL(GiveCashFromCorpAccount)
	PyCallable_DECL_CALL(GetJournal)

	PyTuple * GiveCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID);
	PyTuple * GiveCashToCorp(Client * const client, uint32 corpID, double amount, const char *reason, JournalRefType refTypeID);
	PyTuple * WithdrawCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID);
};

#endif

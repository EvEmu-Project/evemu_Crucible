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


#ifndef __ACCOUNTSERVICE_H_INCL__
#define __ACCOUNTSERVICE_H_INCL__

#include "../PyService.h"

#include "AccountDB.h"

class AccountService
: public PyService {
public:
	AccountService(PyServiceMgr *mgr, DBcore *db);
	virtual ~AccountService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	AccountDB m_db;
	
	PyCallable_DECL_CALL(GetCashBalance)
	PyCallable_DECL_CALL(GetRefTypes)
	PyCallable_DECL_CALL(GetKeyMap)
	PyCallable_DECL_CALL(GiveCash)//mandela
	PyCallable_DECL_CALL(GiveCashFromCorpAccount)
	PyCallable_DECL_CALL(GetJournal)//mandela

	PyRepTuple * GiveCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID);
	PyRepTuple * GiveCashToCorp(Client * const client, uint32 corpID, double amount, const char *reason, JournalRefType refTypeID);
	PyRepTuple * WithdrawCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID);
};

#endif




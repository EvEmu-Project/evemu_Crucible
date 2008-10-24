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


#ifndef __TUTORIALSVC_SERVICE_H_INCL__
#define __TUTORIALSVC_SERVICE_H_INCL__

#include "TutorialDB.h"
#include "../PyService.h"

class TutorialService : public PyService {
public:
	TutorialService(PyServiceMgr *mgr, DBcore *db);
	virtual ~TutorialService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	TutorialDB m_db;

	PyCallable_DECL_CALL(GetTutorialInfo)
	PyCallable_DECL_CALL(GetTutorials)
	PyCallable_DECL_CALL(GetCriterias)
	//LogAborted(tutnum=5, int=1, int=11)
	PyCallable_DECL_CALL(GetCategories)
	PyCallable_DECL_CALL(GetContextHelp)
	PyCallable_DECL_CALL(GetCharacterTutorialState)
};





#endif



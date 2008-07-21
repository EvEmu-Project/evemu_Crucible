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


#include "TutorialService.h"
#include "../common/logsys.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../Client.h"
#include "../PyServiceCD.h"
#include "../PyServiceMgr.h"

#include "../packets/Tutorial.h"

PyCallable_Make_InnerDispatcher(TutorialService)

TutorialService::TutorialService(PyServiceMgr *mgr, DBcore *dbc)
: PyService(mgr, "tutorialSvc"),
  m_dispatch(new Dispatcher(this)),
  m_db(dbc)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(TutorialService, GetTutorialInfo)
	PyCallable_REG_CALL(TutorialService, GetTutorials)
	PyCallable_REG_CALL(TutorialService, GetCriterias)
}

TutorialService::~TutorialService() {
	delete m_dispatch;
}


PyCallResult TutorialService::Handle_GetTutorialInfo(PyCallArgs &call) {
	PyRep *result = NULL;
	Call_GetTutorialInfo args;
	
	if(!args.Decode(&call.tuple))
	{
		codelog(CLIENT__ERROR, "Can't parse args.");
		return(NULL);
	}

	PyRepObject *keyval = new PyRepObject();
	result = keyval;

	keyval->type = "util.KeyVal";
	PyRepDict *keyval_args = new PyRepDict();
	keyval->arguments = keyval_args;


	//pagecriterias item
	{
		PyRepObject *rowset = m_db.GetPageCriterias(args.tutorialID);
		if(rowset == NULL)
		{
			codelog(CLIENT__ERROR, "An error occured while getting pagecriterias.");
			return(NULL);
		}
		keyval_args->add("pagecriterias", rowset);
	}

	//pages item
	{
		PyRepObject *rowset = m_db.GetPages(args.tutorialID);
		if(rowset == NULL)
		{
			codelog(CLIENT__ERROR, "An error occured while getting pages.");
			return(NULL);
		}
		keyval_args->add("pages", rowset);
	}


	//tutorial item
	{
		PyRepObject *rowset = m_db.GetTutorial(args.tutorialID);
		if(rowset == NULL)
		{
			codelog(CLIENT__ERROR, "An error occured while getting tutorial.");
			return(NULL);
		}
		keyval_args->add("tutorial", rowset);
	}

	//criterias item
	{
		PyRepObject *rowset = m_db.GetTutorialCriterias(args.tutorialID);
		if(rowset == NULL)
		{
			codelog(CLIENT__ERROR, "An error occured while getting tutorial criterias.");
			return(NULL);
		}
		keyval_args->add("criterias", rowset);
	}

	return(result);
}


PyCallResult TutorialService::Handle_GetTutorials(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepObject *rowset = m_db.GetAllTutorials();
	if(rowset == NULL)
	{
		codelog(CLIENT__ERROR, "An error occured while getting all tutorials.");
		return(NULL);
	}
	result = rowset;

	return(result);
}


PyCallResult TutorialService::Handle_GetCriterias(PyCallArgs &call) {
	PyRep *result = NULL;

	PyRepObject *rowset = m_db.GetAllCriterias();
	if(rowset == NULL)
	{
		codelog(CLIENT__ERROR, "An error occured while getting criterias.");
		return(NULL);
	}
	result = rowset;

	return(result);
}


























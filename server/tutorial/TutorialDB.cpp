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


#include "TutorialDB.h"
#include "../common/dbcore.h"
#include "../common/logsys.h"
#include "../common/EVEDBUtils.h"
#include "../common/PyRep.h"


TutorialDB::TutorialDB(DBcore *db) 
: ServiceDB(db)
{
}

TutorialDB::~TutorialDB()
{
}

PyRepObject *TutorialDB::GetPageCriterias(uint32 tutorialID)
{
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT tutorial_page_criteria.pageID, criteriaID"
		" FROM tutorial_page_criteria, tutorial_pages"
		" WHERE tutorial_page_criteria.pageID=tutorial_pages.pageID"
		" AND tutorial_pages.tutorialID=%lu", tutorialID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRepObject *TutorialDB::GetPages(uint32 tutorialID)
{
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT pageID, pageNumber, pageName, text, imagePath, audioPath" 
		" FROM tutorial_pages"
		" WHERE tutorialID=%lu"
		" ORDER BY pageNumber", tutorialID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRepObject *TutorialDB::GetTutorial(uint32 tutorialID)
{
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT tutorialID, tutorialVName, nextTutorialID" 
		" FROM tutorials"
		" WHERE tutorialID=%lu", tutorialID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRepObject *TutorialDB::GetTutorialCriterias(uint32 tutorialID)
{
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT criteriaID" 
		" FROM tutorials_criterias"
		" WHERE tutorialID=%lu", tutorialID))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRepObject *TutorialDB::GetAllTutorials()
{
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT tutorialID, tutorialVName, nextTutorialID" 
		" FROM tutorials"))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRepObject *TutorialDB::GetAllCriterias()
{
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT criteriaID, criteriaName, messageText, audioPath" 
		" FROM tutorial_criteria"))
	{
		codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}


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

#include "EvemuPCH.h"

TutorialDB::TutorialDB(DBcore *db) 
: ServiceDB(db)
{
}

TutorialDB::~TutorialDB()
{
}

PyRep *TutorialDB::GetPageCriterias(uint32 tutorialID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT pageID, criteriaID"
		" FROM tutorial_pages"
		" JOIN tutorial_page_criteria USING (pageID)"
		" WHERE tutorialID=%lu", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *TutorialDB::GetPages(uint32 tutorialID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT pageID, pageNumber, pageName, text, imagePath, audioPath, 0 AS dataID" 
		" FROM tutorial_pages"
		" WHERE tutorialID=%lu"
		" ORDER BY pageNumber", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *TutorialDB::GetTutorial(uint32 tutorialID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT tutorialID, tutorialName, nextTutorialID, 0 AS dataID" 
		" FROM tutorials"
		" WHERE tutorialID=%lu", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *TutorialDB::GetTutorialCriterias(uint32 tutorialID) {
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT criteriaID" 
		" FROM tutorials_criterias"
		" WHERE tutorialID=%lu", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *TutorialDB::GetAllTutorials() {
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT tutorialID, tutorialName, nextTutorialID, categoryID, 0 AS dataID"
		" FROM tutorials"))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToPackedRowset(res));
}

PyRep *TutorialDB::GetAllCriterias() {
	DBQueryResult res;

	if(!m_db->RunQuery(res, 
		"SELECT criteriaID, criteriaName, messageText, audioPath, 0 AS dataID" 
		" FROM tutorial_criteria"))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}

PyRep *TutorialDB::GetCategories() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
		"SELECT"
		" categoryID, categoryName, description, 0 AS dataID"
		" FROM tutorial_categories"))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return(NULL);
	}

	return(DBResultToRowset(res));
}


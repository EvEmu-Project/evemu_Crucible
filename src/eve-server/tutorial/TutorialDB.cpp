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

#include "EVEServerPCH.h"

PyRep *TutorialDB::GetPageCriterias(uint32 tutorialID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT pageID, criteriaID"
		" FROM tutorial_pages"
		" JOIN tutorial_page_criteria USING (pageID)"
		" WHERE tutorialID=%u", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *TutorialDB::GetPages(uint32 tutorialID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT pageID, pageNumber, pageName, text, imagePath, audioPath, 0 AS dataID" 
		" FROM tutorial_pages"
		" WHERE tutorialID=%u"
		" ORDER BY pageNumber", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *TutorialDB::GetTutorial(uint32 tutorialID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res, 
		"SELECT tutorialID, tutorialName, nextTutorialID, 0 AS dataID" 
		" FROM tutorials"
		" WHERE tutorialID=%u", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *TutorialDB::GetTutorialCriterias(uint32 tutorialID) {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res, 
		"SELECT criteriaID" 
		" FROM tutorials_criterias"
		" WHERE tutorialID=%u", tutorialID))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *TutorialDB::GetAllTutorials() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res, 
		"SELECT tutorialID, tutorialName, nextTutorialID, categoryID, 0 AS dataID"
		" FROM tutorials"))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return(DBResultToCRowset(res));
}

PyRep *TutorialDB::GetAllCriterias() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res, 
		"SELECT criteriaID, criteriaName, messageText, audioPath, 0 AS dataID" 
		" FROM tutorial_criteria"))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}

PyRep *TutorialDB::GetCategories() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" categoryID, categoryName, description, 0 AS dataID"
		" FROM tutorial_categories"))
	{
		_log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
		return NULL;
	}

	return DBResultToRowset(res);
}


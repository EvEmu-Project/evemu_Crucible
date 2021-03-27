/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "account/TutorialDB.h"

PyRep *TutorialDB::GetPageCriterias(uint32 tutorialID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT pageID, criteriaID"
        " FROM tutorial_pages"
        " JOIN tutorial_page_criteria USING (pageID)"
        " WHERE tutorialID=%u", tutorialID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
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
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
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
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
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
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyRep *TutorialDB::GetAllTutorials() {
    /*  this is wrong...our db is incomplete */
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT tutorialID, tutorialName, nextTutorialID, categoryID, 0 AS dataID"
        " FROM tutorials"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyRep *TutorialDB::GetAllCriterias() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT criteriaID, criteriaName, messageText, audioPath, 0 AS dataID"
        " FROM tutorial_criteria"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
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
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyRep *TutorialDB::GetTutorialsAndConnections(uint8 raceID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT tutorialID, %u AS raceID, nextTutorialID FROM tutorials", raceID);
    /*
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `tutorialID`, `tutorialName`, `categoryID`, `dataID`, `tutorialNameID` FROM `tutorialsvc_tutorials`")) {
        sLog.Error("TutorialService","GetTutorialsAndConnections query1 error: %s", res.error.c_str());
        return NULL;
    }
    PyObjectEx *tutorials = DBResultToCRowset(res);

    res.Reset();
    if(!sDatabase.RunQuery(res, "SELECT `tutorialID`, `raceID`, `nextTutorialID`, `dataID` FROM `tutorialsvc_connections`")) {
        sLog.Error("TutorialService","GetTutorialsAndConnections query2 error: %s", res.error.c_str());
        return NULL;
    }
    DBRowDescriptor
    PyObjectEx *connections = DBResultToCRowset(res);
    PyList *rtn = new PyList(2);
    rtn->SetItem(0, tutorials);
    rtn->SetItem(1, connections);
    rtn->Dump(stdout, "GTAC:   ");
    return rtn; */
    return DBResultToRowset(res);
}


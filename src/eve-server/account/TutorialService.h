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
*/


#ifndef __TUTORIALSVC_SERVICE_H_INCL__
#define __TUTORIALSVC_SERVICE_H_INCL__

#include "account/TutorialDB.h"
#include "services/Service.h"

class TutorialService : public Service <TutorialService>
{
public:
    TutorialService();

protected:
    TutorialDB m_db;

    PyResult GetTutorials(PyCallArgs& call);
    PyResult GetTutorialInfo(PyCallArgs& call, PyInt* tutorialID);
    PyResult GetTutorialAgents(PyCallArgs& call, PyList* agentIDs);
    PyResult GetCriterias(PyCallArgs& call);
    PyResult GetCategories(PyCallArgs& call);
    PyResult GetCharacterTutorialState(PyCallArgs& call);
    PyResult GetTutorialsAndConnections(PyCallArgs& call);
    PyResult GetCareerAgents(PyCallArgs& call);
};

#endif

//{'FullPath': u'UI/Messages', 'messageID': 257694, 'label': u'TutorialGoodiesNotEnoughSpaceInCargoBody'}(u'Some of the items in this tutorial were relocated to the station you were last in ({stationName}) because there was not enough space in your cargo hold.', None, {u'{stationName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'stationName'}})

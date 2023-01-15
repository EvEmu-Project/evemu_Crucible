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

#ifndef _SKILLMGR_SERVICE_H
#define _SKILLMGR_SERVICE_H

#include "character/CharacterDB.h"

#include "services/BoundService.h"
#include "services/ServiceManager.h"
#include "Client.h"

class SkillMgrService : public BindableService <SkillMgrService> {
public:
    SkillMgrService(EVEServiceManager& mgr);

protected:
    CharacterDB m_db;

    BoundDispatcher* BindObject(Client *client, PyRep* bindParameters) override;
};

class SkillMgrBound : public EVEBoundObject <SkillMgrBound>
{
public:
    SkillMgrBound(EVEServiceManager& mgr, CharacterDB &db, PyRep* bindParameters);

protected:
    CharacterDB &m_db;

    bool CanClientCall(Client* client) override;

    PyResult GetRespecInfo(PyCallArgs& call);
    PyResult GetSkillQueueAndFreePoints(PyCallArgs& call);
    PyResult GetEndOfTraining(PyCallArgs& call);
    PyResult GetSkillHistory(PyCallArgs& call);
    PyResult CharStopTrainingSkill(PyCallArgs& call);
    PyResult CharStartTrainingSkill(PyCallArgs& call, PyInt* itemID, PyInt* locationID);
    PyResult AddToEndOfSkillQueue(PyCallArgs& call, PyInt* skillID, PyInt* nextLevel);
    PyResult InjectSkillIntoBrain(PyCallArgs& call, PyList* skillItemIDs, PyInt* stationID);
    PyResult SaveSkillQueue(PyCallArgs& call, PyList* skillQueue);
    PyResult CharStartTrainingSkillByTypeID(PyCallArgs& call, PyInt* skillTypeID);
    PyResult RespecCharacter(PyCallArgs& call, PyInt* charisma, PyInt* intelligence, PyInt* memory, PyInt* perception, PyInt* willpower);
    PyResult GetCharacterAttributeModifiers(PyCallArgs& call, PyInt* attr);
    PyResult CharAddImplant(PyCallArgs& call, PyInt* itemID);
    PyResult RemoveImplantFromCharacter(PyCallArgs& call, PyInt* itemID);
};

#endif


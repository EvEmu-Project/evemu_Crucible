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
    Author: Zhur
*/

#include "eve-server.h"

#include "achievementtracker/AchievementTrackerMgrService.h"

AchievementTrackerMgrService::AchievementTrackerMgrService() :
    Service("achievementTrackerMgr")
{
    this->Add("GetAchievements", &AchievementTrackerMgrService::GetAchievements);
    this->Add("GetAchievement", &AchievementTrackerMgrService::GetAchievement);
    this->Add("GetProgress", &AchievementTrackerMgrService::GetProgress);
    this->Add("GetProgressForAchievement", &AchievementTrackerMgrService::GetProgressForAchievement);
    this->Add("GetProgressForChar", &AchievementTrackerMgrService::GetProgressForChar);
    this->Add("GetProgressForCharAndAchievement", &AchievementTrackerMgrService::GetProgressForCharAndAchievement);
    this->Add("GetCompletedAchievementsAndClientEventCount", &AchievementTrackerMgrService::GetCompletedAchievementsAndClientEventCount);
}

PyResult AchievementTrackerMgrService::GetAchievements(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetAchievements called - stub");
    return new PyList();
}

PyResult AchievementTrackerMgrService::GetAchievement(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetAchievement called - stub");
    return PyStatic.NewNone();
}

PyResult AchievementTrackerMgrService::GetProgress(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetProgress called - stub");
    return new PyDict();
}

PyResult AchievementTrackerMgrService::GetProgressForAchievement(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetProgressForAchievement called - stub");
    return new PyDict();
}

PyResult AchievementTrackerMgrService::GetProgressForChar(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetProgressForChar called - stub");
    return new PyDict();
}

PyResult AchievementTrackerMgrService::GetProgressForCharAndAchievement(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetProgressForCharAndAchievement called - stub");
    return new PyDict();
}

PyResult AchievementTrackerMgrService::GetCompletedAchievementsAndClientEventCount(PyCallArgs& call)
{
    sLog.Debug("AchievementTrackerMgrService", "GetCompletedAchievementsAndClientEventCount called - returning empty dicts");
    
    PyDict* result = new PyDict();
    result->SetItemString("completedDict", new PyDict());
    result->SetItemString("eventDict", new PyDict());
    
    return result;
}

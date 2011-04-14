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


#ifndef __CONFIG_SERVICE_H_INCL__
#define __CONFIG_SERVICE_H_INCL__

#include "config/ConfigDB.h"
#include "PyService.h"

class ConfigService : public PyService
{
public:
	ConfigService(PyServiceMgr *mgr);
	virtual ~ConfigService();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	ConfigDB m_db;

	PyCallable_DECL_CALL(GetMultiOwnersEx)
	PyCallable_DECL_CALL(GetMultiLocationsEx)
	PyCallable_DECL_CALL(GetMultiAllianceShortNamesEx)
	PyCallable_DECL_CALL(GetMultiCorpTickerNamesEx)
	PyCallable_DECL_CALL(GetUnits)
	PyCallable_DECL_CALL(GetMapObjects)
	PyCallable_DECL_CALL(GetMap)
	PyCallable_DECL_CALL(GetMapConnections)
	PyCallable_DECL_CALL(GetMultiGraphicsEx)
	PyCallable_DECL_CALL(GetMultiInvTypesEx)
	PyCallable_DECL_CALL(GetStationSolarSystemsByOwner)
	PyCallable_DECL_CALL(GetCelestialStatistic)
};





#endif



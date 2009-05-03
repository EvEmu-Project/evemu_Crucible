/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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


#ifndef __EVEmuServerConfig_H
#define __EVEmuServerConfig_H

#include "../common/XMLParser.h"

class EVEmuServerConfig : public XMLParser {
public:
	EVEmuServerConfig();
	virtual ~EVEmuServerConfig();

	// From <server/>
	struct {
		uint16      port;
		double      startBalance;
	} server;

	// From <database/>
	struct {
		std::string host;
		std::string username;
		std::string password;
		std::string db;
		uint16      port;
	} database;

	// From <files/>
	struct {
		std::string strings;
		std::string log;
		std::string logSettings;
		std::string cacheDir;
	} files;

	virtual std::string GetByName(const std::string &var_name) const;

	void Dump() const;

protected:
	void _ParseServer(TiXmlElement *ele);
	void _ParseDatabase(TiXmlElement *ele);
	void _ParseFiles(TiXmlElement *ele);
};

// config for everyone to use
extern EVEmuServerConfig sConfig;

#endif

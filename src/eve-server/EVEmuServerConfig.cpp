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

#include "EVEServerPCH.h"

EVEmuServerConfig sConfig;

EVEmuServerConfig::EVEmuServerConfig() {
	// import the needed handler prototypes
	XMLParser::Handlers["server"] =   static_cast<XMLParser::ElementHandler>(&EVEmuServerConfig::_ParseServer);
	XMLParser::Handlers["database"] = static_cast<XMLParser::ElementHandler>(&EVEmuServerConfig::_ParseDatabase);
	XMLParser::Handlers["files"] =    static_cast<XMLParser::ElementHandler>(&EVEmuServerConfig::_ParseFiles);

	// Set sane defaults

	//server
	server.port = 26001;
	server.startBalance = 6666000000.0f;
	server.autoAccount = false; // create account at login if doesn't exist.
	server.autoAccountRole = 2; // role to assign to auto created account.

	// database
	database.host = "localhost";
	database.port = 3306;
	database.username = "eve";
	database.password = "eve";
	database.db = "eve";

	// Files
	files.strings = "../data/strings.txt";
	files.log = "logs/eveserver.log";
	files.logSettings = "log.ini";
	files.cacheDir = "";
}

EVEmuServerConfig::~EVEmuServerConfig() {
}
	
void EVEmuServerConfig::_ParseServer(TiXmlElement *ele) {
	const char *text;

	text = ParseTextBlock(ele, "port", true);
	if(text)
		server.port = atoi(text);

	text = ParseTextBlock(ele, "startBalance", true);
	if(text)
		server.startBalance = atof(text);

	text = ParseTextBlock(ele, "autoAccount", true);
	if(text)
		server.autoAccount = !strncmp(text, "true", 4);
	
	text = ParseTextBlock(ele, "autoAccountRole", true);
	if(text)
		server.autoAccountRole = atof(text);

}

void EVEmuServerConfig::_ParseDatabase(TiXmlElement *ele) {
	const char *text;

	text = ParseTextBlock(ele, "host", true);
	if(text)
		database.host = text;

	text = ParseTextBlock(ele, "port", true);
	if(text)
		database.port = atoi(text);

	text = ParseTextBlock(ele, "username", true);
	if(text)
		database.username = text;

	text = ParseTextBlock(ele, "password", true);
	if(text)
		database.password = text;

	text = ParseTextBlock(ele, "db", true);
	if(text)
		database.db = text;
}

void EVEmuServerConfig::_ParseFiles(TiXmlElement *ele) {
	const char *text;

	text = ParseTextBlock(ele, "strings", true);
	if(text)
		files.strings = text;

	text = ParseTextBlock(ele, "logfile", true);
	if(text)
		files.log = text;

	text = ParseTextBlock(ele, "logsettings", true);
	if(text)
		files.logSettings = text;

	text = ParseTextBlock(ele, "cache", true);
	if(text)
		files.cacheDir = text;
}

std::string EVEmuServerConfig::GetByName(const std::string &var_name) const {
	if(var_name == "server.port")
		return(itoa(server.port));
	if(var_name == "server.startBalance")
		return(itoa(server.startBalance));
	if(var_name == "server.autoAccount")
		return(server.autoAccount ? "true" : "false");
	if(var_name == "server.autoAccountRole")
		return(itoa(server.autoAccountRole));

	if(var_name == "database.host")
		return(database.host);
	if(var_name == "database.username")
		return(database.username);
	if(var_name == "database.password")
		return(database.password);
	if(var_name == "database.db")
		return(database.db);
	if(var_name == "database.port")
		return(itoa(database.port));

	if(var_name == "files.strings")
		return(files.strings);
	if(var_name == "files.log")
		return(files.log);
	if(var_name == "files.logSettings")
		return(files.logSettings);
	if(var_name == "files.cacheDir")
		return(files.cacheDir);

	return("");
}

void EVEmuServerConfig::Dump() const {
	_log(COMMON__MESSAGE, "server.port = %u",         server.port);
	_log(COMMON__MESSAGE, "server.startBalance = %f", server.startBalance);
	_log(COMMON__MESSAGE, "server.autoAccount = %s",	  server.autoAccount ? "true" : "false");
	_log(COMMON__MESSAGE, "server.autoAccountRole = %u", server.autoAccountRole);

	_log(COMMON__MESSAGE, "database.host = %s",       database.host.c_str());
	_log(COMMON__MESSAGE, "database.username = %s",   database.username.c_str());
	_log(COMMON__MESSAGE, "database.password = %s",   database.password.c_str());
	_log(COMMON__MESSAGE, "database.db = %s",         database.db.c_str());
	_log(COMMON__MESSAGE, "database.port = %u",       database.port);

	_log(COMMON__MESSAGE, "files.strings = %s",       files.strings.c_str());
	_log(COMMON__MESSAGE, "files.log = %s",           files.log.c_str());
	_log(COMMON__MESSAGE, "files.logSettings = %s",   files.logSettings.c_str());
	_log(COMMON__MESSAGE, "files.cacheDir = %s",      files.cacheDir.c_str());
}

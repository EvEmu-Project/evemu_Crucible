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

#include "EvemuPCH.h"

std::string EVEmuServerConfig::ConfigFile = "evemuserver.xml";
EVEmuServerConfig *EVEmuServerConfig::_config = NULL;
	
void EVEmuServerConfig::do_database(TiXmlElement *ele) {
	const char *text;

	text=ParseTextBlock(ele,"host",true);
	if (text)
		DatabaseHost=text;

	text=ParseTextBlock(ele,"port",true);
	if (text)
		DatabasePort=atoi(text);

	text=ParseTextBlock(ele,"username",true);
	if (text)
		DatabaseUsername=text;

	text=ParseTextBlock(ele,"password",true);
	if (text)
		DatabasePassword=text;

	text=ParseTextBlock(ele,"db",true);
	if (text)
		DatabaseDB=text;
}

void EVEmuServerConfig::do_server(TiXmlElement *ele) {
	const char *text;

	text=ParseTextBlock(ele,"port",true);
	if (text)
		ServerPort=atoi(text);
}

void EVEmuServerConfig::do_files(TiXmlElement *ele) {
	const char *text;

	text=ParseTextBlock(ele,"strings",true);
	if (text)
		StringsFile=text;

	text=ParseTextBlock(ele,"logsettings",true);
	if (text)
		LogSettingsFile=text;

	text=ParseTextBlock(ele,"logfile",true);
	if (text)
		LogFile=text;

	text=ParseTextBlock(ele,"cache",true);
	if (text)
		CacheDirectory=text;
}

std::string EVEmuServerConfig::GetByName(const std::string &var_name) const {
	if(var_name == "DatabaseHost")
		return(DatabaseHost);
	if(var_name == "DatabaseUsername")
		return(DatabaseUsername);
	if(var_name == "DatabasePassword")
		return(DatabasePassword);
	if(var_name == "DatabaseDB")
		return(DatabaseDB);
	if(var_name == "DatabasePort")
		return(itoa(DatabasePort));
	if(var_name == "StringsFile")
		return(StringsFile);
	if(var_name == "LogSettingsFile")
		return(LogSettingsFile);
	if(var_name == "CacheDirectory")
		return(CacheDirectory);
	return("");
}

void EVEmuServerConfig::Dump() const
{
	_log(COMMON__MESSAGE, "DatabaseHost = %s", DatabaseHost.c_str());
	_log(COMMON__MESSAGE, "DatabaseUsername = %s", DatabaseUsername.c_str());
	_log(COMMON__MESSAGE, "DatabasePassword = %s", DatabasePassword.c_str());
	_log(COMMON__MESSAGE, "DatabaseDB = %s", DatabaseDB.c_str());
	_log(COMMON__MESSAGE, "DatabasePort = %d", DatabasePort);
	_log(COMMON__MESSAGE, "StringsFile = %s", StringsFile.c_str());
	_log(COMMON__MESSAGE, "LogSettingsFile = %s", LogSettingsFile.c_str());
	_log(COMMON__MESSAGE, "CacheDirectory = %s", CacheDirectory.c_str());
}


























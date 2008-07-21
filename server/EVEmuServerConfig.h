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


#ifndef __EVEmuServerConfig_H
#define __EVEmuServerConfig_H

#include "../common/XMLParser.h"

class EVEmuServerConfig : public XMLParser {
public:
	virtual std::string GetByName(const std::string &var_name) const;

	// From <server/>
	uint16 ServerPort;
	
	// From <database/>
	std::string DatabaseHost;
	std::string DatabaseUsername;
	std::string DatabasePassword;
	std::string DatabaseDB;
	uint16 DatabasePort;

	// From <files/>
	std::string StringsFile;
	std::string LogSettingsFile;
	std::string LogFile;
	std::string CacheDirectory;
	
protected:

	static EVEmuServerConfig *_config;

	static std::string ConfigFile;

#define ELEMENT(name) \
	void do_##name(TiXmlElement *ele);
	#include "EVEmuServerConfig_elements.h"


	EVEmuServerConfig() {
		// import the needed handler prototypes
#define ELEMENT(name) \
		Handlers[#name]=(ElementHandler)&EVEmuServerConfig::do_##name;
	#include "EVEmuServerConfig_elements.h"

		// Set sane defaults

		//server
		ServerPort = 26001;
		
		// database
		DatabaseHost="localhost";
		DatabasePort=3306;
		DatabaseUsername="eve";
		DatabasePassword="eve";
		DatabaseDB="eve";

		// Files
		StringsFile="../data/strings.txt";
		LogSettingsFile="log.ini";
		LogFile = "logs/eveserver.log";
		CacheDirectory = "";
		
	}
	virtual ~EVEmuServerConfig() {}

public:

	// Produce a const singleton
	static const EVEmuServerConfig *get() {
		if (_config == NULL) 
			LoadConfig();
		return(_config);
	}

	// Allow the use to set the conf file to be used.
	static void SetConfigFile(const std::string &file) { EVEmuServerConfig::ConfigFile = file; }

	// Load the config
	static bool LoadConfig() {
		if (_config != NULL)
			delete _config;
		_config=new EVEmuServerConfig;

		return _config->ParseFile(EVEmuServerConfig::ConfigFile.c_str(),"eve");
	}

	void Dump() const;
};

#endif

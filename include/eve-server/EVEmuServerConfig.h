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

#ifndef __EVEMU_SERVER_CONFIG_H__INCL__
#define __EVEMU_SERVER_CONFIG_H__INCL__

class EVEmuServerConfig
: public XMLParser<EVEmuServerConfig>,
  public Singleton<EVEmuServerConfig>
{
public:
	EVEmuServerConfig();

	// From <server/>
	struct
    {
		uint16	port;
		double	startBalance;
		bool	autoAccount; // create account at login if doesn't exist.
		uint32	autoAccountRole; // role to assign to auto created account.
	} server;

	// From <database/>
	struct
    {
		std::string host;
		std::string username;
		std::string password;
		std::string db;
		uint16      port;
	} database;

	// From <files/>
	struct
    {
		std::string log;
		std::string logSettings;
		std::string cacheDir;
	} files;

protected:
    bool ProcessEve( const TiXmlElement* ele );
	bool ProcessServer( const TiXmlElement* ele );
	bool ProcessDatabase( const TiXmlElement* ele );
	bool ProcessFiles( const TiXmlElement* ele );

    /**
     * @brief Extracts text from given node.
     *
     * Function finds first child of given name and gives
     * it to function GetText.
     *
     * @param[in] within   The node to extract the text from.
     * @param[in] name     Name of child to extract the text from.
     * @param[in] optional If set to true, no error is printed when
     *                     an error occurs.
     *
     * @return Extracted text; NULL if an error occurred.
     */
    static const char* ParseTextBlock( const TiXmlNode* within, const char* name, bool optional = false );
    /**
     * @brief Extracts text from given node.
     *
     * Function obtains first child and if it's text element,
     * returns its content.
     *
     * @param[in] within   The node to extract text from.
     * @param[in] optional If set to true, no error is printed when
     *                     an error occurs.
     *
     * @return Extracted text; NULL if an error occurred.
     */
    static const char* GetText( const TiXmlNode* within, bool optional = false );
};

#define sConfig \
    ( EVEmuServerConfig::get() )

#endif /* !__EVEMU_SERVER_CONFIG_H__INCL__ */

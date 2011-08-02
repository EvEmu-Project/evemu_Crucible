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
    Author:     Zhur, Bloody.Rabit
*/

#ifndef __EVE_SERVER_CONFIG__H__INCL__
#define __EVE_SERVER_CONFIG__H__INCL__

/**
 * @brief Class which parses and stores eve-server configuration.
 *
 * @author Zhur, Bloody.Rabbit
 */
class EVEServerConfig
: public XMLParserEx,
  public Singleton< EVEServerConfig >
{
public:
    /**
     * @brief Primary constructor; initializes the object with default values.
     */
    EVEServerConfig();

	/// From <rates/>
	struct
	{
		/// Rate at which skills train compared to normal speed
		float skillRate;
		/// Modifier for security rating changes. Changes how fast it goes up/down based on actions
		float secRate;
		/// Modifier for npc bounties automatically awarded for shooting down npc enemies.
		float npcBountyMultiply;
	} rates;

    /// From <account/>
    struct
    {
        /// Role to assign to auto created account; set to 0 to disable auto account creation.
        uint32 autoAccountRole;
        /// A message shown to every client on login.
        std::string loginMessage;
    } account;

    /// From <character/>
    struct
    {
        /// Money balance of new created characters.
        double startBalance;
		/// Starting station ID for new characters
		uint32 startStation;
		/// Starting security rating for new characters.
		double startSecRating;
		/// Starting corp ID for new characters
		uint32 startCorporation;
    } character;

    // From <database/>
    struct
    {
        /// Hostname of database server.
        std::string host;
        /// A port at which the database server listens.
        uint16 port;
        /// Name of database account to use.
        std::string username;
        /// Password for the database account.
        std::string password;
        /// A database to be used by server.
        std::string db;
    } database;

    // From <files/>
    struct
    {
        /// A logfile to be used.
        std::string log;
        /// A log configuration file.
        std::string logSettings;
        /// A directory at which the cache files should be stored.
        std::string cacheDir;
		// used as the base directory for the image server
		std::string imageDir;
    } files;

    /// From <net/>
    struct
    {
        /// Port at which the server should listen.
        uint16 port;
        /// Port at which the imageServer should listen.
        uint16 imageServerPort;
		/// the imageServer for char images. should be the evemu server external ip/host
		std::string imageServer;
        /// Port at which the apiServer should listen.
        uint16 apiServerPort;
        /// the apiServer for API functions. should be the evemu server external ip/host
        std::string apiServer;
    } net;

protected:
    bool ProcessEveServer( const TiXmlElement* ele );
    bool ProcessRates( const TiXmlElement* ele );
    bool ProcessAccount( const TiXmlElement* ele );
    bool ProcessCharacter( const TiXmlElement* ele );
    bool ProcessDatabase( const TiXmlElement* ele );
    bool ProcessFiles( const TiXmlElement* ele );
    bool ProcessNet( const TiXmlElement* ele );
};

/// A macro for easier access to the singleton.
#define sConfig \
    ( EVEServerConfig::get() )

#endif /* !__EVE_SERVER_CONFIG__H__INCL__ */

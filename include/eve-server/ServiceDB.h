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
    Author:     Zhur
*/

#ifndef __SERVICEDB_H_INCL__
#define __SERVICEDB_H_INCL__

/*                                                                              
 * This object is the home for common DB operations which may be needed by many
 * different service objects. It should be inherited by each service DB
 * implementation.
 *
 */

class ItemFactory;

class EntityList;
class Client;
class CharacterData;
class CorpMemberInfo;

class ServiceDB
{
public:
    /**
     * DoLogin()
     *
     * This method performs checks when an account is logging into the server.
     *
     * @param[in] login
     * @param[in] pass
     * @param[out] acountID
     * @param[out] role
     * @author 
     */
    bool DoLogin( const char* login, const char* pass, uint32& accountID, uint32& role );

    //entity/item stuff:
    PyObject *GetSolRow(uint32 systemID) const;
    PyObject *GetSolDroneState(uint32 systemID) const;

    bool GetSystemInfo(uint32 systemID, uint32 *constellationID, uint32 *regionID, std::string *name, std::string *securityClass);    // mapSolarSystems
    bool GetStaticItemInfo(uint32 itemID, uint32 *systemID, uint32 *constellationID, uint32 *regionID, GPoint *position);    // mapDenormalize
    bool GetStationInfo(uint32 stationID, uint32 *systemID, uint32 *constellationID, uint32 *regionID, GPoint *position, GPoint *dockPosition, GVector *dockOrientation);    // staStations

    uint32 GetDestinationStargateID(uint32 fromSystem, uint32 toSystem);

    bool GetConstant(const char *name, uint32 &into);
    
    //these really want to move back into AccountDB
    bool GiveCash( uint32 characterID, JournalRefType refTypeID, uint32 ownerFromID, uint32 ownerToID, const char *argID1, uint32 accountID, EVEAccountKeys accountKey, double amount, double balance, const char *reason);
    double GetCorpBalance(uint32 corpID);
    bool AddBalanceToCorp(uint32 corpID, double amount);

    //johnsus - serverOnline mod
    void SetServerOnlineStatus(bool onoff_status);
    //johnsus - characterOnline mod
    void SetCharacterOnlineStatus(uint32 char_id, bool onoff_status);

    void SetAccountOnlineStatus(uint32 accountID, bool onoff_status);
	void SetAccountBanStatus(uint32 accountID, bool onoff_status);

protected:
    void ProcessStringChange(const char * key, const std::string & oldValue, const std::string & newValue, PyDict * notif, std::vector<std::string> & dbQ);
    void ProcessRealChange(const char * key, double oldValue, double newValue, PyDict * notif, std::vector<std::string> & dbQ);
    void ProcessIntChange(const char * key, uint32 oldValue, uint32 newValue, PyDict * notif, std::vector<std::string> & dbQ);

    /**
     * CreateNewAccount
     *
     * This method is part of the "autoAccount" creation patch by firefoxpdm. This
     * will insert a new account row into the database if the account name doesn't
     * exist at login.
     *
     * @param login is a const char string containing the name.
     * @param pass is a const char string containing the password.
     * @param role is the users role in the game.
     * @author firefoxpdm, xanarox
     */
    uint32 CreateNewAccount( const char* login, const char* pass, uint64 role );
};

#endif



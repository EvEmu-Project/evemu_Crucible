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
    Author:     Zhur
    Updates:     Allan
*/

#ifndef __SERVICEDB_H_INCL__
#define __SERVICEDB_H_INCL__

#include "eve-server.h"
#include "POD_containers.h"

/**
 * This object is the home for common DB operations which may be needed by many
 * different service objects. It should be inherited by each serviceDB
 * implementation.
 *
 * it's also in need of an overhaul.
 */

class ServiceDB
{
public:
    static bool GetAccountInformation(CryptoChallengePacket& ccp, AccountData& aData, std::string& failMsg);
    static bool UpdateAccountHash( const char* username, std::string &hash );
    static bool IncrementLoginCount(uint32 accountID );
    static void UpdatePassword(uint32 accountID, const char* pass);

    uint32 GetStationOwner(uint32 stationID);

    // not used cause eveConstants table doesnt exist...omitted from ccp dump
    static bool GetConstant(const char *name, uint32 &into);

    static void SetServerOnlineStatus(bool online=false);
    static void SetAccountOnlineStatus(uint32 accountID, bool online=false);
    static void SetAccountBanStatus(uint32 accountID, bool banned=false);
    static void SaveServerStats(double threads, float rss, float vm, float user, float kernel, uint32 items, uint32 bubbles);

    static uint32 SetClientSeed();

    static PyRep* LookupChars(const char *match, bool exact=false);
    static PyRep* LookupOwners(const char *match, bool exact=false);
    static PyRep* LookupCorporations(const std::string &);
    static PyRep* LookupFactions(const std::string &);
    static PyRep* LookupCorporationTickers(const std::string &);
    static PyRep* LookupStations(const std::string &);
    static PyRep* LookupKnownLocationsByGroup(const std::string &, uint32);

    static PyRep* PrimeOwners(std::vector<int32>& itemIDs);

    // tests for invalid characters
    static bool ValidateAccountName(CryptoChallengePacket& ccp, std::string& failMsg);

    // get corp hangar names for `.cargo` command
    static void GetCorpHangarNames(uint32 corpID, std::map<uint8, std::string> &hangarNames);

    static void SaveKillOrLoss(KillData& data);

protected:
    void ProcessStringChange(const char* key, const std::string& oldValue, std::string newValue, PyDict* notif, std::vector< std::string >& dbQ);
    void ProcessRealChange(const char * key, double oldValue, double newValue, PyDict * notif, std::vector<std::string> & dbQ);
    void ProcessIntChange(const char * key, uint32 oldValue, uint32 newValue, PyDict * notif, std::vector<std::string> & dbQ);
    void ProcessLongChange(const char* key, int64 oldValue, int64 newValue, PyDict* notif, std::vector< std::string >& dbQ);

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
    static uint32 CreateNewAccount( const char* login, const char* pass, const char* passHash, int64 role );
};


#endif

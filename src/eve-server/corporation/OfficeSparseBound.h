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
    Author:        Almamu
*/

#include "../eve-server.h"

#include "services/SparseBound.h"

#include "corporation/CorporationDB.h"
#include "Client.h"

class EVEServiceManager;

class OfficeSparseBound : public SparseBound <OfficeSparseBound>
{
public:
    OfficeSparseBound(EVEServiceManager &mgr, CorporationDB& db, uint32 corpID, PyList* headers);

protected:
    bool CanClientCall(Client* client) override;

    /**
     * Fetches the list of key values for the rowset
     *
     * @param res
     */
    bool LoadIndexes (DBQueryResult& res) override;

    /**
     * Fetches data from the database with the specified limits
     *
     * The first value in the result must be the key
     *
     * @param res The database result that will store the data
     * @param startPos
     * @param fetchSize
     */
    bool LoadFromDatabase (DBQueryResult& res, int startPos, int fetchSize) override;

    /**
     * Fetches data from the database with the specified limits
     *
     * The first value in the result must be the key
     *
     * @param res The database result that will store the data
     * @param keyList The values for the key we're going to search for
     */
    bool LoadFromDatabase (DBQueryResult& row, PyList* keyList) override;

    /**
     * Fetches data from the database with the specified limits
     *
     * The first value in the result must be the key
     *
     * @param res The database result that will store the data
     * @param columnName The column to filter by
     * @param values The values for the key we're going to search for
     */
    bool LoadFromDatabase (DBQueryResult& res, const std::string& columnName, PyList* values) override;

    /**
     * Sends the update notification
     *
     * @param update
     */
    void SendNotification (OnObjectPublicAttributesUpdated& update) override;

protected:
    CorporationDB& m_db;
    uint32 m_corpID;
};
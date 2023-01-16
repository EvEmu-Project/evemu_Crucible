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

#include "corporation/OfficeSparseBound.h"
#include "station/StationDataMgr.h"
#include "services/ServiceManager.h"
#include "EVE_Mail.h"
#include "corporation/CorpRegistryBound.h"

OfficeSparseBound::OfficeSparseBound(EVEServiceManager &mgr, CorpRegistryBound& parent, CorporationDB& db, uint32 corpID, PyList* headers) :
    SparseBound (mgr, parent, headers),
    m_db(db),
    m_corpID(corpID)
{
}

bool OfficeSparseBound::LoadIndexes (DBQueryResult& res) {
    return m_db.FetchOfficesKeys(m_corpID, res);
}

bool OfficeSparseBound::LoadFromDatabase (DBQueryResult& res, int startPos, int fetchSize) {
    return m_db.FetchOffices (m_corpID, startPos, fetchSize, res);
}

bool OfficeSparseBound::LoadFromDatabase (DBQueryResult& row, PyList* keyList) {
    // TODO: implement this
    return false;
}

bool OfficeSparseBound::LoadFromDatabase (DBQueryResult& res, const std::string& columnName, PyList* values) {
    // TODO: implement this
    return false;
}

void OfficeSparseBound::SendNotification (OnObjectPublicAttributesUpdated& update) {
    // TODO: this should be really notifying only the clients that are bound to this service
    // TODO: but everyone in the corp should be fine
    sEntityList.CorpNotify(m_corpID, Notify::Types::CorpNews, "OnObjectPublicAttributesUpdated", "corpid", update.Encode());
}

bool OfficeSparseBound::CanClientCall(Client* client) {
    return true; // TODO: properly implement this
}

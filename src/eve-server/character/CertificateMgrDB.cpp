/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
*/

#include "eve-server.h"

#include "character/CertificateMgrDB.h"

//PyRep* CertificateMgrDB::GetMyCertificates( uint32 characterID )
//{
//    sLog.Debug( "CertificateMgrDB", "Called GetMyCertificates stub." );
//
//    util_Rowset rs;
//
//    rs.header.push_back( "certificateID" );
//    rs.header.push_back( "grantDate" );
//    rs.header.push_back( "visiblityFlags" );
//
//    return rs.Encode();
//}

PyRep *CertificateMgrDB::GetCertificateCategories() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT categoryID, categoryName, description, dataID, categoryNameID FROM crtCategories")) {
        codelog(DATABASE__ERROR, "Failed to query certificate categories: %s.", res.error.c_str());
        return new PyNone();
    }

    return(DBResultToIndexRowset(res, "categoryID"));
}

PyRep *CertificateMgrDB::GetAllShipCertificateRecommendations() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT shipTypeID, certificateID, recommendationLevel, recommendationID FROM crtRecommendations")) {
        codelog(DATABASE__ERROR, "Failed to query certificate categories: %s.", res.error.c_str());
        return new PyNone();
    }

    return DBResultToRowset(res);
}

PyRep *CertificateMgrDB::GetCertificateClasses() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT classID, className, classNameID, description, dataID FROM crtClasses")) {
        codelog(DATABASE__ERROR, "Failed to query certificate classes: %s.", res.error.c_str());
        return new PyNone();
    }

    return DBResultToIntRowDict(res, 0);
}


void CertificateMgrDB::AddCertificate(uint32 charID, CharCerts cert) {
    DBerror err;
    if (!sDatabase.RunQuery( err,
        "INSERT INTO chrCertificates (characterID, certificateID, grantDate, visibilityFlags)"
        " VALUES (%u, %u, %li, %u)", charID, cert.certificateID, cert.grantDate, cert.visibilityFlags))  {
        _log(DATABASE__ERROR, "Failed to insert certificates of character %u: %s", charID, err.c_str() );
    }
}

void CertificateMgrDB::UpdateCertificate ( uint32 charID, uint32 certificateID, bool pub ) {
    DBerror err;
    if (!sDatabase.RunQuery( err,
        "UPDATE chrCertificates SET visibilityFlags = %u WHERE characterID = %u AND certificateID = %u", (pub ? 1 : 0), charID, certificateID)) {
        _log(DATABASE__ERROR, "Failed to insert certificates of character %u: %s", charID, err.c_str() );
    }
}


bool CertificateMgrDB::LoadCertificates( uint32 characterID, CertMap &into )
{
    DBQueryResult res;
    if ( !sDatabase.RunQuery( res, "SELECT certificateID, grantDate, visibilityFlags FROM chrCertificates WHERE characterID=%u", characterID)) {
        _log(DATABASE__ERROR, "Failed to query certificates of character %u: %s", characterID, res.error.c_str() );
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        CharCerts cert = CharCerts();
        cert.certificateID     = row.GetUInt( 0 );
        cert.grantDate         = row.GetInt64( 1 );
        cert.visibilityFlags   = row.GetBool( 2 );
        into.emplace(row.GetUInt( 0 ), cert );
    }

    return true;
}

bool CertificateMgrDB::SaveCertificates( uint32 characterID, const CertMap &data )
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO chrCertificates";
    Inserts << " (characterID, certificateID, grantDate, visibilityFlags)";
    bool first = true;
    for (auto cur : data) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else {
            Inserts << ", ";
        }
        Inserts << "(" << characterID << ", " << cur.first << ", " << cur.second.grantDate << ", " << cur.second.visibilityFlags << ")";
    }

    if (!first) {
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "visibilityFlags=VALUES(visibilityFlags)";
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveCertificates - unable to save data - %s", err.c_str());
    }
    return true;
}

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
	Author:		Zhur
*/

#include "EVEServerPCH.h"

PyRep* CertificateMgrDB::GetMyCertificates( uint32 characterID )
{
    sLog.Debug( "CertificateMgrDB", "Called GetMyCertificates stub." );

	util_Rowset rs;

	rs.header.push_back( "certificateID" );
	rs.header.push_back( "grantDate" );
	rs.header.push_back( "visiblityFlags" );

	return rs.Encode();
}

PyRep *CertificateMgrDB::GetCertificateCategories() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" categoryID,"
		" categoryName,"
		" description,"
		" 0 AS dataID"
		" FROM crtCategories"))
	{
		_log(DATABASE__ERROR, "Failed to query certificate categories: %s.", res.error.c_str());
		return(NULL);
	}

	return(DBResultToIndexRowset(res, "categoryID"));
}

PyRep *CertificateMgrDB::GetAllShipCertificateRecommendations() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" shipTypeID,"
		" certificateID,"
		" recommendationLevel,"
		" recommendationID"
		" FROM crtRecommendations"))
	{
		_log(DATABASE__ERROR, "Failed to query certificate categories: %s.", res.error.c_str());
		return(NULL);
	}

	return DBResultToRowset(res);
}

PyRep *CertificateMgrDB::GetCertificateClasses() {
	DBQueryResult res;

	if(!sDatabase.RunQuery(res,
		"SELECT"
		" classID,"
		" className,"
		" description,"
		" 0 AS dataID"
		" FROM crtClasses"))
	{
		_log(DATABASE__ERROR, "Failed to query certificate classes: %s.", res.error.c_str());
		return(NULL);
	}

	return(DBResultToIndexRowset(res, "classID"));
}




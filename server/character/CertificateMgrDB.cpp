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

#include "EvemuPCH.h"

CertificateMgrDB::CertificateMgrDB(DBcore *db)
: ServiceDB(db)
{
}

PyRep *CertificateMgrDB::GetMyCertificates(uint32 characterID) {
	_log(DATABASE__ERROR, "%lu: GetMyCertificates unimplemented.", characterID);

	util_Rowset rs;

	rs.header.push_back("certificateID");
	rs.header.push_back("grantDate");
	rs.header.push_back("visiblityFlags");

	return(rs.Encode());
}

PyRep *CertificateMgrDB::GetCertificateCategories() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
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

	return(DBResultToIndexRowset("categoryID", res));
}

PyRep *CertificateMgrDB::GetAllShipCertificateRecommendations() {
	_log(DATABASE__ERROR, "GetAllShipCertificateRecommendations unimplemented.");

	util_Rowset rs;

	rs.header.push_back("shipTypeID");
	rs.header.push_back("certificateID");
	rs.header.push_back("recommendationLevel");
	rs.header.push_back("recommendationID");

	return(rs.Encode());
}

PyRep *CertificateMgrDB::GetCertificateClasses() {
	DBQueryResult res;

	if(!m_db->RunQuery(res,
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

	return(DBResultToIndexRowset("classID", res));
}




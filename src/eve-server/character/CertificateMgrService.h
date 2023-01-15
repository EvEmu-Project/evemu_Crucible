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
    Author:        Zhur
*/

#ifndef __CERTIFICATE_MGR_SERVICE__H__INCL__
#define __CERTIFICATE_MGR_SERVICE__H__INCL__

#include "character/CertificateMgrDB.h"
#include "services/Service.h"
#include "services/ServiceManager.h"
#include "cache/ObjCacheService.h"
#include "Client.h"

class CertificateMgrService : public Service <CertificateMgrService>
{
public:
    CertificateMgrService(EVEServiceManager& mgr);

protected:
    EVEServiceManager& m_manager;
    CertificateMgrDB m_db;

    PyResult GetMyCertificates(PyCallArgs& call);
    PyResult GetCertificateCategories(PyCallArgs& call);
    PyResult GetAllShipCertificateRecommendations(PyCallArgs& call);
    PyResult GetCertificateClasses(PyCallArgs& call);
    PyResult GrantCertificate(PyCallArgs& call, PyInt* certificateID);
    PyResult UpdateCertificateFlags(PyCallArgs& call, PyInt* certificateID, PyInt* visibility);
    PyResult BatchCertificateGrant(PyCallArgs& call, PyList* certificateIDs);
    PyResult BatchCertificateUpdate(PyCallArgs& call, PyDict* batchUpdate);
    PyResult GetCertificatesByCharacter(PyCallArgs& call, PyInt* characterID);

private:
    ObjCacheService* m_cache;
};

#endif /* __CERTIFICATE_MGR_SERVICE__H__INCL__ */





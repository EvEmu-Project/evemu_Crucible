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
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "character/CertificateMgrService.h"

PyCallable_Make_InnerDispatcher(CertificateMgrService)

CertificateMgrService::CertificateMgrService(PyServiceMgr *mgr)
: PyService(mgr, "certificateMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CertificateMgrService, GetMyCertificates);
    PyCallable_REG_CALL(CertificateMgrService, GetCertificateCategories);
    PyCallable_REG_CALL(CertificateMgrService, GetAllShipCertificateRecommendations);
    PyCallable_REG_CALL(CertificateMgrService, GetCertificateClasses);
    PyCallable_REG_CALL(CertificateMgrService, GrantCertificate);
    PyCallable_REG_CALL(CertificateMgrService, UpdateCertificateFlags);
    PyCallable_REG_CALL(CertificateMgrService, BatchCertificateGrant);
    PyCallable_REG_CALL(CertificateMgrService, BatchCertificateUpdate);
    PyCallable_REG_CALL(CertificateMgrService, GetCertificatesByCharacter);
}

CertificateMgrService::~CertificateMgrService()
{
    delete m_dispatch;
}

PyResult CertificateMgrService::Handle_GetMyCertificates(PyCallArgs &call) {
    CertMap crt = CertMap();
    call.client->GetChar()->GetCertificates( crt );

    util_Rowset rs;
        rs.lines = new PyList();
        rs.header.push_back( "certificateID" );
        rs.header.push_back( "grantDate" );
        rs.header.push_back( "visibilityFlags" );

    for (auto cur : crt) {
        PyList* fieldData = new PyList();
            fieldData->AddItemInt( cur.first );
            fieldData->AddItemLong( cur.second.grantDate );
            fieldData->AddItemInt( cur.second.visibilityFlags );
        rs.lines->AddItem( fieldData );
    }
    return rs.Encode();
}

PyResult CertificateMgrService::Handle_GetCertificateCategories(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetCertificateCategories");

    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep* res = m_db.GetCertificateCategories();
        if (res == nullptr)
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
        m_manager->cache_service->GiveCache(method_id, &res);
    }

    return m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
}

PyResult CertificateMgrService::Handle_GetAllShipCertificateRecommendations(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetAllShipCertificateRecommendations");

    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep* res = m_db.GetAllShipCertificateRecommendations();
        if (res == nullptr)
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
        m_manager->cache_service->GiveCache(method_id, &res);
    }

    return m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
}

PyResult CertificateMgrService::Handle_GetCertificateClasses(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetCertificateClasses");

    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep* res = m_db.GetCertificateClasses();
        if (res == nullptr)
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
        m_manager->cache_service->GiveCache(method_id, &res);
    }

    return m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
}

PyResult CertificateMgrService::Handle_GrantCertificate(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    call.client->GetChar()->GrantCertificate(arg.arg);
    return PyStatic.NewNone();
}

PyResult CertificateMgrService::Handle_UpdateCertificateFlags(PyCallArgs &call) {
    Call_TwoIntegerArgs arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    call.client->GetChar()->UpdateCertificate( arg.arg1, arg.arg2 );
    return PyStatic.NewNone();
}

PyResult CertificateMgrService::Handle_BatchCertificateGrant(PyCallArgs &call) {
    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    PyList* res = new PyList();
    CharacterRef ch = call.client->GetChar();
    std::vector<int32>::iterator itr = arg.ints.begin();
    for (; itr != arg.ints.end(); ++itr) {
        ch->GrantCertificate(*itr);
        res->AddItemInt(*itr);
    }
    return res;
}

PyResult CertificateMgrService::Handle_BatchCertificateUpdate(PyCallArgs &call) {
    Call_BatchCertificateUpdate args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    CharacterRef ch = call.client->GetChar();
    std::map<uint32, uint32>::iterator itr = args.update.begin();
    for (; itr != args.update.end(); ++itr)
        ch->UpdateCertificate( itr->first, itr->second );
    return PyStatic.NewNone();
}

PyResult CertificateMgrService::Handle_GetCertificatesByCharacter( PyCallArgs& call )
{
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    CertMap crt;
    sItemFactory.GetCharacterRef(arg.arg)->GetCertificates(crt);

    util_Rowset rs;
        rs.lines = new PyList();
        rs.header.push_back("grantDate");
        rs.header.push_back("certificateID");
        rs.header.push_back("visibilityFlags");

    for (auto cur : crt) {
        PyList* fieldData = new PyList();
            fieldData->AddItemLong( cur.second.grantDate );
            fieldData->AddItemInt( cur.second.certificateID );
            fieldData->AddItemInt( cur.second.visibilityFlags );
        rs.lines->AddItem( fieldData );
    }
    return rs.Encode();
}

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


#include "cache/ObjCacheService.h"
#include "character/CertificateMgrService.h"

CertificateMgrService::CertificateMgrService(EVEServiceManager& mgr) :
    Service("certificateMgr"),
    m_manager (mgr)
{
    this->Add("GetMyCertificates", &CertificateMgrService::GetMyCertificates);
    this->Add("GetCertificateCategories", &CertificateMgrService::GetCertificateCategories);
    this->Add("GetAllShipCertificateRecommendations", &CertificateMgrService::GetAllShipCertificateRecommendations);
    this->Add("GetCertificateClasses", &CertificateMgrService::GetCertificateClasses);
    this->Add("GrantCertificate", &CertificateMgrService::GrantCertificate);
    this->Add("UpdateCertificateFlags", &CertificateMgrService::UpdateCertificateFlags);
    this->Add("BatchCertificateGrant", &CertificateMgrService::BatchCertificateGrant);
    this->Add("BatchCertificateUpdate", &CertificateMgrService::BatchCertificateUpdate);
    this->Add("GetCertificatesByCharacter", &CertificateMgrService::GetCertificatesByCharacter);

    this->m_cache = this->m_manager.Lookup <ObjCacheService>("objectCaching");
}

PyResult CertificateMgrService::GetMyCertificates(PyCallArgs &call) {
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

PyResult CertificateMgrService::GetCertificateCategories(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName().c_str(), "GetCertificateCategories");

    if (!this->m_cache->IsCacheLoaded(method_id)) {
        PyRep* res = m_db.GetCertificateCategories();
        if (res == nullptr)
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
        this->m_cache->GiveCache(method_id, &res);
    }

    return this->m_cache->MakeObjectCachedMethodCallResult(method_id);
}

PyResult CertificateMgrService::GetAllShipCertificateRecommendations(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName().c_str(), "GetAllShipCertificateRecommendations");

    if (!this->m_cache->IsCacheLoaded(method_id)) {
        PyRep* res = m_db.GetAllShipCertificateRecommendations();
        if (res == nullptr)
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
        this->m_cache->GiveCache(method_id, &res);
    }

    return this->m_cache->MakeObjectCachedMethodCallResult(method_id);
}

PyResult CertificateMgrService::GetCertificateClasses(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName().c_str(), "GetCertificateClasses");

    if (!this->m_cache->IsCacheLoaded(method_id)) {
        PyRep* res = m_db.GetCertificateClasses();
        if (res == nullptr)
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
        this->m_cache->GiveCache(method_id, &res);
    }

    return this->m_cache->MakeObjectCachedMethodCallResult(method_id);
}

PyResult CertificateMgrService::GrantCertificate(PyCallArgs &call, PyInt* certificateID) {
    call.client->GetChar()->GrantCertificate(certificateID->value());
    return PyStatic.NewNone();
}

PyResult CertificateMgrService::UpdateCertificateFlags(PyCallArgs &call, PyInt* certificateID, PyInt* visibility) {
    call.client->GetChar()->UpdateCertificate(certificateID->value(), visibility->value());
    return PyStatic.NewNone();
}

PyResult CertificateMgrService::BatchCertificateGrant(PyCallArgs &call, PyList* certificateIDs) {
    // TODO: rewrite this when improvements to py types are made
    std::vector<int32> ints;

    PyList::const_iterator list_2_cur = certificateIDs->begin();
    for (size_t list_2_index(0); list_2_cur != certificateIDs->end(); ++list_2_cur, ++list_2_index) {
        if (!(*list_2_cur)->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_SingleIntList failed: Element %u in list list_2 is not an integer: %s", list_2_index, (*list_2_cur)->TypeString());
            return nullptr;
        }

        const PyInt* t = (*list_2_cur)->AsInt();
        ints.push_back(t->value());
    }

    PyList* res = new PyList();
    CharacterRef ch = call.client->GetChar();
    std::vector<int32>::iterator itr = ints.begin();
    for (; itr != ints.end(); ++itr) {
        ch->GrantCertificate(*itr);
        res->AddItemInt(*itr);
    }
    return res;
}

PyResult CertificateMgrService::BatchCertificateUpdate(PyCallArgs &call, PyDict* batchUpdate) {
    // TODO: rewrite this when improvements to py types are made
    std::map<uint32, uint32> update;

    PyDict::const_iterator dict_2_cur = batchUpdate->begin();
    for (size_t dict_2_index(0); dict_2_cur != batchUpdate->end(); ++dict_2_cur, ++dict_2_index) {
        if (!dict_2_cur->first->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_BatchCertificateUpdate failed: Key %u in dict dict_2 is not Int: %s", dict_2_index, dict_2_cur->first->TypeString());
            return nullptr;
        }

        const PyInt* k = dict_2_cur->first->AsInt();
        if (!dict_2_cur->second->IsInt()) {
            _log(XMLP__DECODE_ERROR, "Decode Call_BatchCertificateUpdate failed: Value %d in dict dict_2 is not Int: %s", dict_2_index, dict_2_cur->second->TypeString());
            return nullptr;
        }

        const PyInt* v = dict_2_cur->second->AsInt();
        update[k->value()] = v->value();
    }
    CharacterRef ch = call.client->GetChar();
    std::map<uint32, uint32>::iterator itr = update.begin();
    for (; itr != update.end(); ++itr)
        ch->UpdateCertificate( itr->first, itr->second );
    return PyStatic.NewNone();
}

PyResult CertificateMgrService::GetCertificatesByCharacter(PyCallArgs& call, PyInt* characterID)
{
    CertMap crt;
    sItemFactory.GetCharacterRef(characterID->value())->GetCertificates(crt);

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

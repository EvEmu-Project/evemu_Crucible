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


#ifndef __PYSERVICEMGR_H_INCL__
#define __PYSERVICEMGR_H_INCL__

#include "eve-server.h"

class PyService;
class PyCallable;
class PyBoundObject;
class PyPacket;
class ServiceDB;
class Client;
class PyRep;
class PySubStruct;
class EntityList;
class ObjCacheService;
class DBcore;
class ItemFactory;

class LSCService;

class PyServiceMgr
{
public:
    struct BoundObj
    {
        Client* client;         //we do not own this.
        PyBoundObject* object;  //we own this. PyServiceMgr deletes it
    };

    PyServiceMgr( uint32 nodeID, EntityList& elist);
    ~PyServiceMgr();

    void Initalize(double startTime);

    void Close();
    void Process();

    void RegisterService(const std::string& name, PyService* svc);
    PyService* LookupService(const std::string& name);

    uint32 GetNodeID() const                            { return m_nodeID; }

    //object binding, not fully understood yet.
    PySubStruct *BindObject(Client* pClient, PyBoundObject* pObj, PyDict* dict=nullptr, PyDict *oid=nullptr);
    PyBoundObject *FindBoundObject(uint32 bindID);
    void ClearBoundObject(uint32 bindID);

    //Area to access services by name. This isn't ideal, but it avoids casting.
    //these may be NULL during service init, but should never be after that.
    //we do not own these pointers (we do in their PyService * form though)
    LSCService* lsc_service;
    ObjCacheService* cache_service;

    // for shitz-n-giggles...command to list all bound objects in map
    void BoundObjectVec(std::vector<BoundObj>& vec);

protected:
    std::map<std::string, PyService*>   m_svcList;      //we own these pointers.


    typedef std::map<uint32, BoundObj>  ObjectsBoundMap;
    typedef ObjectsBoundMap::iterator   ObjectsBoundMapItr;
    ObjectsBoundMap                     m_boundObjects;         // bindID/BoundObject{client/object(PyBoundObject)}

    uint32 m_nodeID;
    uint32 m_nextBindID;
};

#endif

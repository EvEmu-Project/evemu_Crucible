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

#include "Service.h"

template<class T>
Service<T>::Service(const std::string& name, AccessLevel level) :
    mName(std::move(name)),
    mAccessLevel(level)
{
}

template<class T>
const AccessLevel Service<T>::GetAccessLevel() const {
    return this->mAccessLevel;
}

template<class T>
const std::string& Service<T>::GetName() const {
    return this->mName;
}

template<class T>
void Service<T>::add (const std::string& name, CallHandler <T> handler) {
    this->mHandlers.push_back(std::make_pair (std::string (name), handler));
}

template<class T>
PyResult Service<T>::Dispatch(const std::string& name, PyCallArgs& args) {
    for (auto handler : this->mHandlers) {
        if (handler.first != name)
            continue;

        // try to call the function, might be a good idea to first check if it matches
        // to make our lifes easier
        return handler.second (reinterpret_cast <T*> (this), args);
    }

    throw std::runtime_error("Cannot find matching function to call");
}

MachoNetServiceTest::MachoNetServiceTest() :
    Service ("machoNet")
{
    this->add("test", &MachoNetServiceTest::test);
}

PyResult MachoNetServiceTest::test (PyCallArgs& args, PyBool* boolean) {
    return new PyString("Hello World");
}
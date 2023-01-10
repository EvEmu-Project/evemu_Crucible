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

template <class S>
template <typename T>
bool CallHandler<S>::validateArg (size_t index, PyCallArgs & args) {
    // handle optional values
    if constexpr (is_optional <T>::value) {
        if (index >= args.tuple->size()) {
            return true;
        } else {
            return validateArg <typename T::value_type>(index, args);
        }
    }

    if (index >= args.tuple->size()) {
        return false;
    }

    PyRep* rep = args.tuple->GetItem(index);

    // validate type with their parameter equivalent
    if constexpr (std::is_same_v <T, PyBool*>)
        return rep->IsBool();
    if constexpr (std::is_same_v <T, PyInt*>)
        return rep->IsInt();
    if constexpr (std::is_same_v <T, PyLong*>)
        return rep->IsLong();
    if constexpr (std::is_same_v <T, PyFloat*>)
        return rep->IsFloat();
    if constexpr (std::is_same_v <T, PyBuffer*>)
        return rep->IsBuffer();
    if constexpr (std::is_same_v <T, PyString*>)
        return rep->IsString();
    if constexpr (std::is_same_v <T, PyToken*>)
        return rep->IsToken();
    if constexpr (std::is_same_v <T, PyTuple*>)
        return rep->IsTuple();
    if constexpr (std::is_same_v <T, PyList*>)
        return rep->IsList();
    if constexpr (std::is_same_v <T, PyDict*>)
        return rep->IsDict();
    if constexpr (std::is_same_v <T, PyNone*>)
        return rep->IsNone();
    if constexpr (std::is_same_v <T, PySubStruct*>)
        return rep->IsSubStruct();
    if constexpr (std::is_same_v <T, PySubStream*>)
        return rep->IsSubStream();
    if constexpr (std::is_same_v <T, PyChecksumedStream*>)
        return rep->IsChecksumedStream();
    if constexpr (std::is_same_v <T, PyObject*>)
        return rep->IsObject();
    if constexpr (std::is_same_v <T, PyObjectEx*>)
        return rep->IsObjectEx();
    if constexpr (std::is_same_v <T, PyPackedRow*>)
        return rep->IsPackedRow();

    return false;
}

template <class S>
template <typename T>
decltype(auto) CallHandler<S>::getAs(size_t index, PyTuple* tup) {
    if constexpr (is_optional <T>::value) {
        if (index >= tup->size()) {
            return T{};
        } else {
            return std::make_optional (getAs <typename T::value_type>(index, tup));
        }
    }

    if (index >= tup->size()) {
        throw std::runtime_error("This should not happen. Trying to get parameter out of bounds. What happened to the validation?!");
    }

    PyRep* rep = tup->GetItem(index);

    if constexpr (std::is_same_v <T, PyBool*>)
        return rep->AsBool();
    if constexpr (std::is_same_v <T, PyInt*>)
        return rep->AsInt();
    if constexpr (std::is_same_v <T, PyLong*>)
        return rep->AsLong();
    if constexpr (std::is_same_v <T, PyFloat*>)
        return rep->AsFloat();
    if constexpr (std::is_same_v <T, PyBuffer*>)
        return rep->AsBuffer();
    if constexpr (std::is_same_v <T, PyString*>)
        return rep->AsString();
    if constexpr (std::is_same_v <T, PyToken*>)
        return rep->AsToken();
    if constexpr (std::is_same_v <T, PyTuple*>)
        return rep->AsTuple();
    if constexpr (std::is_same_v <T, PyList*>)
        return rep->AsList();
    if constexpr (std::is_same_v <T, PyDict*>)
        return rep->AsDict();
    if constexpr (std::is_same_v <T, PyNone*>)
        return rep->AsNone();
    if constexpr (std::is_same_v <T, PySubStruct*>)
        return rep->AsSubStruct();
    if constexpr (std::is_same_v <T, PySubStream*>)
        return rep->AsSubStream();
    if constexpr (std::is_same_v <T, PyChecksumedStream*>)
        return rep->AsChecksumedStream();
    if constexpr (std::is_same_v <T, PyObject*>)
        return rep->AsObject();
    if constexpr (std::is_same_v <T, PyObjectEx*>)
        return rep->AsObjectEx();
    if constexpr (std::is_same_v <T, PyPackedRow*>)
        return rep->AsPackedRow();
}

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
        try
        {
            return handler.second(reinterpret_cast <T*> (this), args);
        }
        catch (std::invalid_argument)
        {
            // ignored, this just means the function does not match the possible calls
        }
    }

    throw std::runtime_error("Cannot find matching function to call");
}

MachoNetServiceTest::MachoNetServiceTest() :
    Service ("machoNet")
{
    this->add("test", &MachoNetServiceTest::test);
    this->add("secondTest", &MachoNetServiceTest::secondTest);
}

PyResult MachoNetServiceTest::test (PyCallArgs& args, PyBool* boolean) {
    return new PyString("Hello World");
}


PyResult MachoNetServiceTest::secondTest(PyCallArgs& args, PyInt* value, std::optional<PyInt*> secondValue) {
    std::cout << value->value() << std::endl;

    if (secondValue.has_value()) {
        std::cout << "SECOND!!" << std::endl;
        std::cout << secondValue.value()->value() << std::endl;
    }

    return new PyString("second!");
}
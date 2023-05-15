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
    Update:     Allan
*/

#include "ClientSession.h"
#include "EntityList.h"
#include "EVEServerConfig.h"

/**
 * Map of variables that should never be sent through the wire on the session data
 * These variables will be ignored. In reality we should have a whitelist
 *
 * TODO: COME UP WITH A REAL WHITELIST OF THINGS THAT HAVE TO BE SENT TO THE CLIENT
 *
 * @author Almamu
 */
static std::map<std::string, bool> NONPERSISTVARS = {
    {"clientID", true},
    {"sessionID", true},
    {"sid", true}
};


ClientSession::ClientSession()
: mSession(new PyDict()),
mDirty(false),
m_sessionID(0)
{
    /* default session values */
    mSession->SetItemString("role", new_tuple(PyStatic.NewNone(), new PyLong(Acct::Role::PLAYER | Acct::Role::NEWBIE), PyStatic.NewFalse()));
    mSession->SetItemString("userid", new_tuple(PyStatic.NewNone(), PyStatic.NewZero(), PyStatic.NewFalse()));
    mSession->SetItemString("address", new_tuple(PyStatic.NewNone(), new PyString("0.0.0.0"), PyStatic.NewFalse()));

    /*  session id is unique to each session.
     * is not saved or shared between chars
     */
    //random.getrandbits(63)
    m_sessionID = GetTimeUSeconds() *15;
    sEntityList.RegisterSID(m_sessionID);
}

ClientSession::~ClientSession()
{
    // do we clear session vars here, or let ~PyDict() do it?
    PyDecRef(mSession);
    sEntityList.RemoveSID(m_sessionID);
}

// note:  cannot destroy these Py* objects here.
void ClientSession::Clear(const char* name)
{
    _Set(name, PyStatic.NewNone());
}

void ClientSession::SetInt(const char* name, int32 value)
{
    _Set(name, new PyInt(value));
}

void ClientSession::SetLong(const char* name, int64 value)
{
    _Set(name, new PyLong(value));
}

void ClientSession::SetString(const char* name, const char* value)
{
    _Set(name, new PyString(value));
}

void ClientSession::SetFloat(const char* name, double value)
{
    _Set(name, new PyFloat(value));
}

int32 ClientSession::GetLastInt(const char* name) const
{
    return PyRep::IntegerValue(_GetLast(name));
}

int32 ClientSession::GetCurrentInt(const char* name) const
{
    return PyRep::IntegerValue(_GetCurrent(name));
}

int64 ClientSession::GetLastLong(const char* name) const
{
    return PyRep::IntegerValue(_GetLast(name));
}

int64 ClientSession::GetCurrentLong(const char* name) const
{
    return PyRep::IntegerValue(_GetCurrent(name));
}

std::string ClientSession::GetLastString(const char* name) const
{
    return PyRep::StringContent(_GetLast(name));
}

std::string ClientSession::GetCurrentString(const char* name) const
{
    return PyRep::StringContent(_GetCurrent(name));
}

double ClientSession::GetLastFloat(const char* name) const
{
    return PyRep::FloatValue(_GetLast(name));
}

double ClientSession::GetCurrentFloat(const char* name) const
{
    return PyRep::FloatValue(_GetCurrent(name));
}

void ClientSession::EncodeChanges(PyDict* into)
{
    if (!mDirty)
        return;

    for (auto cur : *mSession)
    {
        PyTuple* valueTuple = cur.second->AsTuple ();

        if (valueTuple->GetItem (2)->AsBool ()->value () == false)
            continue;

        // mark the value as not new
        valueTuple->SetItem (2, PyStatic.NewFalse());

        // add the value to the list if it should be persisted
        if (NONPERSISTVARS.find (cur.first->AsString ()->content ()) == NONPERSISTVARS.end ())
            into->SetItem (cur.first, new_tuple (valueTuple->GetItem (0), valueTuple->GetItem(1)));
    }

    mDirty = false;
}

void ClientSession::EncodeInitialState (PyDict* into)
{
    for (auto cur : *mSession)
    {
        PyTuple* valueTuple = cur.second->AsTuple ();

        valueTuple->SetItem(2, PyStatic.NewFalse());

        // add the value to the initial state only if required
        if (NONPERSISTVARS.find (cur.first->AsString ()->content ()) == NONPERSISTVARS.end ())
            into->SetItem (cur.first, cur.second->AsTuple ()->GetItem (1));
    }

    // mark the session as not dirty
    mDirty = false;
}

bool ClientSession::HasValue (const char* name) const
{
    PyTuple* valueTuple = _GetValueTuple (name);

    if (valueTuple == nullptr)
        return false;

    PyRep* value = valueTuple->GetItem(1);

    return value != nullptr && value->IsNone() == false;
}

PyTuple* ClientSession::_GetValueTuple(const char* name) const
{
    PyRep* value(mSession->GetItemString(name));
    if (value == nullptr)
        return nullptr;
    return value->AsTuple();
}

PyRep* ClientSession::_GetLast(const char* name) const
{
    PyTuple* tuple(_GetValueTuple(name)); // copy c'tor
    if (tuple == nullptr) {
        _log(CLIENT__SESSION_NOTFOUND, "ClientSession::_GetLast - value not found with name '%s'", name);
        return nullptr;
    }
    return tuple->GetItem(0);
}

PyRep* ClientSession::_GetCurrent(const char* name) const
{
    PyTuple* tuple(_GetValueTuple(name)); // copy c'tor
    if (tuple == nullptr) {
        if (is_log_enabled(CLIENT__SESSION_NOTFOUND)) {
            _log(CLIENT__SESSION_NOTFOUND, "ClientSession::_GetCurrent - value not found with name '%s'", name);
            EvE::traceStack();
        }
        return nullptr;
    }
    return tuple->GetItem(1);
}

void ClientSession::_Set(const char* name, PyRep* value)
{
    PyTuple* tuple(_GetValueTuple(name)); // copy c'tor
    if (tuple == nullptr) {
        tuple = new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse());
        mSession->SetItemString(name, tuple);
    }

    PyRep* current(tuple->GetItem(1)); // copy c'tor
    if (value->hash() != current->hash()) {
        tuple->SetItem(0, current); /* didn't the session need to store the old value too? */
        tuple->SetItem(1, value);
        tuple->SetItem(2, PyStatic.NewTrue());
        mDirty = true;
    } else {
        PyDecRef(value);
    }
}

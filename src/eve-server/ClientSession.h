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
    Author:        Bloody.Rabbit
    Updates:    Allan
*/

#ifndef __CLIENT_SESSION_H__INCL__
#define __CLIENT_SESSION_H__INCL__

#include "eve-server.h"
/**
 * @brief Value keeper for single EVE session.
 *
 * This object keeps track of string-value pairs
 * and encodes their changes as session changes.
 */
class ClientSession
{
public:
    ClientSession();
    ~ClientSession();

    bool isDirty() const { return mDirty; }

    // PyInt
    void SetInt( const char* name, int32 value );
    int32 GetLastInt( const char* name ) const;
    int32 GetCurrentInt( const char* name ) const;
    // PyLong
    void SetLong( const char* name, int64 value );
    int64 GetLastLong( const char* name ) const;
    int64 GetCurrentLong( const char* name ) const;
    // PyString
    void SetString( const char* name, const char* value );
    std::string GetLastString( const char* name ) const;
    std::string GetCurrentString( const char* name ) const;

    void Clear( const char* name );

    /**
     * Buils a PyDictionary with the list of changes the session has gone through
     * Every item is a tuple that includes the old and new value so the client can compare against what it has
     *
     * @param into
     */
    void EncodeChanges( PyDict* into );

    /**
     * Builds a PyDictionary with the current state of the session. Useful for macho.SessionInitialStateNotification
     * The side-effects include overwriting of the old value on the session data with the new one
     *
     * @param into
     */
    void EncodeInitialState (PyDict* into);

    int64 GetSessionID()  { return m_sessionID; }

protected:
    PyRep* _GetLast( const char* name ) const;
    PyRep* _GetCurrent( const char* name ) const;
    PyTuple* _GetValueTuple( const char* name ) const;

    void _Set( const char* name, PyRep* value );

    PyDict* const mSession;

private:
    bool mDirty;

    int64 m_sessionID;
};

#endif /* !__CLIENT_SESSION_H__INCL__ */

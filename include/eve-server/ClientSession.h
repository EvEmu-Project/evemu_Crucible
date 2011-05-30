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
	Author:		Bloody.Rabbit
*/

#ifndef __CLIENT_SESSION_H__INCL__
#define __CLIENT_SESSION_H__INCL__

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
    int32 GetLastInt( const char* name ) const;
    int32 GetCurrentInt( const char* name ) const;
    void SetInt( const char* name, int32 value );

    // PyLong
    int64 GetLastLong( const char* name ) const;
    int64 GetCurrentLong( const char* name ) const;
    void SetLong( const char* name, int64 value );

    // PyString
    std::string GetLastString( const char* name ) const;
    std::string GetCurrentString( const char* name ) const;
    void SetString( const char* name, const char* value );

    void Clear( const char* name );
    void EncodeChanges( PyDict* into );

protected:
    PyTuple* _GetValueTuple( const char* name ) const;

    PyRep* _GetLast( const char* name ) const;
    PyRep* _GetCurrent( const char* name ) const;
    void _Set( const char* name, PyRep* value );

    PyDict* const mSession;
	bool mDirty;
};

#endif /* !__CLIENT_SESSION_H__INCL__ */

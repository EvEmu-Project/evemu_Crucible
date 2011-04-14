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

#ifndef __PY_EXCEPTIONS_H__INCL__
#define __PY_EXCEPTIONS_H__INCL__

#include "python/PyRep.h"

/**
 * @brief Python object "exceptions.GPSTransportClosed".
 *
 * This exception is used when
 * refusing client login.
 *
 * @author Bloody.Rabbit
 */
class GPSTransportClosed
: public PyObjectEx_Type1
{
public:
	GPSTransportClosed( const char* reason );

	/**
	 * @brief Adds keyword to exception.
	 *
	 * @param[in] name The keyword.
	 * @param[in] value Value to be associated with the keyword.
	 */
	void AddKeyword( const char* name, PyRep* value );

protected:
	PyDict* _GetReasonArgs() const;

	static PyTuple* _CreateArgs( const char* reason );
	static PyDict* _CreateKeywords( const char* reason );
};

/**
 * @brief Python object "ccp_exceptions.UserError"
 *
 * This exception can be used
 * for various errors.
 *
 * @author Bloody.Rabbit
 */
class UserError
: public PyObjectEx_Type1
{
public:
	UserError( const char* msg );

	/**
	 * @brief Adds keyword to exception.
	 *
	 * @param[in] name The keyword.
	 * @param[in] value Value to be associated with the keyword.
	 */
	void AddKeyword( const char* name, PyRep* value );

protected:
	PyDict* _GetTupleKeywords() const;
	PyDict* _GetDictKeywords() const;

	static PyTuple* _CreateArgs( const char* msg );
	static PyDict* _CreateKeywords( const char* msg );
};

#endif /* !__PY_EXCEPTIONS_H__INCL__ */

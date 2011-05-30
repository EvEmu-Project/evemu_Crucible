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
    Author:     Bloody.Rabbit
*/

#ifndef __PY_UTILS_H__INCL__
#define __PY_UTILS_H__INCL__

#include "python/PyRep.h"

/**
 * @brief Python object "util.PasswordString"
 *
 * This object is used when transporting
 * plain password over network.
 *
 * @author Bloody.Rabbit
 */
class PasswordString
: public PyObjectEx_Type2
{
public:
	/**
	 * @param[in] password Password to be stored.
	 */
	PasswordString( PyWString* password );

	/** @return Password stored within this object. */
	PyWString* GetPassword() const;

protected:
	static PyTuple* _CreateArgs( PyWString* password );
};

#endif /* !__PY_UTILS_H__INCL__ */


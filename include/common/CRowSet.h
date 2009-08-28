/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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

#ifndef __CROWSET_H__INCL__
#define __CROWSET_H__INCL__

#include "PyRep.h"

/**
 * \brief Python object "dbutil.CRowset".
 *
 * This object contains DBRowDescriptor header
 * and bunch of PyPackedRows.
 *
 * @note At the moment it's designed that once
 * it's created, its header cannot be changed.
 * I don't know whether it's correct or not,
 * but it makes our life easier.
 *
 * \author Bloody.Rabbit
 */
class CRowSet
: public PyObjectEx_Type2
{
public:
	/**
	 * @param[in] rowDesc DBRowDescriptor header to be used.
	 */
	CRowSet(DBRowDescriptor **rowDesc);

	/**
	 * @return Row count.
	 */
	uint32 GetRowCount() const;
	/**
	 * @param[in] Index of row to be returned.
	 *
	 * @return Row with given index.
	 */
	PyPackedRow &GetRow(uint32 index) const;

	/**
	 * @return New row which user may fill.
	 */
	PyPackedRow &NewRow();

	CRowSet *TypedClone() const { return static_cast<CRowSet *>( PyObjectEx::TypedClone() ); }

protected:
	DBRowDescriptor &_GetRowDesc() const;
	PyList &_GetColumnList() const;

	static PyTuple *_CreateArgs();
	static PyDict *_CreateKeywords(DBRowDescriptor *rowDesc);
};

#endif /* !__CROWSET_H__INCL__ */
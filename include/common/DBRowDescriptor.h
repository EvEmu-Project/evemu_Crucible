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

#ifndef __DB_ROW_DESCRIPTOR_H__INCL__
#define __DB_ROW_DESCRIPTOR_H__INCL__

#include "packet_types.h"
#include "dbcore.h"

#include "PyRep.h"

/**
 * \brief Python object "blue.DBRowDescriptor".
 *
 * This object is used by PyPackedRow and CRowSet
 * to describe stored columns.
 *
 * \author Bloody.Rabbit
 */
class DBRowDescriptor
: public PyObjectEx_Type1
{
public:
	DBRowDescriptor();
	/**
	 * @param[in] result Query result to build column list from.
	 */
	DBRowDescriptor(const DBQueryResult &res);
	/**
	 * @param[in] result Row to build column list from.
	 */
	DBRowDescriptor(const DBResultRow &row);

	/**
	 * @return Column count.
	 */
    uint32 ColumnCount() const;
	/**
	 * @param[in] index Index of column name of which should be returned.
	 * @return Name of column.
	 */
    std::string &GetColumnName(uint32 index) const;
    /**
     * @param[in] index Index of column name of which should be returned.
     * @param[out] str PyString pointer that gets set with the column name.
    */
    void GetColumnName(uint32 index, PyString *& str) const;
	/**
	 * @param[in] index Index of column type of which should be returned.
	 * @return Type of column.
	 */
    DBTYPE &GetColumnType(uint32 index) const;
	/**
	 * @param[in] name Name of column index of which should be returned.
	 * @return Index of column; column count if not found.
	 */
    uint32 FindColumn(const char *name) const;

	/**
	 * Adds new column.
	 *
	 * @param[in] name Name of new column.
	 * @param[in] type Type of new column.
	 */
	void AddColumn(const char *name, DBTYPE type);

	// Some utilities:
	DBRowDescriptor *TypedClone() const { return static_cast<DBRowDescriptor *>( PyObjectEx::TypedClone() ); }

protected:
	// Helper functions:
	PyTuple &_GetColumnList() const;
	PyTuple &_GetColumn(size_t index) const;

	static PyTuple *_CreateArgs();
};

#endif /* !__DB_ROW_DESCRIPTOR_H__INCL__ */


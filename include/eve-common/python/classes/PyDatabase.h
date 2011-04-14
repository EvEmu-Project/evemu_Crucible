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

#ifndef __PY_DATABASE_H__INCL__
#define __PY_DATABASE_H__INCL__

#include "database/dbcore.h"
#include "python/PyRep.h"

/**
 * @brief Python object "blue.DBRowDescriptor".
 *
 * This object is used by PyPackedRow and CRowSet
 * to describe stored columns.
 *
 * @author Bloody.Rabbit
 */
class DBRowDescriptor
: public PyObjectEx_Type1
{
public:
	DBRowDescriptor();
	/**
	 * @param[in] result Query result to build column list from.
	 */
	DBRowDescriptor( const DBQueryResult& res );
	/**
	 * @param[in] result Row to build column list from.
	 */
	DBRowDescriptor( const DBResultRow& row );

	/**
	 * @return Column count.
	 */
    uint32 ColumnCount() const;
	/**
	 * @param[in] index Index of column name of which should be returned.
     *
	 * @return Name of column.
	 */
    PyString* GetColumnName( uint32 index ) const;
	/**
	 * @param[in] index Index of column type of which should be returned.
     *
	 * @return Type of column.
	 */
    DBTYPE GetColumnType( uint32 index ) const;
	/**
	 * @param[in] name Name of column index of which should be returned.
     *
	 * @return Index of column; column count if not found.
	 */
    uint32 FindColumn( const char* name ) const;
    /**
     * @brief Verifies value for certain column.
     *
     * @param[in] index Index of the column.
     * @param[in] value The value.
     *
     * @retval true  Type of column and type of value are compatible.
     * @retval false Type of column and type of value aren't compatible.
     */
    bool VerifyValue( uint32 index, PyRep* value );

	/**
	 * Adds new column.
	 *
	 * @param[in] name Name of new column.
	 * @param[in] type Type of new column.
	 */
	void AddColumn( const char* name, DBTYPE type );

protected:
	// Helper functions:
	PyTuple* _GetColumnList() const;
	PyTuple* _GetColumn(size_t index) const;

	static PyTuple* _CreateArgs();
};

/**
 * @brief Python object "dbutil.CRowset".
 *
 * This object contains DBRowDescriptor header
 * and bunch of PyPackedRows.
 *
 * @note At the moment it's designed that once
 * it's created, its header cannot be changed.
 * I don't know whether it's correct or not,
 * but it makes our life easier.
 *
 * @author Bloody.Rabbit
 */
class CRowSet
: public PyObjectEx_Type2
{
public:
    /**
     * @param[in] rowDesc DBRowDescriptor header to be used.
     */
    CRowSet( DBRowDescriptor** rowDesc );

    /**
     * @return Row count.
     */
    size_t GetRowCount() const { return list().size(); }

    /**
     * @param[in] Index of row to be returned.
     *
     * @return Row with given index.
     */
    PyPackedRow* GetRow( uint32 index ) const { return list().GetItem( index )->AsPackedRow(); }

    /**
     * @return New row which user may fill.
     */
    PyPackedRow* NewRow();

protected:
	DBRowDescriptor* _GetRowDesc() const;
	PyList* _GetColumnList() const;

	static PyTuple* _CreateArgs();
	static PyDict* _CreateKeywords(DBRowDescriptor* rowDesc);
};

#endif /* !__PY_DATABASE_H__INCL__ */

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
    Author:     Zhur, Bloody.Rabbit
*/

#include "EVECommonPCH.h"

#include "database/RowsetToSQL.h"

const size_t INSERT_QUERY_ROW_LIMIT = 1024;

/************************************************************************/
/* ReaderColumnContentDesc                                              */
/************************************************************************/
const char* const ReaderColumnContentDesc::smTypeStrings[] =
{
    "int",              /* TYPE_INT */
    "real",             /* TYPE_FLOAT */
    "text /* ascii */", /* TYPE_STRING */
    "text /* utf8 */",  /* TYPE_WSTRING */
    "UNKNOWN",          /* TYPE_UNKNOWN */
    "ERROR",            /* TYPE_ERROR */
};

const char* const ReaderColumnContentDesc::smNullStrings[] =
{
    "NOT NULL", /* !isNull */
    "NULL",     /* isNull */
};

ReaderColumnContentDesc::ReaderColumnContentDesc()
: mType( TYPE_UNKNOWN ),
  mIsNull( false )
{
}

void ReaderColumnContentDesc::Process( const PyRep::PyType t )
{
    switch( t )
    {
    case PyRep::PyTypeNone:
        mIsNull = true;
        break;

    case PyRep::PyTypeBool:
    case PyRep::PyTypeInt:
    case PyRep::PyTypeLong:
        switch( mType )
        {
        case TYPE_UNKNOWN:
            mType = TYPE_INTEGER;
            break;
        case TYPE_INTEGER:
        case TYPE_FLOAT:
            break;
        default:
            mType = TYPE_ERROR;
            break;
        }
        break;

    case PyRep::PyTypeFloat:
        switch( mType )
        {
        case TYPE_UNKNOWN:
        case TYPE_INTEGER:
            mType = TYPE_FLOAT;
            break;
        case TYPE_FLOAT:
            break;
        default:
            mType = TYPE_ERROR;
            break;
        }
        break;

    case PyRep::PyTypeString:
        switch( mType )
        {
        case TYPE_UNKNOWN:
            mType = TYPE_STRING;
            break;
        case TYPE_STRING:
        case TYPE_WSTRING:
            break;
        default:
            mType = TYPE_ERROR;
            break;
        }
        break;

    case PyRep::PyTypeWString:
        switch( mType )
        {
        case TYPE_UNKNOWN:
        case TYPE_STRING:
            mType = TYPE_WSTRING;
            break;
        case TYPE_WSTRING:
            break;
        default:
            mType = TYPE_ERROR;
            break;
        }
        break;

    default:
        mType = TYPE_ERROR;
        break;
    }
}

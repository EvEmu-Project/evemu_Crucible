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
    Author:     Zhur
*/

#include "EVECommonPCH.h"

#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyDumpVisitor.h"
#include "utils/EVEUtils.h"

/************************************************************************/
/* PyDumpVisitor                                                        */
/************************************************************************/
PyDumpVisitor::PyDumpVisitor( const char* pfx, bool full_nested ) : PyPfxVisitor( pfx ), mFullNested( full_nested ) {}

bool PyDumpVisitor::VisitInteger( const PyInt* rep )
{
    _print( "%sInteger field: %d", _pfx(), rep->value() );
    return true;
}

bool PyDumpVisitor::VisitLong( const PyLong* rep )
{
    _print( "%sInteger field: "I64d, _pfx(), rep->value() );
    return true;
}

bool PyDumpVisitor::VisitReal( const PyFloat* rep )
{
    _print( "%sReal field: %f", _pfx(), rep->value() );
    return true;
}

bool PyDumpVisitor::VisitBoolean( const PyBool* rep )
{
    _print( "%sBoolean field: %s", _pfx(), rep->value() ? "true" : "false" );
    return true;
}

bool PyDumpVisitor::VisitNone( const PyNone* rep )
{
    _print( "%s(None)", _pfx() );
    return true;
}

bool PyDumpVisitor::VisitBuffer( const PyBuffer* rep )
{
    _print( "%sBuffer of length %lu:", _pfx(), rep->content().size() );

    _pfxExtend( "  " );
    _dump( _pfx(), &rep->content()[ 0 ], rep->content().size() );
    _pfxWithdraw();

    return true;
}

bool PyDumpVisitor::VisitString( const PyString *rep )
{
    if( IsPrintable( rep ) )
        _print( "%sString: '%s'", _pfx(), rep->content().c_str() );
    else
        _print( "%sString: '<binary, len=%lu>'", _pfx(), rep->content().length() );

    return true;
}

bool PyDumpVisitor::VisitWString( const PyWString* rep )
{
    // how to do it correctly?
    if( IsPrintable( rep ) )
        _print( "%sWString: '%s'", _pfx(), rep->content().c_str() );
    else
        _print( "%sWstring: '<binary, len=%lu>'", _pfx(), rep->content().length() );

    return true;
}

bool PyDumpVisitor::VisitToken( const PyToken* rep )
{
    _print( "%sToken: '%s'", _pfx(), rep->content().c_str() );

    return true;
}

bool PyDumpVisitor::VisitTuple( const PyTuple* rep )
{
    if( rep->empty() )
        _print( "%sTuple: Empty", _pfx() );
    else
    {
        _print( "%sTuple: %lu elements", _pfx(), rep->size() );

        PyTuple::const_iterator cur, _end;
        cur = rep->begin();
        _end = rep->end();
        for( uint32 i = 0; cur != _end; cur++, i++ )
        {
            if( i > 200 && !fullNested() )
            {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] ", i );
            bool res = (*cur)->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;
        }
    }
    return true;
}

bool PyDumpVisitor::VisitList( const PyList* rep )
{
    if( rep->empty() )
        _print( "%sList: Empty", _pfx() );
    else
    {
        _print( "%sList: %lu elements", _pfx(), rep->size() );

        PyList::const_iterator cur, _end;
        cur = rep->begin();
        _end = rep->end();
        for( uint32 i = 0; cur != _end; cur++, i++ )
        {
            if( i > 200 && !fullNested() )
            {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] ", i );
            bool res = (*cur)->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;
        }
    }
    return true;
}

bool PyDumpVisitor::VisitDict( const PyDict* rep )
{
    if( rep->empty() )
        _print( "%sDictionary: Empty", _pfx() );
    else
    {
        _print( "%sDictionary: %lu entries", _pfx(), rep->size() );

        PyDict::const_iterator cur, _end;
        cur = rep->begin();
        _end = rep->end();
        for( uint32 i = 0; cur != _end; cur++, i++ )
        {
            if( i > 200 && !fullNested() )
            {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] Key: ", i );
            bool res = cur->first->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;

            _pfxExtend( "  [%2u] Value: ", i );
            res = cur->second->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;
        }
    }
    return true;
}

bool PyDumpVisitor::VisitObject( const PyObject* rep )
{
    _print( "%sObject:", _pfx() );

    _pfxExtend( "  Type: " );
    bool res = rep->type()->visit( *this );
    _pfxWithdraw();

    if( !res )
        return false;

    _pfxExtend( "  Args: " );
    res = rep->arguments()->visit( *this );
    _pfxWithdraw();

    if( !res )
        return false;

    return true;
}

bool PyDumpVisitor::VisitObjectEx( const PyObjectEx* rep )
{
    _print( "%sObjectEx%s:", _pfx(), rep->isType2() ? " (Type2)" : "" );

    _print( "%sHeader:", _pfx() );
    if( rep->header() == NULL )
        _print( "%s  (None)", _pfx() );
    else
    {
        _pfxExtend( "  " );
        bool res = rep->header()->visit( *this );
        _pfxWithdraw();

        if( !res )
            return false;
    }

    _print( "%sList data:", _pfx() );
    if( rep->list().empty() )
        _print( "%s  Empty", _pfx() );
    else
    {
        PyObjectEx::const_list_iterator cur, end;
        cur = rep->list().begin();
        end = rep->list().end();
        for( uint32 i = 0; cur != end; cur++, i++ )
        {
            if( i > 200 && !fullNested() )
            {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] ", i );
            bool res = (*cur)->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;
        }
    }

    _print( "%sDict data:", _pfx() );
    if( rep->dict().empty() )
        _print( "%s  Empty", _pfx() );
    else
    {
        PyObjectEx::const_dict_iterator cur, end;
        cur = rep->dict().begin();
        end = rep->dict().end();
        for( uint32 i = 0; cur != end; cur++, i++ )
        {
            if( i > 200 && !fullNested() )
            {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] Key: ", i );
            bool res = cur->first->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;

            _pfxExtend( "  [%2u] Value: ", i );
            res = cur->second->visit( *this );
            _pfxWithdraw();

            if( !res )
                return false;
        }
    }

    return true;
}

bool PyDumpVisitor::VisitPackedRow( const PyPackedRow* rep )
{
    _print( "%sPacked Row:", _pfx() );
    _print( "%s column_count=%u", _pfx(), rep->header()->ColumnCount() );

    PyPackedRow::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for( uint32 i = 0; cur != end; cur++, i++ )
    {
        _pfxExtend( "  [%2u] %s: ", i, rep->header()->GetColumnName( i )->content().c_str() );

        bool res = true;
        if( (*cur) == NULL )
            _print( "%s(None)", _pfx() );
        else
            res = (*cur)->visit( *this );

        _pfxWithdraw();

        if( !res )
            return false;
    }

    return true;
}

bool PyDumpVisitor::VisitSubStruct( const PySubStruct* rep )
{
    _print( "%sSubstruct:", _pfx() );

    _pfxExtend( "  " );
    bool res = PyVisitor::VisitSubStruct( rep );
    _pfxWithdraw();

    return res;
}

bool PyDumpVisitor::VisitSubStream( const PySubStream* rep )
{
    _print( "%sSubstream: %s", _pfx(), ( rep->decoded() == NULL ) ? "from data" : "from rep" );

    _pfxExtend( "  " );
    bool res = PyVisitor::VisitSubStream( rep );
    _pfxWithdraw();

    return res;
}

bool PyDumpVisitor::VisitChecksumedStream( const PyChecksumedStream* rep )
{
    _print( "%sStream With Checksum: 0x%08x", _pfx(), rep->checksum() );

    _pfxExtend( "  " );
    bool res = PyVisitor::VisitChecksumedStream( rep );
    _pfxWithdraw();

    return res;
}

PyLogDumpVisitor::PyLogDumpVisitor( LogType log_type, LogType log_hex_type, const char* pfx, bool full_nested, bool full_hex )
: PyDumpVisitor( pfx, full_nested ),
  mFullHex( full_hex ),
  mLogType( log_type ),
  mLogHexType( log_hex_type )
{
}

void PyLogDumpVisitor::_print( const char* fmt, ... )
{
    if( !is_log_enabled( logType() ) )
        return;

    va_list ap;
    va_start( ap, fmt );

    log_messageVA( logType(), fmt, ap );

    va_end( ap );
}

void PyLogDumpVisitor::_dump( const char* pfx, const uint8* data, size_t len )
{
    if( !is_log_enabled( logHexType() ) )
        return;

    if( fullHex() )
        pfxHexDump( pfx, logHexType(), data, len );
    else
        pfxHexDumpPreview( pfx, logHexType(), data, len );
}

PyFileDumpVisitor::PyFileDumpVisitor( FILE* _file, const char* pfx, bool full_nested, bool full_hex )
: PyDumpVisitor( pfx, full_nested ),
  mFullHex( full_hex ),
  mFile( _file )
{
}

void PyFileDumpVisitor::_print( const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    vfprintf( file(), fmt, ap );
    fprintf( file(), "\n" );

    va_end( ap );
}

void PyFileDumpVisitor::_dump( const char* pfx, const uint8* data, size_t len )
{
    if( fullHex() )
        pfxHexDump( pfx, file(), data, len );
    else
        pfxHexDumpPreview( pfx, file(), data, len );
}

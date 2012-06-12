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

#include "eve-test.h"

int marshal_EVEMarshalTest( int argc, char* argv[] )
{
    DBRowDescriptor *header = new DBRowDescriptor;
    // Fill header:
    header->AddColumn( "historyDate", DBTYPE_FILETIME );
    header->AddColumn( "lowPrice", DBTYPE_CY );
    header->AddColumn( "highPrice", DBTYPE_CY );
    header->AddColumn( "avgPrice", DBTYPE_CY );
    header->AddColumn( "volume", DBTYPE_I8 );
    header->AddColumn( "orders", DBTYPE_I4 );

    CRowSet* rs = new CRowSet( &header );

    PyPackedRow* row = rs->NewRow();
    row->SetField( "historyDate", new PyLong( Win32TimeNow() ) );
    row->SetField( "lowPrice", new PyLong( 18000 ) );
    row->SetField( "highPrice", new PyLong( 19000 ) );
    row->SetField( "avgPrice", new PyLong( 18400 ) );
    row->SetField( "volume", new PyLong( 5463586 ) );
    row->SetField( "orders", new PyInt( 254 ) );

    ::puts( "Marshaling..." );

    Buffer marshaled;
    bool res = MarshalDeflate( rs, marshaled );
    PyDecRef( rs );

    if( !res )
    {
        ::puts( "Failed to marshal Python object." );
        return EXIT_FAILURE;
    }

    ::puts( "Unmarshaling..." );

    PyRep* rep = InflateUnmarshal( marshaled );
    if( NULL == rep )
    {
        ::puts( "Failed to unmarshal Python object." );
        return EXIT_FAILURE;
    }

    ::puts( "Final:" );
    rep->Dump( stdout, "    " );
    PyDecRef( rep );

    return EXIT_SUCCESS;
}

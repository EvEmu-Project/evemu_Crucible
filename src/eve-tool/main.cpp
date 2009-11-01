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
    Author:        Zhur
*/

#include "EVEToolPCH.h"

#include "main.h"
#include "Commands.h"

int main( int argc, char* argv[] )
{
    // Load server log settings ( will be removed )
    if( !load_log_settings( "log.ini" ) )
        sLog.Warning( "init", "Unable to read %s (this file is optional)", "log.ini" );
    else
        sLog.Success( "init", "Log settings loaded from %s", "log.ini" );

    //skip first argument (launch path), we don't need it
    --argc;
    ++argv;

    for(; argc > 0; --argc, ++argv)
        ProcessFile( *argv );

    ProcessFile( stdin );

    sLog.Log( "shutdown", "Exiting." );

    return 0;
}

void ProcessString( const char* str )
{
    ProcessCommand( Seperator( str ) );
}

void ProcessString( const std::string& str )
{
    ProcessString( str.c_str() );
}

void ProcessFile( FILE* file )
{
    std::string cmd;

    while( true )
    {
        char input = fgetc( file );

        if( '\n' == input || EOF == input )
        {
            if( !cmd.empty() )
            {
                ProcessString( cmd );
                cmd.clear();
            }

            if( EOF == input )
                break;
        }
        else
            cmd += input;
    }
}

void ProcessFile( const char* filename )
{
    FILE* file = fopen( filename, "r" );
    if( file == NULL )
    {
        sLog.Error( "input", "Unable to open script '%s'.", filename );
        return;
    }

    sLog.Log( "input", "Queuing commands from script '%s'.", filename );
    ProcessFile( file );

    if( feof( file ) )
        sLog.Success( "input", "Load of script '%s' successfully completed.", filename );
    else
        sLog.Error( "input", "Error occured while reading script '%s'.", filename );

    fclose( file );
}





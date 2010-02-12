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
	Author:		Zhur
*/

#include "XMLPktGenPCH.h"

#include "XMLPacketGen.h"

int main( int argc, char* argv[] )
{
    if( 1 == argc )
    {
        printf( "Usage: %s xml-packet-file [xml-packet-file [...]]\n", *argv );

        return -1;
    }

    // skip first argument, not interested
    --argc;
    ++argv;

    XMLPacketGen gen;
    for(; 0 < argc; --argc, ++argv )
    {
        const char* file = *argv;

        std::string basename = file;
        basename = basename.substr( 0, basename.length() - 5 );

        std::string xml = basename + ".xmlp";
        std::string h = basename + ".h";
        std::string cpp = basename + ".cpp";

        if( xml != file )
        {
            printf( "Error: bad input filename '%s'.\n", file );

            return -1;
        }

#ifdef WIN32
        //this is a hack to deal with the issue that we cannot seem to get
        //VC++ to properly track the dependancies for custom build rules,
        //which results in it always rebuilding the XML packets, which makes
        // build times incredibly long. This checks file modification dates
        // similar to `make` before actually building anything... however,
        // on unix, where make is doing this for us, we also set ourself
        // dependant on the xmlpktgen binary itself, so if it changes, we
        // will rebuild all the xmlp files... so this check is not adequate.
        // It would be better to have this code preform the same check, but
        // for now, I dont feel like writting it, so I just exclude this check
        // on unix, and let make do its job.
        struct stat xml_stat;
        if( stat( xml.c_str(), &xml_stat ) < 0 )
        {
            fprintf( stderr, "Error: Unable to stat '%s'.\n", xml.c_str() );

            return -1;
        }

        struct stat h_stat, cpp_stat;
        if(    !( stat( h.c_str(), &h_stat ) < 0 )
            && !( stat( cpp.c_str(), &cpp_stat ) < 0 ) )
        {
            //all three files exist, check times.
            if(    xml_stat.st_mtime < h_stat.st_mtime
                && xml_stat.st_mtime < cpp_stat.st_mtime )
            {
                printf( "%s is older than %s and %s, not rebuilding.\n", xml.c_str(), h.c_str(), cpp.c_str() );

                continue;
            }
        }
#endif

        if( !gen.LoadFile( xml.c_str() ) )
            return -1;

        if( !gen.GenPackets( h.c_str(), cpp.c_str() ) )
            return -1;
    }

    return 0;
}





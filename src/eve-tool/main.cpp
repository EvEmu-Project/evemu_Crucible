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

#include "Commands.h"

static volatile bool RunLoops = true;

//user input:
static Mutex MInputQueue;
static std::queue<char*> InputQueue; //consumer must free() these strings.

static ThreadReturnType UserInputThread( void* data );
void ProcessInput( char* input );
void CatchSignal( int sig_num );
void SetupSignals();

void LoadScript( const char* filename );
void LoadScript( FILE* f );

int main( int argc, char* argv[] )
{
    // Load server log settings ( will be removed )
    if( !load_log_settings( "log.ini" ) )
        sLog.Warning( "init", "Unable to read %s (this file is optional)", "log.ini" );
    else
        sLog.Success( "init", "Log settings loaded from %s", "log.ini" );

    //start the "input" thread
#ifdef WIN32
    _beginthread( UserInputThread, 0, NULL );
#else
    pthread_t thread;
    pthread_create( &thread, NULL, UserInputThread, NULL );
#endif

    //skip first argument (launch path), we don't need it
    --argc;
    ++argv;

    for(; argc > 0; --argc, ++argv)
        LoadScript( *argv );

    SetupSignals();

    /*
     * THE MAIN LOOP
     *
     * Everything except IO should happen in this loop, in this thread context.
     *
     */
    const uint32 LOOP_DELAY = 10; // delay 10 ms
    uint32 start, etime;

    while( RunLoops )
    {
        start = GetTickCount();

        Timer::SetCurrentTime();

        //process input:
        MInputQueue.lock();
        while( !InputQueue.empty() )
        {
            char* str = InputQueue.front();
            InputQueue.pop();
            MInputQueue.unlock();

            ProcessInput( str );
            free( str );

            MInputQueue.lock();
        }
        MInputQueue.unlock();

        etime = GetTickCount() - start;
        if( LOOP_DELAY > etime )
            Sleep( LOOP_DELAY - etime );
    }

    sLog.Log( "shutdown", "Main loop stopped." );

    return 0;
}

static ThreadReturnType UserInputThread( void *data )
{
    LoadScript( stdin );

    THREAD_RETURN( 0 );
}

void ProcessInput( char* input )
{
    const Seperator sep( input );
    
    if( 0 == strcasecmp( sep.arg[0], "exit" ) )
    {
        RunLoops = false;
        return;
    }
    else if( 0 == strcasecmp( sep.arg[0], "script" ) )
    {
        LoadScript( sep.arg[1] );
        return;
    }

    for( size_t i = 0; i < EVETOOL_COMMAND_COUNT; ++i )
    {
        const EVEToolCommand& cmd = EVETOOL_COMMANDS[ i ];

        if( 0 == strcasecmp( sep.arg[0], cmd.name ) )
        {
            ( *cmd.callback )( sep );
            return;
        }
    }

    sLog.Error( "input", "Unknown command '%s'.", sep.arg[0] );
}

void SetupSignals()
{
    signal( SIGINT, CatchSignal );
    signal( SIGTERM, CatchSignal );
    signal( SIGABRT, CatchSignal );
#ifdef WIN32
    signal( SIGBREAK, CatchSignal );
    signal( SIGABRT_COMPAT, CatchSignal );
#else
    signal( SIGHUP, CatchSignal );
#endif
}

void CatchSignal( int sig_num )
{
    sLog.Log( "Signal system", "Caught signal: %d.", sig_num );

    RunLoops = false;

    //do this to get the input thread to die off gracefully.
    fclose( stdin );
}

void LoadScript( const char* filename )
{
    FILE* script = fopen( filename, "r" );
    if( script == NULL )
    {
        sLog.Error( "script", "Unable to open script '%s'.", filename );
        return;
    }

    sLog.Log( "script", "Queuing commands from script '%s'.", filename );
    LoadScript( script );

    if( feof( script ) )
        sLog.Success( "script", "Load of script '%s' successfully completed.", filename );
    else
        sLog.Error( "script", "Error occured while reading script '%s'.", filename );

    fclose( script );
}

void LoadScript( FILE* f )
{
    std::string cmd;

    while( RunLoops )
    {
        char input = fgetc( f );

        if( '\n' == input || EOF == input )
        {
            if( !cmd.empty() )
            {
                LockMutex lock( &MInputQueue );
                InputQueue.push( strdup( cmd.c_str() ) );
            }

            cmd.clear();
            if( EOF == input )
                break;
        }
        else
            cmd += input;
    }
}





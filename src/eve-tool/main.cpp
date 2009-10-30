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

#include "common.h"

// MySQL header
#include <mysql.h>

// dep headers
#include "utils/Base64.h"

// common headers
#include "log/logsys.h"
#include "log/LogNew.h"

#include "threading/Mutex.h"

#include "utils/Buffer.h"
#include "utils/DirWalker.h"
#include "utils/misc.h"
#include "utils/seperator.h"
#include "utils/timer.h"
#include "utils/utils_string.h"
#include "utils/utils_time.h"

// eve-common headers
#include "cache/CachedObjectMgr.h"

#include "database/RowsetReader.h"
#include "database/RowsetToSQL.h"

#include "destiny/DestinyBinDump.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalStringTable.h"
#include "marshal/EVEUnmarshal.h"

#include "network/packet_types.h"

#include "packets/General.h"

#include "python/classes/DBRowDescriptor.h"
#include "python/classes/CRowSet.h"

#include "python/PyRep.h"
#include "python/PyVisitor.h"

#include "utils/EVEUtils.h"

// eve-tool headers
#include "TriFile.h"

#ifndef NO_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif /* !NO_READLINE */

// global log hook.
NewLog* pLog;
MarshalStringTable* pMarshalStringTable;

void CatchSignal(int sig_num);

static volatile bool RunLoops = true;
//user input:
static Mutex MInputQueue;
static std::queue<char *> InputQueue;	//consumer must free() these strings.

static ThreadReturnType UserInputThread(void *data);
void ProcessInput(char *input);
void LoadScript(const char *filename);

int main(int argc, char *argv[])
{
    /* logging system */
    pLog = new NewLog;
    /* marshal string table */
    pMarshalStringTable = new MarshalStringTable;

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(CCLIENT__INIT_ERR, "Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(CCLIENT__INIT_ERR, "Could not set signal handler");
		return 1;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		_log(CCLIENT__INIT_ERR, "Could not set signal handler");
		return 1;
	}
	#endif

	if(!load_log_settings("log.ini"))
	{
		_log(CCLIENT__INIT, "Warning: Unable to read %s", "log.ini");
	}
	else
	{
		_log(CCLIENT__INIT, "Log settings loaded from %s", "log.ini");
	}

	//start the "input" thread
#ifdef WIN32
	_beginthread(UserInputThread, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, UserInputThread, NULL);
#endif

	//skip first argument (launch path), we don't need it
	argc--;
	argv++;

	while(argc > 0) {
		LoadScript(argv[0]);
		argc--;
		argv++;
	}
	
	/*
	 * THE MAIN LOOP
	 *
	 * Everything except IO should happen in this loop, in this thread context.
	 *
	 */
	while(RunLoops) {
		Timer::SetCurrentTime();
		
		//check for timeouts in other threads
		//timeout_manager.CheckTimeouts();
		
		//process input:
		MInputQueue.lock();
		while(!InputQueue.empty()) {
			char *str = InputQueue.front();
			InputQueue.pop();
			MInputQueue.unlock();
			ProcessInput(str);
			free(str);
			MInputQueue.lock();
		}
		MInputQueue.unlock();
		
		/*if (numclients == 0) {
			Sleep(50);
			continue;
		}*/
		Sleep(3);	//this should be a parameter
	}
	_log(CCLIENT__SHUTDOWN,"main loop stopped");

#ifndef NO_READLINE
	rl_deprep_terminal();
#endif
	return(0);
}

static ThreadReturnType UserInputThread(void *data)
{
#ifndef NO_READLINE
	stifle_history(100);
	
	while(RunLoops) {
		char *input = readline("action?> ");
		if(input == NULL)
			continue;	//detect errors???
		
		//else, we got a string
		add_history(input);
		
		MInputQueue.lock();
		InputQueue.push(input);
		MInputQueue.unlock();
	}
	
#else	//NO READLINE
	char* input_buffer = new char[10240];
	while( RunLoops )
    {
		if( fgets( input_buffer, 10239, stdin ) == NULL )
        {
			if( feof( stdin ) )
            {
			    RunLoops = false;
			    break;
			}
            else
				_log(CCLIENT__ERROR, "Failed to read a line");
		}
		
		//else, we got a string
		MInputQueue.lock();
		InputQueue.push(strdup(input_buffer));
		MInputQueue.unlock();
	}
	SafeDelete( input_buffer );
#endif
	
	THREAD_RETURN(0);
}

void CatchSignal(int sig_num) {
	_log(CCLIENT__SHUTDOWN,"Caught signal %d",sig_num);
//	if(zoneserver_list.worldclock.saveFile(WorldConfig::get()->EQTimeFile.c_str())==false)
//		_log(CLIENT__SHUTDOWN,"Failed to save time file.");
	RunLoops = false;

	//do this to get the input thread to die off gracefully.
	fclose(stdin);
}

void TimeToString(const char *time);
void PrintTimeNow();
void ObjectToSQL(const Seperator &command);
void TriToOBJ(const Seperator &command);
void UnmarshalLogText(const Seperator &command);
void DestinyDumpLogText(const Seperator &command);
void TestMarshal();

void ProcessInput(char *input) {
	//strip newlines
	char *tmp = input;
	while(*tmp != '\0') {
		if(*tmp == '\r' || *tmp == '\n') {
			*tmp = '\0';
			break;
		}
		++tmp;
	}
	
	Seperator sep(input);
	
	if(strcasecmp(sep.arg[0], "exit") == 0)
		RunLoops = false;
    else if(strcasecmp(sep.arg[0], "script") == 0)
		LoadScript(sep.arg[1]);
    else if(strcasecmp(sep.arg[0], "time") == 0)
		TimeToString(sep.arg[1]);
    else if(strcasecmp(sep.arg[0], "now") == 0)
		PrintTimeNow();
    else if(strcasecmp(sep.arg[0], "obj2sql") == 0)
		ObjectToSQL(sep);
    else if(strcasecmp(sep.arg[0], "tri2obj") == 0)
		TriToOBJ(sep);
    else if(strcasecmp(sep.arg[0], "unmarshal") == 0)
		UnmarshalLogText(sep);
    else if(strcasecmp(sep.arg[0], "destinydump") == 0)
		DestinyDumpLogText(sep);
    else if(strcasecmp(sep.arg[0], "mtest") == 0)
		TestMarshal();
    else
		printf("Unknown command '%s'\n", sep.arg[0]);
}

void TimeToString(const char *time) {
	uint64 t;
#ifndef WIN32   // Make GCC happy.
    sscanf( time, I64u, (unsigned long long int*)&t );
#else
    sscanf( time, I64u, &t );
#endif
	printf("\n"I64u" is %s\n", t, Win32TimeToString(t).c_str());
}

void PrintTimeNow()
{
	printf( "Now in Win32 time: "I64u"\n", Win32TimeNow() );
}

void ObjectToSQL(const Seperator &command)
{
	if(command.argnum != 4) {
		printf("Usage: obj2sql [cache_file] [table_name] [key_field] [file_name]\n");
		return;
	}

    std::string abs_fname( "../data/cache/" );
	abs_fname += command.arg[1];
	abs_fname += ".cache";
	printf("Converting cached object %s:\n", abs_fname.c_str());

	CachedObjectMgr mgr;
	PyCachedObjectDecoder *obj = mgr.LoadCachedObject(abs_fname.c_str(), command.arg[1]);
	if(obj == NULL) {
		_log(CLIENT__ERROR, "Unable to load or decode '%s'!", abs_fname.c_str());
		return;
	}

	obj->cache->DecodeData();
	if(obj->cache->decoded() == NULL) {
		_log(CLIENT__ERROR, "Unable to load or decode body of '%s'!", abs_fname.c_str());
		delete obj;
		return;
	}

	FILE *out = fopen(command.arg[4], "w");
	if(out == NULL) {
		_log(CLIENT__ERROR, "Unable to open output file '%s'", command.arg[4]);
		delete obj;
		return;
	}

	bool success = false;
	if(obj->cache->decoded()->IsObject()) {
		util_Rowset rowset;
		if( !rowset.Decode( obj->cache->decoded() ) )
        {
			_log(CLIENT__ERROR, "Unable to load a rowset from the object body!");
			delete obj;
			return;
		}
	
		RowsetReader reader(&rowset);
		success = ReaderToSQL<RowsetReader>(command.arg[2], command.arg[3], out, reader);
	} else if(obj->cache->decoded()->IsTuple()) {
		util_Tupleset rowset;
		if( !rowset.Decode( obj->cache->decoded() ) )
        {
			_log(CLIENT__ERROR, "Unable to load a tupleset from the object body!");
			delete obj;
			return;
		}

        TuplesetReader reader(&rowset);
		success = ReaderToSQL<TuplesetReader>(command.arg[2], command.arg[3], out, reader);
	} else {
		_log(CLIENT__ERROR, "Unknown cache body type: %s", obj->cache->decoded()->TypeString());
	}

	fclose(out);
	delete obj;
	if(success)
		printf("Success.\n");
}

void TriToOBJ(const Seperator &command) {
	if(command.argnum != 3) {
		printf("Usage: tri2obj [trifile] [objout] [mtlout]\n");
		return;
	}

	TriExporter::TriFile f;
	if(!f.LoadFile(command.arg[1])) {
		printf("Failed to load TRI file.\n");
		return;
	}

	f.DumpHeaders();
	f.ExportObj(command.arg[2], command.arg[3]);
	printf("%s - %s - written.\n", command.arg[2], command.arg[3]);
}

void UnmarshalLogText(const Seperator &command)
{
    Buffer data;
	if( !py_decode_escape( command.argplus[1], data ) )
    {
		sLog.Error( "unmarshal", "Failed to decode string into binary." );
        return;
	}

	PyRep* r = InflateUnmarshal( data );
	if( r == NULL )
		sLog.Error( "unmarshal", "Failed to unmarshal binary." );
    else
    {
        sLog.Success( "unmarshal", "Result:" );

		r->Dump( stdout, "    " );
	}
}

void TestMarshal()
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

	PyPackedRow& row = rs->NewRow();
	row.SetField( "historyDate", new PyLong( Win32TimeNow() ) );
	row.SetField( "lowPrice", new PyLong( 18000 ) );
	row.SetField( "highPrice", new PyLong( 19000 ) );
	row.SetField( "avgPrice", new PyLong( 18400 ) );
	row.SetField( "volume", new PyLong( 5463586 ) );
	row.SetField( "orders", new PyInt( 254 ) );

    sLog.Log( "mtest", "Marshaling..." );

    Buffer* marshaled = MarshalDeflate( rs );
    PyDecRef( rs );
    if( marshaled == NULL )
    {
        sLog.Error( "mtest", "Failed to marshal Python object." );
        return;
    }

    sLog.Log( "mtest", "Unmarshaling..." );

	PyRep* rep = InflateUnmarshal( *marshaled );
	SafeDelete( marshaled );
    if( rep == NULL )
        sLog.Error( "mtest", "Failed to unmarshal Python object." );
    else
    {
        sLog.Success( "mtest", "Final:" );
	    rep->Dump( stdout, "    " );

        PyDecRef( rep );
    }
}

void LoadScript(const char *filename) {
	FILE *script = fopen(filename, "r");
	if(script == NULL) {
		_log(CCLIENT__INIT_ERR, "Unable to open script '%s'", filename);
	} else {
		_log(CCLIENT__INIT, "Queuing commands from script '%s'", filename);
		std::string cmd;

		char input = fgetc(script);
		while(input != EOF) {
			if(input == '\n') {
				InputQueue.push(strdup(cmd.c_str()));
				cmd.clear();
			} else
				cmd += input;
			input = fgetc(script);
		}
		if(feof(script)) {
			InputQueue.push(strdup(cmd.c_str()));
			_log(CCLIENT__INIT, "Load of script '%s' successfully completed.", filename);
		} else
			_log(CCLIENT__INIT_ERR, "Error occured while reading script '%s'.", filename);

		fclose(script);
	}
}

void DestinyDumpLogText( const Seperator &command )
{
    Buffer data;
    if( !py_decode_escape( command.argplus[1], data ) )
		return;

	Destiny::DumpUpdate( DESTINY__MESSAGE, &data[0], data.size() );
}







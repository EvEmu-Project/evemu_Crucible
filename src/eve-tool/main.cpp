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

#include "utils/DirWalker.h"
#include "utils/misc.h"
#include "utils/seperator.h"
#include "utils/timer.h"
#include "utils/utils_time.h"

// eve-common headers
#include "cache/CachedObjectMgr.h"

#include "database/RowsetReader.h"
#include "database/RowsetToSQL.h"

#include "destiny/DestinyBinDump.h"

#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalStringTable.h"
#include "marshal/EVEUnmarshal.h"
#include "marshal/EVEZeroCompress.h"

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

#define CACHE_DIR "../data/cache"

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


int main(int argc, char *argv[]) {
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

void TestCache() {
	PyString* str = new PyString("charCreationInfo.departments");

	uint32 len;
	uint8* data = Marshal( str, len );
    PyDecRef( str );
	
	std::string into;
    // Skip first 5 bytes (marshal header)
	Base64::encode( &data[5], len-5, into, false );
    SafeDeleteArray( data );

	printf("Marshaled, base64'd string: '%s'\n", into.c_str());
}


static ThreadReturnType UserInputThread(void *data) {
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
	char *input_buffer = new char[10240];
	while(RunLoops) {
		if(fgets(input_buffer, 10239, stdin) == NULL) {
			if(feof(stdin)) {
				//do nothing...
			} else {
				_log(CCLIENT__ERROR, "Failed to read a line");
			}
			RunLoops = false;
			break;
		}
		
		//else, we got a string
		MInputQueue.lock();
		InputQueue.push(strdup(input_buffer));
		MInputQueue.unlock();
	}
	delete input_buffer;
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

void ListCache(const char *filter);
void CatCacheObject(const char *file);
void CatCacheCall(const char *file);
void CatAllObjects(const char *file);
void TimeToString(const char *time);
void ObjectToSQL(const Seperator &command);
void TriToOBJ(const Seperator &command);
void UnmarshalLogText(const Seperator &command);
void DestinyDumpLogText(const Seperator &command);
void PrintTimeNow();
void TestMarshal();
void TestZeroCompress();

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
	
	if(strcasecmp(sep.arg[0], "exit") == 0) {
		RunLoops = false;
		return;
	} else if(strcasecmp(sep.arg[0], "ls") == 0) {
		ListCache(sep.arg[1]);
		return;
	} else if(strcasecmp(sep.arg[0], "catc") == 0) {
		CatCacheCall(sep.arg[1]);
		return;
	} else if(strcasecmp(sep.arg[0], "cato") == 0) {
		CatCacheObject(sep.arg[1]);
		return;
	} else if(strcasecmp(sep.arg[0], "obj2sql") == 0) {
		ObjectToSQL(sep);
		return;
	} else if(strcasecmp(sep.arg[0], "catall") == 0) {
		CatAllObjects(sep.arg[1]);
		return;
	} else if(strcasecmp(sep.arg[0], "time") == 0) {
		TimeToString(sep.arg[1]);
		return;
	} else if(strcasecmp(sep.arg[0], "now") == 0) {
		PrintTimeNow();
		return;
	} else if(strcasecmp(sep.arg[0], "tri2obj") == 0) {
		TriToOBJ(sep);
		return;
	} else if(strcasecmp(sep.arg[0], "mtest") == 0) {
		TestMarshal();
		return;
	} else if(strcasecmp(sep.arg[0], "unmarshal") == 0) {
		UnmarshalLogText(sep);
		return;
	} else if(strcasecmp(sep.arg[0], "script") == 0) {
		LoadScript(sep.arg[1]);
		return;
	} else if(strcasecmp(sep.arg[0], "destinydump") == 0) {
		DestinyDumpLogText(sep);
		return;
	} else if(strcasecmp(sep.arg[0], "zctest") == 0) {
		TestZeroCompress();
		return;
	} else {
		printf("Unknown command '%s'\n", sep.arg[0]);
	}
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

void PrintTimeNow() {
	printf("\nNow in Win32 time: "I64u"\n", Win32TimeNow());
}

void ListCache(const char *in_filter) {
	std::string filter;
	if(in_filter != NULL && *in_filter == ' ') {
		in_filter++;
		filter = in_filter;
	}
	std::vector<std::string> files;
	if(!DirWalker::ListDirectory(CACHE_DIR, files)) {
		printf("Unable to read cache dir\n");
		return;
	}

	printf("Cache files:\n");
	std::vector<std::string>::iterator cur, end;
	cur = files.begin();
	end = files.end();
	for(; cur != end; cur++) {
		std::string s(*cur);
		if(s.find(".cache") == std::string::npos) {
			if(!filter.empty())
				printf("file '%s' is not a cache file\n", s.c_str());
			continue;
		}
		std::string fname = s;

		
		s.erase(s.find('.'));
		s.append("=======");
		uint8 *b = new uint8[s.length()];
		size_t len = s.length();
		Base64::decode(s, b, len);
		
		//impossible that this is a stream.
		if(len < 4 || b[0] != '~')
			continue;
		
		PyRep *rep = InflateUnmarshal(b, uint32(len-3));

		std::string hexed;
		char buf[10];
		size_t r;
		for(r = 0; r < len; r++) {
			if(b[r] < ' ' || b[r] > '~') {
				snprintf(buf, 9, "\\x%02x", b[r]);
			} else {
				buf[0] = b[r];
				buf[1] = '\0';
			}
			hexed += buf;
		}
		
		if(!filter.empty()) {
			if(strstr(hexed.c_str(), filter.c_str()) == NULL) {
				delete rep;
				continue;
			}
		}
		
		printf("file: %s\n", fname.c_str());
		printf("  --> %s\n", hexed.c_str());
		if(rep != NULL)
			rep->Dump(stdout, "    --> ");
	}
	printf("\n");
}


void CatAllObjects(const char *in_filter) {
	std::string filter;
	if(in_filter != NULL && *in_filter == ' ') {
		in_filter++;
		filter = in_filter;
	}
	std::vector<std::string> files;
	if(!DirWalker::ListDirectory(CACHE_DIR, files)) {
		printf("Unable to read cache dir\n");
		return;
	}

	printf("Cache files:\n");
	std::vector<std::string>::iterator cur, end;
	cur = files.begin();
	end = files.end();
	for(; cur != end; cur++) {
		std::string s(*cur);
		if(s.find(".cache") == std::string::npos) {
			if(!filter.empty())
				printf("file '%s' is not a cache file\n", s.c_str());
			continue;
		}
		std::string fname = s;

		
		s.erase(s.find('.'));

		CatCacheObject(s.c_str());
	}
}

void CatCacheObject(const char *file) {
	std::string abs_fname(CACHE_DIR "/");
	abs_fname += file;
	abs_fname += ".cache";
	printf("Dumping cached object %s:\n", abs_fname.c_str());

	CachedObjectMgr mgr;
	PyCachedObjectDecoder *obj = mgr.LoadCachedObject(abs_fname.c_str(), file);
	if(obj == NULL) {
		_log(CLIENT__ERROR, "Unable to load or decode '%s'!", abs_fname.c_str());
		return;
	}

	printf("Cached Object Data:\n");
	obj->Dump(stdout, "    ", false);

	printf("Cached Object Contents:\n");
	SubStreamDecoder ssd;
	obj->cache->visit(&ssd);
	obj->cache->Dump(stdout, "    ");
}

void CatCacheCall(const char *file) {
	std::string abs_fname(CACHE_DIR "/");
	abs_fname += file;
	abs_fname += ".cache";
	printf("Dumping cached object %s:\n", abs_fname.c_str());

	CachedObjectMgr mgr;
	PyCachedCall *obj = mgr.LoadCachedCall(abs_fname.c_str(), file);
	if(obj == NULL) {
		_log(CLIENT__ERROR, "Unable to load or decode '%s'!", abs_fname.c_str());
		return;
	}

	printf("Cached Call Data:\n");
	obj->Dump(stdout, "    ", false);

	printf("Cached Call Results:\n");
	SubStreamDecoder ssd;
	obj->result->visit(&ssd);
	obj->result->Dump(stdout, "    ");
	
	
	
/*  uint32 file_length = GetFileLength(abs_fname.c_str());
	if(file_length == 0) {
		_log(CLIENT__ERROR, "Unable to stat cache file '%s'", abs_fname.c_str());
		return;
	}
	
	FILE *f = fopen(abs_fname.c_str(), "rb");
	if(f == NULL) {
		_log(CLIENT__ERROR, "Unable to open cache file '%s': %s", abs_fname.c_str(), strerror(errno));
		return;
	}
	
	uint8 *b = new uint8[file_length+10];
	int32 len = fread(b, 1, file_length+10, f);
	fclose(f);

	PySubStream *into = new PySubStream();
	into->data = b;
	into->length = len;

	into->DecodeData();
	if(into->decoded == NULL) {
		_log(CLIENT__ERROR, "Unable to unmarshal cache file '%s'", abs_fname.c_str());
		_hex(CLIENT__ERROR, into->data, into->length);
		return;
	}
	SubStreamDecoder ssd;
	into->decoded->visit(&ssd);
	into->Dump(stdout, "    ");*/
}

void ObjectToSQL(const Seperator &command) {
	if(command.argnum != 4) {
		printf("Usage: obj2sql [cache_file] [table_name] [key_field] [file_name]\n");
		return;
	}

	
	std::string abs_fname(CACHE_DIR "/");
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

//based on PyString_DecodeEscape from python.
static bool PyString_DecodeEscape(const char *s, vector<uint8> &result)
{
	int c;
	const char *end;
    /*
	char *p, *buf;
	PyObject *v;
	int newlen = recode_encoding ? 4*len:len;
	v = PyString_FromStringAndSize((char *)NULL, newlen);
	if (v == NULL)
		return NULL;
	p = buf = PyString_AsString(v);
	*/
	int len = (int)strlen(s);
	end = s + len;
	while (s < end) {
		if (*s != '\\') {
			result.push_back(*s++);
			continue;
		}
		s++;
        if (s==end) {
            //ended with a \ char
			return false;
		}
		switch (*s++) {
		/* XXX This assumes ASCII! */
		case '\n': break;	//?
		case '\\': result.push_back('\\'); break;
		case '\'': result.push_back('\''); break;
		case '\"': result.push_back('\"'); break;
		case 'b':  result.push_back('\b'); break;
		case 'f':  result.push_back('\014'); break; /* FF */
		case 't':  result.push_back('\t'); break;
		case 'n':  result.push_back('\n'); break;
		case 'r':  result.push_back('\r'); break;
		case 'v':  result.push_back('\013'); break; /* VT */
		case 'a':  result.push_back('\007'); break; /* BEL, not classic C */
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c = s[-1] - '0';
			if ('0' <= *s && *s <= '7') {
				c = (c<<3) + *s++ - '0';
				if ('0' <= *s && *s <= '7')
					c = (c<<3) + *s++ - '0';
			}
			 result.push_back(c);
			break;
		case 'x':
			if (isxdigit(s[0]) 
			    && isxdigit(s[1])) {
				unsigned int x = 0;
				c = *s;
				s++;
				if (isdigit(c))
					x = c - '0';
				else if (islower(c))
					x = 10 + c - 'a';
				else
					x = 10 + c - 'A';
				x = x << 4;
				c = *s;
				s++;
				if (isdigit(c))
					x += c - '0';
				else if (islower(c))
					x += 10 + c - 'a';
				else
					x += 10 + c - 'A';
				 result.push_back(x);
				break;
			}
			//"invalid \\x escape");
			return false;
		default:
			return false;
		}
	}
	return true;
}



void UnmarshalLogText(const Seperator &command) {
	vector<uint8> result;
	if(!PyString_DecodeEscape(command.argplus[1], result)) {
		printf("Failed to decode string... trying with what we did get (%lu bytes).\n", result.size());
	}

	_log(NET__UNMARSHAL_TRACE, "Decoded %lu bytes:", result.size());
	_hex(NET__UNMARSHAL_BUFHEX, &result[0], uint32(result.size()));
	
	PyRep *r = InflateUnmarshal(&result[0], uint32(result.size()));
	if(r == NULL) {
		printf("Failed to unmarshal.\n");
	} else {
		//decode substreams.
		SubStreamDecoder ssd;
		r->visit(&ssd);
		printf("Unmarshaled rep:\n");
		r->Dump(stdout, "    ");
	}
}

void TestMarshal() {
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

	_log(COMMON__MESSAGE, "Marshaling...");
    uint32 len;
	uint8* marshaled = MarshalDeflate( rs, len );
    PyDecRef( rs );

    if( marshaled == NULL )
    {
        sLog.Error( "mtest", "Failed to marshal Python object." );
        return;
    }

	_log(COMMON__MESSAGE, "Unmarshaling...");
	PyRep* rep = InflateUnmarshal( marshaled, len );
	SafeDeleteArray( marshaled );

    if(rep == NULL)
    {
        sLog.Error( "mtest", "Failed to unmarshal Python object." );
        return;
    }

	rep->Dump( stdout, " Final: " );

    PyDecRef( rep );
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

void DestinyDumpLogText(const Seperator &command) {
	std::vector<uint8> result;

	if(!PyString_DecodeEscape(command.argplus[1], result))
		return;

	Destiny::DumpUpdate(DESTINY__MESSAGE, &result[0], uint32(result.size()));
}

void TestZeroCompress() {
	uint8 input[] = {
		0x00, 0xC3, 0xF5, 0x28, 0x5C, 0x8F, 0x16, 0x85, 0x40, 0x90, 0x38, 0x0C, 0x23, 0x20, 0x27, 0xC9, 0x01, 0xA7, 0x14, 0xA7, 0x15, 0xA7,
		0xFF, 0xA7, 0x05, 0xA7, 0x14, 0xA7, 0x01, 0xA7, 0x01, 0x81, 0x31, 0x96, 0x93, 0x03, 0x90, 0x96, 0x98, 0x84, 0xFD, 0xC8, 0xC9, 0x01,
		0x07, 0x01
	};

	_log(COMMON__MESSAGE, "Original: %lu bytes", sizeof(input));
	_hex(COMMON__MESSAGE, input, sizeof(input));

	std::vector<uint8> unpacked;
	UnpackZeroCompressed(input, sizeof(input), unpacked);

	_log(COMMON__MESSAGE, "Unpacked: %lu bytes", unpacked.size());
	_hex(COMMON__MESSAGE, &unpacked[0], (uint32)unpacked.size());

	std::vector<uint8> repacked;
	PackZeroCompressed(&unpacked[0], unpacked.size(), repacked);
	
	_log(COMMON__MESSAGE, "Re-packed: %lu bytes", repacked.size());
	for(size_t r = 0; r < repacked.size(); r++) {
		if(r % 16 == 0 && r != 0)
			printf("\n");

		if(repacked[r] != input[r])
			printf("(%02X) ", repacked[r]);
		else
			printf("%02X ", repacked[r]);
	}
	printf("\n");
}







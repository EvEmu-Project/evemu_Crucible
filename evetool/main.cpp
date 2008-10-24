/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#define CACHE_DIR "../data/cache"


#include "../common/common.h"

#include "../common/logsys.h"
#include "../common/EVEMarshal.h"
#include "../common/EVEUnmarshal.h"
#include "../common/timer.h"
#include "../common/Mutex.h"
#include "../common/PyRep.h"
#include "../common/Base64.h"
#include "../common/DirWalker.h"
#include "../common/misc.h"
#include "../common/PyVisitor.h"
#include "../common/EVEUtils.h"
#include "../common/CachedObjectMgr.h"
#include "../common/seperator.h"
#include "../common/RowsetReader.h"
#include "../common/RowsetToSQL.h"
#include "../common/packet_types.h"
#include "../common/DestinyBinDump.h"
#include "../packets/General.h"
#include "TriFile.h"
#include <queue>
#ifndef WIN32
#include <sys/signal.h>
#endif
#include <signal.h>
#include <errno.h>

#ifndef NO_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif



void CatchSignal(int sig_num);

static volatile bool RunLoops = true;
//user input:
static Mutex MInputQueue;
static std::queue<char *> InputQueue;	//consumer must free() these strings.


static ThreadReturnType UserInputThread(void *data);
void ProcessInput(char *input);
void LoadScript(const char *filename);


int main(int argc, char *argv[]) {
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
		_log(CCLIENT__INIT, "Warning: Unable to read %s", "log.ini");
	else
		_log(CCLIENT__INIT, "Log settings loaded from %s", "log.ini");

	if(!PyRepString::LoadStringFile("../data/strings.txt")) {
		_log(CCLIENT__INIT_ERR, "Unable to open ../data/strings.txt, i need it to decode string table elements!");
		return(1);
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
	PyRepString *s = new PyRepString("charCreationInfo.departments");

	uint32 len = 0;
	byte *data = Marshal(s, len, false);
	
	std::string into;
	Base64::encode(data, len, into, false);

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
	} else {
		printf("Unknown command '%s'\n", sep.arg[0]);
	}
}

void TimeToString(const char *time) {
	uint64 t;
	t = strtoull(time, NULL, 10);
	printf("\n%llu is %s\n", t, Win32TimeToString(t).c_str());
}

void PrintTimeNow() {
	printf("\nNow in Win32 time: %llu\n", Win32TimeNow());
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
		byte *b = new byte[s.length()];
		size_t len = s.length();
		Base64::decode(s, b, len);
		
		//impossible that this is a stream.
		if(len < 4 || b[0] != '~')
			continue;
		
		PyRep *rep = InflateAndUnmarshal(b, uint32(len-3));

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
	PyCachedObjectDecoder *obj = mgr.LoadCachedFile(abs_fname.c_str(), file);
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
	
	byte *b = new byte[file_length+10];
	int32 len = fread(b, 1, file_length+10, f);
	fclose(f);

	PyRepSubStream *into = new PyRepSubStream();
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

//we template this function to avoid duplicating the code for these two
//reader types which present the same interface, but are not polymorphic
template <class T>
bool _ObjectToSQLHelper(const Seperator &command, FILE *out, T &reader) {
	int col;
	int cc = reader.ColumnCount();
	int key_col = -1;
	for(col = 0; col < cc; col++) {
		if(strcmp(command.arg[3], reader.ColumnName(col)) == 0) {
			key_col = col;
			break;
		}
	}
	if(key_col == -1) {
		_log(CLIENT__ERROR, "Unable to find key column '%s'", command.arg[3]);
		return(false);
	}
	
	std::string field_list;
	printf("Columns:\n");
	fprintf(out, "CREATE TABLE %s (\n", command.arg[2]);
	for(col = 0; col < cc; col++) {
		if(col != 0)
			fprintf(out, ",\n");
		if(col == key_col)
			fprintf(out, "  %s INT NOT NULL PRIMARY KEY", reader.ColumnName(col));
		else	//hack it to TEXT for simplicity for now
			fprintf(out, "  %s TEXT", reader.ColumnName(col));
		
		if(col != 0)
			field_list += ",";
		field_list += reader.ColumnName(col);
		
		printf(" %s |", reader.ColumnName(col));
	}
	printf("\n");
	fprintf(out, "\n);\n\n\n--- data ---\n\n");

	typedef typename T::iterator T_iterator;
	T_iterator cur, end;
	cur = reader.begin();
	end = reader.end();
	for(; cur != end; ++cur) {
		fprintf(out, "INSERT INTO %s (%s) VALUES(", command.arg[2], field_list.c_str());
		for(col = 0; col < cc; col++) {
			if(col != 0)
				fprintf(out, ",");
			if(cur.IsNone(col)) {
				fprintf(out, "NULL");
			} else {
				std::string colval = cur.GetAsString(col);
				EscapeStringSequence(colval, "'", "\\'");
				fprintf(out, "'%s'", colval.c_str());
			}
		}
		fprintf(out, ");\n");
	}

	return(true);
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
	PyCachedObjectDecoder *obj = mgr.LoadCachedFile(abs_fname.c_str(), command.arg[1]);
	if(obj == NULL) {
		_log(CLIENT__ERROR, "Unable to load or decode '%s'!", abs_fname.c_str());
		return;
	}

	obj->cache->DecodeData();
	if(obj->cache->decoded == NULL) {
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
	if(obj->cache->decoded->CheckType(PyRep::Object)) {
		util_Rowset rowset;
		if(!rowset.Decode(&obj->cache->decoded)) {
			_log(CLIENT__ERROR, "Unable to load a rowset from the object body!");
			delete obj;
			return;
		}
	
		RowsetReader reader(&rowset);
		success = ReaderToSQL<RowsetReader>(command.arg[2], command.arg[3], out, reader);
	} else if(obj->cache->decoded->CheckType(PyRep::Tuple)) {
		util_Tupleset rowset;
		if(!rowset.Decode(&obj->cache->decoded)) {
			_log(CLIENT__ERROR, "Unable to load a tupleset from the object body!");
			delete obj;
			return;
		}
	
		TuplesetReader reader(&rowset);
		success = ReaderToSQL<TuplesetReader>(command.arg[2], command.arg[3], out, reader);
	} else {
		_log(CLIENT__ERROR, "Unknown cache body type: %s", obj->cache->decoded->TypeString());
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
static bool PyString_DecodeEscape(const char *s, vector<byte> &result)
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
			return(false);
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
			return(false);
		default:
			return(false);
		}
	}
	return(true);
}



void UnmarshalLogText(const Seperator &command) {
	vector<byte> result;
	if(!PyString_DecodeEscape(command.argplus[1], result)) {
		printf("Failed to decode string... trying with what we did get (%d bytes).\n", result.size());
	}

	_log(NET__UNMARSHAL_TRACE, "Decoded %lu bytes:", result.size());
	_hex(NET__UNMARSHAL_BUFHEX, &result[0], result.size());
	
	PyRep *r = InflateAndUnmarshal(&result[0], result.size());
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
	//PyRepTuple *t = new PyRepTuple(2);
	PyRepPackedObject1 *dbrowdesc = new PyRepPackedObject1("blue.DBRowDescriptor");
	dbrowdesc->args = new PyRepTuple(1);

		PyRepTuple *arg_list = new PyRepTuple(6);
		dbrowdesc->args->items[0] = arg_list;
			PyRepTuple *pair_tuple;
			int r = 0;
			
			pair_tuple = new PyRepTuple(2);
			pair_tuple->items[0] = new PyRepString("historyDate");
			pair_tuple->items[1] = new PyRepInteger(DBTYPE_FILETIME);
			arg_list->items[r++] = pair_tuple;
			
			pair_tuple = new PyRepTuple(2);
			pair_tuple->items[0] = new PyRepString("lowPrice");
			pair_tuple->items[1] = new PyRepInteger(DBTYPE_CY);
			arg_list->items[r++] = pair_tuple;
			
			pair_tuple = new PyRepTuple(2);
			pair_tuple->items[0] = new PyRepString("highPrice");
			pair_tuple->items[1] = new PyRepInteger(DBTYPE_CY);
			arg_list->items[r++] = pair_tuple;
			
			pair_tuple = new PyRepTuple(2);
			pair_tuple->items[0] = new PyRepString("avgPrice");
			pair_tuple->items[1] = new PyRepInteger(DBTYPE_CY);
			arg_list->items[r++] = pair_tuple;
			
			pair_tuple = new PyRepTuple(2);
			pair_tuple->items[0] = new PyRepString("volume");
			pair_tuple->items[1] = new PyRepInteger(DBTYPE_I8);
			arg_list->items[r++] = pair_tuple;
			
			pair_tuple = new PyRepTuple(2);
			pair_tuple->items[0] = new PyRepString("orders");
			pair_tuple->items[1] = new PyRepInteger(DBTYPE_I4);
			arg_list->items[r++] = pair_tuple;

	PyRepPackedObject2 *rs = new PyRepPackedObject2("dbutil.RowList");
	PyRepDict *d = new PyRepDict;
	rs->args2 = d;
	d->add("header", dbrowdesc);
	
	//PyRepList *l = new PyRepList();
		PyRepPackedRow *row;

		{
		static byte data[] = {
			0x00, 0x40, 0x7b, 0x30, 0xb2, 0x6c, 0xc6, 0x01, 0x68, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x38, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x01, 0xee, 0x95, 0x31, 0x00, 0x00, 0x00, 0x00, 0x2f, 0x0f, 0x00, 0x00
		};
		row = new PyRepPackedRow(dbrowdesc, false, data, sizeof(data));
		//l->add(row);
		rs->list_data.push_back(row);
		}
	//t->items[1] = l;

	uint32 mlen = 0;
	printf("Marshaling...\n");
	byte *marshaled = Marshal(rs, mlen, false);
	delete rs;
	//byte *marshaled = MarshalOnly(t, mlen);
	//delete t;
	
	printf("Unmarshaling...\n");
	PyRep *rep = InflateAndUnmarshal(marshaled, mlen);
	if(rep != NULL) {
		rep->Dump(stdout, " Final: ");
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

void DestinyDumpLogText(const Seperator &command) {
	std::vector<byte> result;

	if(!PyString_DecodeEscape(command.argplus[1], result))
		return;

	Destiny::DumpUpdate(DESTINY__MESSAGE, &result[0], result.size());
}













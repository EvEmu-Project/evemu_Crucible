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


#include "logsys.h"
#include "common.h"
#include "misc.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <string>

#ifdef WIN32
	extern int gettimeofday (timeval *tp, ...);
	#define NO_LOG_TIME	//temp disabled on windows because I dont wanna deal with it right now.
#else
#include <sys/time.h>
#endif

static FILE *logsys_log_file = NULL;

#define LOG_CATEGORY(category) #category ,
const char *log_category_names[NUMBER_OF_LOG_CATEGORIES] = {
	#include "logtypes.h"
};
std::list<std::string> memory_log;


//this array is private to this file, only a const version of it is exposed
#define LOG_TYPE(category, type, enabled, str) { enabled, LOG_ ##category, #category "__" #type, str },
static LogTypeStatus real_log_type_info[NUMBER_OF_LOG_TYPES+1] = 
{
	#include "logtypes.h"
	{ false, NUMBER_OF_LOG_CATEGORIES, "BAD TYPE", "Bad Name" }	/* dummy trailing record */
};
const LogTypeStatus *log_type_info = real_log_type_info;



void log_hex(LogType type, const void *data, unsigned long length, unsigned char padding) {
	if(!is_log_enabled(type))
		return;
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line((const char *)data,length,offset,buffer,padding);
		log_message(type, "%s", buffer);	//%s is to prevent % escapes in the ascii
	}
}

void log_phex(LogType type, const void *data, unsigned long length, unsigned char padding) {
	if(!is_log_enabled(type))
		return;
	if(length <= 1024)
		log_hex(type, data, length, padding);
	else {
		char buffer[80];
		log_hex(type, data, 1024-32, padding);
		log_message(type, " ... truncated ...");
		build_hex_line((const char *)data,length,length-16,buffer,padding);
		log_message(type, "%s", buffer);
		
	}
}

void log_message(LogType type, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_messageVA(type, fmt, args);
	va_end(args);
}

void log_messageVA(LogType type, const char *fmt, va_list args) {
	std::string message("");
#ifndef NO_LOG_TIME
	struct timeval tv;
	gettimeofday(&tv, NULL);
	char buf[32];
	strftime(buf, sizeof(buf), "%T", localtime(&tv.tv_sec));
	//printf("%s.%04d ", buf, tv.tv_usec/100);
	message = buf;
	message += '.';
	snprintf(buf, sizeof(buf), "%04d", tv.tv_usec/100); buf[sizeof(buf)-1] = '\0';
	message += buf;	
#endif

	message += "[";
	message += log_type_info[type].display_name;
	message += "] ";

	int size = vsnprintf(NULL, 0, fmt, args);
	char * msg = new char[size + 1];
	vsnprintf(msg, size + 1, fmt, args); msg[size] = '\0';
	message += msg;
	message += "\n";
	delete[] msg;

	printf(message.c_str());
	if (logsys_log_file != NULL) { fprintf(logsys_log_file, message.c_str()); fflush(logsys_log_file); /* keep the logfile updated */}
	memory_log.push_back(message);
	if (memory_log.size() > 1000) memory_log.pop_front();
	
	/*printf("[%s] ", log_type_info[type].display_name);
	vprintf(fmt, args);
	printf("\n");
	if(logsys_log_file != NULL) {
		fprintf(logsys_log_file, "[%s] ", log_type_info[type].display_name);
		vfprintf(logsys_log_file, fmt, args);
		fprintf(logsys_log_file, "\n");
	}*/
}

void log_enable(LogType t) {
	real_log_type_info[t].enabled = true;
}

void log_disable(LogType t) {
	real_log_type_info[t].enabled = false;
}

void log_toggle(LogType t) {
	real_log_type_info[t].enabled = !real_log_type_info[t].enabled;
}

bool log_open_logfile(const char *fname) {
	//TODO: prolly need a mutex here...
	if(logsys_log_file != NULL)
		fclose(logsys_log_file);
	logsys_log_file = fopen(fname, "w");
	if(logsys_log_file == NULL)
		return(false);
	return(true);
}


bool load_log_settings(const char *filename) {
	//this is a terrible algorithm, but im lazy today
	FILE *f = fopen(filename, "r");
	if(f == NULL)
		return(false);
	char linebuf[512], type_name[256], value[256];
	while(!feof(f)) {
		if(fgets(linebuf, 512, f) == NULL)
			continue;
#ifdef WIN32
		if (sscanf(linebuf, "%[^=]=%[^\n]\n", type_name, value) != 2)
			continue;
#else	
		if (sscanf(linebuf, "%[^=]=%[^\r\n]\n", type_name, value) != 2)
			continue;
#endif
		
		if(type_name[0] == '\0' || type_name[0] == '#')
			continue;
		
		//first make sure we understand the value
		bool enabled;
		if(!strcasecmp(value, "on") || !strcasecmp(value, "yes") || !strcasecmp(value, "enabled") || !strcmp(value, "1"))
			enabled = true;
		else if(!strcasecmp(value, "off") || !strcasecmp(value, "no") || !strcasecmp(value, "disabled") || !strcmp(value, "0"))
			enabled = false;
		else {
			printf("Unable to parse value '%s' from %s. Skipping line.", value, filename);
			continue;
		}
		
		int r;
		//first see if it is a category name
		for(r = 0; r < NUMBER_OF_LOG_CATEGORIES; r++) {
			if(!strcasecmp(log_category_names[r], type_name))
				break;
		}
		if(r != NUMBER_OF_LOG_CATEGORIES) {
			//matched a category.
			int k;
			for(k = 0; k < NUMBER_OF_LOG_TYPES; k++) {
				if(log_type_info[k].category != r)
					continue;	//does not match this category.
				if(enabled)
					log_enable(LogType(k));
				else
					log_disable(LogType(k));
			}
			continue;
		}
		
		for(r = 0; r < NUMBER_OF_LOG_TYPES; r++) {
			if(!strcasecmp(log_type_info[r].name, type_name))
				break;
		}
		if(r == NUMBER_OF_LOG_TYPES) {
			printf("Unable to locate log type %s from file %s. Skipping line.", type_name, filename);
			continue;
		}
		
		//got it all figured out, do something now...
		if(enabled)
			log_enable(LogType(r));
		else
			log_disable(LogType(r));
	}
	fclose(f);
	return(true);
}





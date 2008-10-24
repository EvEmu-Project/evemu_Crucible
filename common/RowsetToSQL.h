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

//this is a template function which takes an object which models
// int ColumnCount();
// const char *ColumnName(uint32 column);
// typedef iterator;
// iterator begin();
// iterator end();
// and who's iterator models:
// bool IsNone(uint32 column);
// PyRep::Type GetType(uint32 column);
// std::string GetAsString(uint32 column)
// 
// current compatible classes:
// 	RowsetReader
// 	TuplesetReader

#ifndef __ROWSETTOSQL_H_INCL__
#define __ROWSETTOSQL_H_INCL__

#include <stdio.h>
#include <string.h>
#include <vector>
#include "logsys.h"
#include "misc.h"

//this is a little mini state machine
typedef enum {
	ReaderColumnInteger,
	ReaderColumnIntegerNull,
	ReaderColumnReal,
	ReaderColumnRealNull,
	ReaderColumnString,
	ReaderColumnStringNull,
	ReaderColumnLongString,
	ReaderColumnLongStringNull,
	ReaderColumnUnknown,
	ReaderColumnUnknownNull,
	ReaderColumnError	//invalid element encountered (non-terminal)
} ReaderColumnContentsType;

static const uint32 ReaderColumnLongStringCutoff = 63;

static const char *const ReaderColumnContentTypeStrings[11] = {
	/*ReaderColumnInteger*/			"INT UNSIGNED NOT NULL",
	/*ReaderColumnIntegerNull*/		"INT UNSIGNED NULL",
	/*ReaderColumnReal*/			"REAL NOT NULL",
	/*ReaderColumnRealNull*/		"REAL NULL",
	/*ReaderColumnString*/			"VARCHAR(64) NOT NULL",
	/*ReaderColumnStringNull*/		"VARCHAR(64) NULL",
	/*ReaderColumnLongString*/		"MEDIUMTEXT NOT NULL",
	/*ReaderColumnLongStringNull*/	"MEDIUMTEXT NULL",
	/*ReaderColumnUnknown*/			"TEXT NOT NULL",
	/*ReaderColumnUnknownNull*/		"TEXT NULL",
	/*ReaderColumnError*/			"ERROR"
};


//look through the rows in a reader and try to classify the storage type of each column
template <class Reader>
void ClassifyColumnTypes(std::vector<ReaderColumnContentsType> &into, Reader &reader) {
	int col;
	int cc = reader.ColumnCount();
	
	into.clear();
	into.resize(cc, ReaderColumnUnknown);

	
	typedef typename Reader::iterator T_iterator;
	T_iterator cur, end;
	cur = reader.begin();
	end = reader.end();
	for(; cur != end; ++cur) {
		for(col = 0; col < cc; col++) {
			switch(into[col]) {
			case ReaderColumnInteger:
				switch(cur.GetType(col)) {
				case PyRep::None:
					into[col] = ReaderColumnIntegerNull;
					break;
				case PyRep::Integer:
					break;	//no transition
				case PyRep::Real:
					//we can become a real
					into[col] = ReaderColumnReal;
					break;
				case PyRep::String:
					//we can become a string
					into[col] = ReaderColumnString;
					break;
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnIntegerNull:
				switch(cur.GetType(col)) {
				case PyRep::None:
					break;	//no transition
				case PyRep::Integer:
					break;	//no transition
				case PyRep::Real:
					//we can become a real
					into[col] = ReaderColumnRealNull;
					break;
				case PyRep::String:
					//we can become a string
					into[col] = ReaderColumnStringNull;
					break;
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnReal:
				switch(cur.GetType(col)) {
				case PyRep::None:
					into[col] = ReaderColumnRealNull;
					break;
				case PyRep::Integer:
					break;	//no transition, stay in real land
				case PyRep::Real:
					break;	//no transition
				case PyRep::String:
					//we can become a string
					into[col] = ReaderColumnString;
					break;
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnRealNull:
				switch(cur.GetType(col)) {
				case PyRep::None:
					break;	//no transition
				case PyRep::Integer:
					break;	//no transition, stay in real land
				case PyRep::Real:
					break;	//no transition
				case PyRep::String:
					//we can become a string
					into[col] = ReaderColumnString;
					break;
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnString:
				switch(cur.GetType(col)) {
				case PyRep::None:
					into[col] = ReaderColumnStringNull;
				case PyRep::Integer:
					break;	//no transition, stay in string land
				case PyRep::Real:
					break;	//no transition, stay in string land
				case PyRep::String: {
					std::string contents = cur.GetAsString(col);
					if(contents.length() > ReaderColumnLongStringCutoff)
						into[col] = ReaderColumnLongString;
					break;
				}
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnStringNull:
				switch(cur.GetType(col)) {
				case PyRep::None:
					break;	//no transition
				case PyRep::Integer:
					break;	//no transition, stay in string land
				case PyRep::Real:
					break;	//no transition, stay in string land
				case PyRep::String: {
					std::string contents = cur.GetAsString(col);
					if(contents.length() > ReaderColumnLongStringCutoff)
						into[col] = ReaderColumnLongStringNull;
					break;
				}
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnLongString:
				switch(cur.GetType(col)) {
				case PyRep::None:
					into[col] = ReaderColumnLongStringNull;
					break;
				case PyRep::Integer:
					break;	//no transition, stay in string land
				case PyRep::Real:
					break;	//no transition, stay in string land
				case PyRep::String:
					break;	//no transition
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnLongStringNull:
				switch(cur.GetType(col)) {
				case PyRep::None:
					break;	//no transition
				case PyRep::Integer:
					break;	//no transition, stay in string land
				case PyRep::Real:
					break;	//no transition, stay in string land
				case PyRep::String:
					break;	//no transition
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnUnknown:
				switch(cur.GetType(col)) {
				case PyRep::None:
					into[col] = ReaderColumnUnknownNull;
					break;
				case PyRep::Integer:
					into[col] = ReaderColumnInteger;
					break;
				case PyRep::Real:
					into[col] = ReaderColumnReal;
					break;
				case PyRep::String: {
					std::string contents = cur.GetAsString(col);
					if(contents.length() > ReaderColumnLongStringCutoff)
						into[col] = ReaderColumnLongString;
					else
						into[col] = ReaderColumnString;
					break;
				}
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnUnknownNull:
				switch(cur.GetType(col)) {
				case PyRep::None:
					break;	//no transition
				case PyRep::Integer:
					into[col] = ReaderColumnIntegerNull;
					break;
				case PyRep::Real:
					into[col] = ReaderColumnRealNull;
					break;
				case PyRep::String: {
					std::string contents = cur.GetAsString(col);
					if(contents.length() > ReaderColumnLongStringCutoff)
						into[col] = ReaderColumnLongStringNull;
					else
						into[col] = ReaderColumnStringNull;
					break;
				}
				default:
					//anything else is an error.
					into[col] = ReaderColumnError;
					break;
				}
				break;
			case ReaderColumnError:
				//nothing can get us out of this state.
				break;
			}
		}
	}
}

template <class Reader>
bool ReaderToSQL(const char *table_name, const char *key_field, FILE *out, Reader &reader) {
	size_t col;
	size_t cc = reader.ColumnCount();
	int key_col = -1;

	if(key_field != NULL) {
		for(col = 0; col < cc; col++) {
			if(strcmp(key_field, reader.ColumnName(col)) == 0) {
				key_col = col;
				break;
			}
		}
		if(key_col == -1) {
			_log(CLIENT__ERROR, "Unable to find key column '%s'", key_field);
			return(false);
		}
	}

	std::vector<ReaderColumnContentsType> types;
	ClassifyColumnTypes<Reader>(types, reader);
	
	std::string field_list;
	fprintf(out, "CREATE TABLE %s (\n", table_name);
	for(col = 0; col < cc; col++) {
		//a little type patchwork to handle all-NULL fields which end in ID
		if(types[col] == ReaderColumnUnknownNull || types[col] == ReaderColumnUnknown) {
			std::string cn = reader.ColumnName(col);
			if(cn.rfind("ID") == (cn.length()-2))
				types[col] = ReaderColumnIntegerNull;
		}

		
		if(col != 0)
			fprintf(out, ",\n");
		if(col == key_col)
			fprintf(out, "  %s %s PRIMARY KEY", reader.ColumnName(col), ReaderColumnContentTypeStrings[types[col]]);
		else	//hack it to TEXT for simplicity for now
			fprintf(out, "  %s %s", reader.ColumnName(col), ReaderColumnContentTypeStrings[types[col]]);
		
		if(col != 0)
			field_list += ",";
		field_list += reader.ColumnName(col);
	}
	if(key_field == NULL) {
		fprintf(out, ",\n  PRIMARY KEY()");
	}
	fprintf(out, "\n);\n\n\n--- %s data ---\n\n", table_name);

	typedef typename Reader::iterator T_iterator;
	T_iterator cur, end;
	cur = reader.begin();
	end = reader.end();
	for(; cur != end; ++cur) {
		fprintf(out, "INSERT INTO %s (%s) VALUES(", table_name, field_list.c_str());
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
	fprintf(out, "\n\n--- end %s ---\n\n", table_name);

	return(true);
}

#endif




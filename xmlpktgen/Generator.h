#ifndef __GENERATOR_H_INCL__
#define __GENERATOR_H_INCL__

#include "../common/tinyxml/tinyxml.h"
#include "../common/logsys.h"
#include <stdio.h>
#include <map>
#include <string>

class Generator {
public:
	virtual ~Generator() {}
	virtual void Generate(FILE *into, TiXmlElement *root);
protected:
	virtual void Process_root(FILE *into, TiXmlElement *element) = 0;
	virtual void Process_include(FILE *into, TiXmlElement *element);

	const char *GetEncodeType(TiXmlElement *element) const;
	std::map<std::string, std::string> m_eleTypes;
};

class ClassHeaderGenerator;

template <class T>
class TmplGenerator : public Generator {
public:
	virtual ~TmplGenerator() {}

protected:
	typedef bool (T:: * fProcPtr)(FILE *into, TiXmlElement *field);
	typedef typename std::map<std::string, fProcPtr>::iterator iterator;
	
	bool ProcessFields(FILE *into, TiXmlElement *element, int max=0) {
		TiXmlNode *field = NULL;
		int count = 0;
		while((field = element->IterateChildren( field )) ) {
			//pass through comments.
			if(field->Type() == TiXmlNode::COMMENT) {
				TiXmlComment *com = field->ToComment();
				fprintf(into, "\t/* %s */\n", com->Value());
				continue;
			}
			if(field->Type() != TiXmlNode::ELEMENT)
				continue;	//skip crap we dont care about

			count++;
			if(max > 0 && count > max) {
				_log(COMMON__ERROR, "Element at line %d has too many children elements, at most %d expected.", element->Row(), max);
				return(false);
			}
			if(!DispatchField(into, field->ToElement()))
				return(false);
		}
		return(true);
	}
	
	bool DispatchField(FILE *into, TiXmlElement *field) {
		iterator res;
		res = m_procs.find(field->Value());
		if(res == m_procs.end()) {
			_log(COMMON__ERROR, "Unexpected field type '%s' on line %d", field->Value(), field->Row());
			return(false);
		}
		fProcPtr p = res->second;
		T *t = (T *) this;

		return( (t->*p)(into, field) );
	}

	

	std::map<std::string, fProcPtr> m_procs;
};

#define ProcFDecl(t) \
	bool Process_##t(FILE *into, TiXmlElement *field)
#define ProcFMap(c, ename, type) \
	m_eleTypes[ #ename ] = #type; m_procs[ #ename ] = &c::Process_##ename

#define AllProcFDecls() \
	ProcFDecl(InlineTuple); \
	ProcFDecl(InlineDict); \
	ProcFDecl(InlineList); \
	ProcFDecl(InlineSubStream); \
	ProcFDecl(InlineSubStruct); \
	ProcFDecl(strdict); \
	ProcFDecl(intdict); \
	ProcFDecl(primdict); \
	ProcFDecl(strlist); \
	ProcFDecl(intlist); \
	ProcFDecl(int64list); \
	ProcFDecl(element); \
	ProcFDecl(elementptr); \
	ProcFDecl(none); \
	ProcFDecl(object); \
	ProcFDecl(buffer); \
	ProcFDecl(raw); \
	ProcFDecl(dict); \
	ProcFDecl(list); \
	ProcFDecl(tuple); \
	ProcFDecl(int); \
	ProcFDecl(bool); \
	ProcFDecl(int64); \
	ProcFDecl(real); \
	ProcFDecl(string)

#define AllProcFMaps(c) \
	ProcFMap(c, InlineTuple, PyRepTuple); \
	ProcFMap(c, InlineDict, PyRepDict); \
	ProcFMap(c, InlineList, PyRepList); \
	ProcFMap(c, InlineSubStream, PyRepSubStream); \
	ProcFMap(c, InlineSubStruct, PyRepSubStruct); \
	ProcFMap(c, strdict, PyRepDict); \
	ProcFMap(c, intdict, PyRepDict); \
	ProcFMap(c, primdict, PyRepDict); \
	ProcFMap(c, strlist, PyRepList); \
	ProcFMap(c, intlist, PyRepList); \
	ProcFMap(c, int64list, PyRepList); \
	ProcFMap(c, element, PyRep); \
	ProcFMap(c, elementptr, PyRep); \
	ProcFMap(c, none, PyRep); \
	ProcFMap(c, object, PyRepObject); \
	ProcFMap(c, buffer, PyRepBuffer); \
	ProcFMap(c, raw, PyRep); \
	ProcFMap(c, list, PyRepList); \
	ProcFMap(c, tuple, PyRepTuple); \
	ProcFMap(c, dict, PyRepDict); \
	ProcFMap(c, int, PyRepInteger); \
	ProcFMap(c, bool, PyRepBoolean); \
	ProcFMap(c, int64, PyRepInteger); \
	ProcFMap(c, real, PyRepReal); \
	ProcFMap(c, string, PyRepString)
	


#endif



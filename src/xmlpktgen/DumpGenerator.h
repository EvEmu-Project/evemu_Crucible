#ifndef __DUMPGENERATOR_H_INCL__
#define __DUMPGENERATOR_H_INCL__

#include "Generator.h"


class ClassDumpGenerator
: public TmplGenerator<ClassDumpGenerator> {
public:
	ClassDumpGenerator();
	virtual ~ClassDumpGenerator() {}
	
	void Process_root(FILE *into, TiXmlElement *element);
	
protected:
	
	AllProcFDecls();
	ProcFDecl(IDEntry);
};



#endif



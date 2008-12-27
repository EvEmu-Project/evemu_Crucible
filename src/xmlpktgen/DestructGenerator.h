#ifndef __DESTRUCTGENERATOR_H_INCL__
#define __DESTRUCTGENERATOR_H_INCL__

#include "Generator.h"


class ClassDestructGenerator
: public TmplGenerator<ClassDestructGenerator> {
public:
	ClassDestructGenerator();
	virtual ~ClassDestructGenerator() {}
	
	void Process_root(FILE *into, TiXmlElement *element);
	
protected:
	
	AllProcFDecls();
	ProcFDecl(IDEntry);
};





#endif



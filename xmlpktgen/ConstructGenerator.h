#ifndef __CONSTRUCTGENERATOR_H_INCL__
#define __CONSTRUCTGENERATOR_H_INCL__


#include "Generator.h"

class ClassConstructGenerator
: public TmplGenerator<ClassConstructGenerator> {
public:
	ClassConstructGenerator();
	virtual ~ClassConstructGenerator() {}
	
	void Process_root(FILE *into, TiXmlElement *element);
	
protected:
	
	AllProcFDecls();
	ProcFDecl(IDEntry);
};





#endif



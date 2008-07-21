#ifndef __CLONEGENERATOR_H_INCL__
#define __CLONEGENERATOR_H_INCL__

#include "Generator.h"



class ClassCloneGenerator
: public TmplGenerator<ClassCloneGenerator> {
public:
	ClassCloneGenerator();
	virtual ~ClassCloneGenerator() {}
	
	void Process_root(FILE *into, TiXmlElement *element);
	
protected:
	
	AllProcFDecls();
	ProcFDecl(IDEntry);
};



#endif



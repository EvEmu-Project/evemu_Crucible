#ifndef __HEADERGENERATOR_H_INCL__
#define __HEADERGENERATOR_H_INCL__

#include "Generator.h"
#include <set>



class ClassHeaderGenerator
: public TmplGenerator<ClassHeaderGenerator> {
public:
	ClassHeaderGenerator();
	virtual ~ClassHeaderGenerator() {}
protected:
	void Process_root(FILE *into, TiXmlElement *element);
	void Process_include(FILE *into, TiXmlElement *element);
	
	AllProcFDecls();
	ProcFDecl(IDEntry);
	
	std::set<std::string> m_namesUsed;
};


#endif



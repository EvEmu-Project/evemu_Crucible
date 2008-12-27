#ifndef __IMPLGENERATOR_H_INCL__
#define __IMPLGENERATOR_H_INCL__

#include "ConstructGenerator.h"
#include "DestructGenerator.h"
#include "DumpGenerator.h"
#include "EncodeGenerator.h"
#include "DecodeGenerator.h"
#include "CloneGenerator.h"

class ClassImplementationGenerator
: public Generator {
public:
	virtual ~ClassImplementationGenerator() {}
protected:
	void Process_root(FILE *into, TiXmlElement *element);

	ClassConstructGenerator	m_construct;
	ClassDestructGenerator	m_destruct;
	ClassDumpGenerator		m_dump;
	ClassEncodeGenerator	m_encode;
	ClassDecodeGenerator	m_decode;
	ClassCloneGenerator		m_clone;
};





#endif



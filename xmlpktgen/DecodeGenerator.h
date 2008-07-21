#ifndef __DECODEGENERATOR_H_INCL__
#define __DECODEGENERATOR_H_INCL__

#include "Generator.h"
#include <stack>

class ClassDecodeGenerator
: public TmplGenerator<ClassDecodeGenerator> {
public:
	ClassDecodeGenerator();
	virtual ~ClassDecodeGenerator() {}
	
	void Process_root(FILE *into, TiXmlElement *element);
	
protected:
	int m_itemNumber;
	std::stack<std::string> m_variableQueue;
	const char *m_name;
	
	inline const char *top() const { return(m_variableQueue.top().c_str()); }
	inline void pop() { m_variableQueue.pop(); }
	inline void push(const char *v) { m_variableQueue.push(v); }
	
	AllProcFDecls();
};


#endif



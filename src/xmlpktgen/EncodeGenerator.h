#ifndef __ENCODEGENERATOR_H_INCL__
#define __ENCODEGENERATOR_H_INCL__

#include "Generator.h"
#include <stack>

class ClassEncodeGenerator
: public TmplGenerator<ClassEncodeGenerator> {
public:
	ClassEncodeGenerator();
	virtual ~ClassEncodeGenerator() {}
	
	void Process_root(FILE *into, TiXmlElement *element);
	
protected:
	bool m_fast;
	int m_itemNumber;
	std::stack<std::string> m_variableQueue;
	const char *m_name;
	
	inline const char *top() const { return(m_variableQueue.top().c_str()); }
	inline void pop() { m_variableQueue.pop(); }
	inline void push(const char *v) { m_variableQueue.push(v); }
	
	AllProcFDecls();
};


#endif



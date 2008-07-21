
#include "ImplGenerator.h"

void ClassImplementationGenerator::Process_root(FILE *into, TiXmlElement *element) {
	m_construct.Process_root(into, element);
	m_destruct.Process_root(into, element);
	m_dump.Process_root(into, element);
	m_encode.Process_root(into, element);
	m_decode.Process_root(into, element);
	m_clone.Process_root(into, element);
}



















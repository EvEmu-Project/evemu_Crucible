#ifndef __PYUTILS_H_INCL__
#define __PYUTILS_H_INCL__

#include "types.h"
#include <vector>

class PyRepTuple;

bool ParseIntegerList(const PyRepTuple *iargs, const char *caller, std::vector<uint32> &into);



#endif





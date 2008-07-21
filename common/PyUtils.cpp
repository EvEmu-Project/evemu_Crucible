
#include "PyUtils.h"
#include "logsys.h"
#include "PyRep.h"


bool ParseIntegerList(const PyRepTuple *iargs, const char *caller, std::vector<uint32> &into) {
	
	if(iargs->items.size() != 1) {
		_log(SERVICE__ERROR, "Invalid arg count to %s: %d", caller, iargs->items.size());
		//TODO: throw exception
		return(false);
	} else if(!iargs->items[0]->CheckType(PyRep::List)) {
		_log(SERVICE__ERROR, "Invalid argument to %s: got %s, list expected", caller, iargs->items[0]->TypeString());
		return(false);
	}
	
	const PyRepList *chanlist = (const PyRepList *) iargs->items[0];

	PyRepList::const_iterator cur, end;
	cur = chanlist->begin();
	end = chanlist->end();
	for(; cur != end; cur++) {
		if(!(*cur)->CheckType(PyRep::Integer)) {
			_log(SERVICE__ERROR, "Invalid argument to %s: got %s, int expected", caller, (*cur)->TypeString());
			return(false);
		}
		const PyRepInteger *i = (const PyRepInteger *) *cur;
		into.push_back(i->value);
	}

	return(true);
}




















#include "EVECollectDisp.h"
#include "../common/PyRep.h"



void EVECollectDispatcher::Rsp_GetCharactersToSelect(const PyPacket *packet, PyRepTuple **res) {
	//RspGetCharactersToSelect rsp;
	_log(COLLECT__ERROR, "Unhandled char list");
}


void EVECollectDispatcher::Bound_Rsp_List(const PyPacket *packet, PyRepTuple **res) {
	
	_log(COLLECT__ERROR, "Unhandled item list");
}


















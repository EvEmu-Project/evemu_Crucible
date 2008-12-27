
#include "NPCAI_State.h"
#include "../common/logsys.h"
#include "../NPC.h"

namespace NPCAI {
 namespace State {

void _Top::init() {
	setState<Idle>();
}

void _Top::entry() {
	_log(NPC__AI_TRACE, "%s: AI has entered state %s", npc()->GetName(), GetStateName());
}

void Idle::Process() {
}

void Wandering::Process() {
}

void Engaged::Process() {
}

void Engaged::TargetLost(SystemEntity *who) {
}

void Pursuing::Process() {
}








 }
}













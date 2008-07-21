#ifndef __NPCAI_STATE_H_INCL__
#define __NPCAI_STATE_H_INCL__

#include "../libs/cppstate/CPPState.h"

class NPC;
class SystemEntity;

namespace NPCAI {

 namespace State {
	TOPSTATE(_Top) {
		typedef NPC *Box;
		
		STATE(_Top)
		
        // Event handlers
		virtual void Process() {}
		virtual void TargetedByOther(SystemEntity *who) {}
		virtual void TargetLost(SystemEntity *who) {}

	protected:
		NPC *npc() { return(box()); }
	private:
		//State Transitions:
		virtual void entry();
		virtual void init();
		//virtual void exit();
		
		PERSISTENT()	//do not destroy the Box field. Inherited by all substates.
	};

	
    SUBSTATE(Idle, _Top) {
        STATE(Idle)
		
        // Event handlers
		virtual void Process();
    private:
		//State Transitions:
		//virtual void entry();
		//virtual void exit();
    };
	
    SUBSTATE(Wandering, Idle) {
        STATE(Wandering)
		
        // Event handlers
		virtual void Process();
    private:
		//State Transitions:
		//virtual void entry();
		//virtual void exit();
    };

	
    SUBSTATE(Engaged, Idle) {
        STATE(Engaged)
		
        // Event handlers
		virtual void Process();
		virtual void TargetLost(SystemEntity *who);
    private:
		//State Transitions:
		//virtual void entry();
		//virtual void exit();
    };

	
    SUBSTATE(Pursuing, Engaged) {
        STATE(Pursuing)
		
        // Event handlers
		virtual void Process();
    private:
		//State Transitions:
		//virtual void entry();
		//virtual void exit();
    };



 }

	typedef CPPState::Machine<NPCAI::State::Idle> NPCAI_StateMachine;
}

#include "../libs/cppstate/CPPState_end.h"


#endif





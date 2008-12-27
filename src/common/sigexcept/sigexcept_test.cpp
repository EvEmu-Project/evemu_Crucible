#include <stdio.h>

#define IMPLEMENT_SIGEXCEPT
#include "sigexcept.h"

int main(int argc, char *argv[]) {
	INIT_SIGEXCEPT();

	TRY_SIGEXCEPT {
		char *bad = NULL;
		*bad = 1;
	} CATCH_SIGEXCEPT(e) {
		  printf( "Exception '%s' caught processing:\n%s", e.type_string(), e.to_string().c_str());
	}
	
	TRY_SIGEXCEPT {
        int x = 42/0;
        printf("ran divide: %d\n", x);
	} CATCH_SIGEXCEPT(e) {
		  printf( "Exception '%s' caught processing:\n%s", e.type_string(), e.to_string().c_str());
	}
	
	TRY_SIGEXCEPT {
		unsigned char insn[4] = { 0xff, 0xff, 0xff, 0xff };
		void (*function)() = (void (*)()) insn;
		function();
	} CATCH_SIGEXCEPT(e) {
		  printf( "Exception '%s' caught processing:\n%s", e.type_string(), e.to_string().c_str());
	}
	return(1);
}

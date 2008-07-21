/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef SIGEXCEPT_H__
#define SIGEXCEPT_H__

#include <vector>
#include <string>
#include <map>

#include "../common.h"

#ifdef __x86_64__
typedef uint64 SigExceptRegisterType;
#define SigExceptRegisterFormat I64x
#else
typedef uint32 SigExceptRegisterType;
#define SigExceptRegisterFormat "%lx"
#endif


class sigexcept_exception {
public:
	typedef enum {
		InvalidMemoryAccess,
		FloatingPointException,
		InvalidInstruction,
		Other
	} etype;
	static const char *const TypeStrings[4];
	etype type;
	std::vector< std::string > stacktrace;
	std::map< std::string, SigExceptRegisterType > registers;
	sigexcept_exception( etype t ) : type(t) { }
	void stack_string(std::string &into) const {
		std::vector< std::string >::const_iterator cur, end;
		cur = stacktrace.begin();
		end = stacktrace.end();
		for(; cur != end; ++cur) {
			into += *cur;
			into += "\n";
		}
	}
	void registers_string(std::string &into) const {
		std::map< std::string, SigExceptRegisterType >::const_iterator cur, end;
		cur = registers.begin();
		end = registers.end();
		char buf[64];
		int row_count = 0;
		for(; cur != end; ++cur ) {
			snprintf(buf, sizeof(buf), SigExceptRegisterFormat, cur->second);
			buf[sizeof(buf)-1] = '\0';
			into += cur->first + ": 0x" + buf;
			if(row_count < 3) {
				into += ", ";
				++row_count;
			} else {
				into += "\n";
				row_count = 0;
			}
		}
	}
	std::string to_string() const {
		std::string t;
		t += "Stack:\n";
		stack_string(t);
		if(!registers.empty()) {
			t += "Register Dump:\n";
			registers_string(t);
		}
		return(t);
	}
	std::string stack_string() const { std::string t; stack_string(t); return(t); }
	const char *type_string() const { return(TypeStrings[type]); }
};

#ifdef IMPLEMENT_SIGEXCEPT
const char *const sigexcept_exception::TypeStrings[4] = {
	"InvalidMemoryAccess",
	"FloatingPointException",
	"InvalidInstruction",
	"Other"
};
#endif

#ifdef USE_RUNTIME_EXCEPTIONS

#ifdef WIN32
	#include <eh.h>
	#include "sym_engine.h"
//#error need to test this.
    /* windows has a build in mechanism for runtime exceptions. */
	#ifdef IMPLEMENT_SIGEXCEPT
		static void SigExcept_trans_func( unsigned int ExceptionCode, EXCEPTION_POINTERS *ExceptionInfo ) {
			sigexcept_exception e(sigexcept_exception::Other);
			switch(ExceptionCode) {
			case EXCEPTION_ACCESS_VIOLATION:
			case STATUS_STACK_OVERFLOW:
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				e = sigexcept_exception(sigexcept_exception::InvalidMemoryAccess);
				break;
			case STATUS_FLOAT_DIVIDE_BY_ZERO:
			case STATUS_FLOAT_DENORMAL_OPERAND:
			case STATUS_FLOAT_INEXACT_RESULT:
			case STATUS_FLOAT_INVALID_OPERATION:
			case STATUS_FLOAT_OVERFLOW:
			case STATUS_FLOAT_STACK_CHECK:
			case STATUS_FLOAT_UNDERFLOW:
			case STATUS_INTEGER_OVERFLOW:
			case STATUS_INTEGER_DIVIDE_BY_ZERO:
				e = sigexcept_exception(sigexcept_exception::FloatingPointException);
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
			case EXCEPTION_PRIV_INSTRUCTION:
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				e = sigexcept_exception(sigexcept_exception::InvalidInstruction);
				break;
			default:
				e = sigexcept_exception(sigexcept_exception::Other);
				break;
			}
			sym_engine::stack_trace(e.stacktrace, ExceptionInfo->ContextRecord, 0, "[%a] %s (%m at %f:%l)");
			throw e;
		}
	#endif
	#define INIT_SIGEXCEPT() \
		_set_se_translator( SigExcept_trans_func );

	#define TRY_SIGEXCEPT \
		try
	#define CATCH_SIGEXCEPT(signame) \
		catch ( sigexcept_exception e )
	#define RUNTIME_EXCEPTIONS_DEFINED__ 1
#else
#if defined(__i386__) || defined(__x86_64__)
    /* use GNU libjava stuff. */

	#define MAX_BACKTRACE_FRAMES 32
	
	#ifdef IMPLEMENT_SIGEXCEPT
		#define ADD_REG(n) \
			e.registers[ #n ] = _regs->n
		#ifdef __i386__
			#include "i386-signal.h"
			#define SIGEXCEPT_DECODE_REGS \
				ADD_REG(gs); \
				ADD_REG(fs); \
				ADD_REG(es); \
				ADD_REG(ds); \
				ADD_REG(edi); \
				ADD_REG(esi); \
				ADD_REG(ebp); \
				ADD_REG(esp); \
				ADD_REG(ebx); \
				ADD_REG(edx); \
				ADD_REG(ecx); \
				ADD_REG(eax); \
				ADD_REG(trapno); \
				ADD_REG(err); \
				ADD_REG(eip); \
				ADD_REG(eflags);
			#define SIGEXCEPT_DECODE_GREGS \
				ADD_REG(GS); \
				ADD_REG(FS); \
				ADD_REG(ES); \
				ADD_REG(DS); \
				ADD_REG(EDI); \
				ADD_REG(ESI); \
				ADD_REG(EBP); \
				ADD_REG(ESP); \
				ADD_REG(EBX); \
				ADD_REG(EDX); \
				ADD_REG(ECX); \
				ADD_REG(EAX); \
				ADD_REG(TRAPNO); \
				ADD_REG(ERR); \
				ADD_REG(EIP); \
				ADD_REG(EFL);
			//#define SIGEXCEPT_LAST_INSTRUCTION ctx->gregs[REG_EIP]
			#define SIGEXCEPT_LAST_INSTRUCTION _regs->eip
		#elif __x86_64__
			#include "x86_64-signal.h"
			//NOTE: some reworking needs to be done in this crap
			//to support x86_64, since the way the libjava stuff
			// obtains the sigcontext is different.
			#define SIGEXCEPT_DECODE_REGS
			#define SIGEXCEPT_DECODE_REGS_EVENTUALLY \
				ADD_REG(gs); \
				ADD_REG(fs); \
				ADD_REG(es); \
				ADD_REG(ds); \
				ADD_REG(ss); \
				ADD_REG(cs); \
				ADD_REG(rax); \
				ADD_REG(rbx); \
				ADD_REG(rcx); \
				ADD_REG(rdx); \
				ADD_REG(rsi); \
				ADD_REG(rbp); \
				ADD_REG(rsp); \
				ADD_REG(r8); \
				ADD_REG(r9); \
				ADD_REG(r10); \
				ADD_REG(r11); \
				ADD_REG(r12); \
				ADD_REG(r13); \
				ADD_REG(r14); \
				ADD_REG(rflags);
			#define SIGEXCEPT_LAST_INSTRUCTION backtrace_buffer[1]
			#define SIGEXCEPT_LAST_INSTRUCTION_EVENTUALLY _regs->rip
		#endif
		#include <execinfo.h>
		
		static void sigexcept_unblock_signal (int signum __attribute__ ((__unused__))) {
		  sigset_t sigs;
		  sigemptyset (&sigs);
		  sigaddset (&sigs, signum);
		  sigprocmask (SIG_UNBLOCK, &sigs, NULL);
		}
		#define MAKE_SIG_HANDLER(name, sig, type) \
		SIGNAL_HANDLER(name) { \
			sigexcept_unblock_signal (sig); \
			DIV_OVF \
			MAKE_THROW_FRAME( type ); \
			void *backtrace_buffer[MAX_BACKTRACE_FRAMES]; \
			int frame_count = backtrace(backtrace_buffer, MAX_BACKTRACE_FRAMES); \
			void **_p2 = (void **)&_dummy; \
			volatile struct sigcontext_struct *_regs = (struct sigcontext_struct *)++_p2; \
			backtrace_buffer[1] = (void *)(SIGEXCEPT_LAST_INSTRUCTION); \
			int r; \
			sigexcept_exception e(sigexcept_exception::type); \
			char **frames = backtrace_symbols(backtrace_buffer, frame_count); \
			for(r = 0; r < frame_count; r++) { \
				e.stacktrace.push_back(frames[r]); \
			} \
			free(frames); \
            SIGEXCEPT_DECODE_REGS \
			throw e; \
		}
		#ifdef HANDLE_SEGV
			#define DIV_OVF
			MAKE_SIG_HANDLER(catch_segv, SIGSEGV, InvalidMemoryAccess)
			MAKE_SIG_HANDLER(catch_ill, SIGILL, InvalidInstruction)
			#undef DIV_OVF
		#else
			#define INIT_SEGV if(0)
			#define INIT_ILL if(0)
		#endif
		#ifdef HANDLE_FPE
			#ifdef HANDLE_DIVIDE_OVERFLOW
			  #define DIV_OVF HANDLE_DIVIDE_OVERFLOW;
			#else
			  #define DIV_OVF 
			#endif
			MAKE_SIG_HANDLER(catch_fpe, SIGFPE, FloatingPointException)
			#undef DIV_OVF
		#else
			#define INIT_FPE if(0)
		#endif
		#undef ADD_REG
		#undef MAKE_SIG_HANDLER

		//must be used in the same file as IMPLEMENT_SIGEXCEPT is defined.
		#define INIT_SIGEXCEPT() \
			INIT_SEGV; \
			INIT_ILL; \
			INIT_FPE
	#endif
		
	#define TRY_SIGEXCEPT \
		try 
	#define CATCH_SIGEXCEPT(signame) \
		catch ( sigexcept_exception signame )
	#define RUNTIME_EXCEPTIONS_DEFINED__ 1
#else
#warning No runtime exception handler is defined for this architecture.
#endif
#endif

#endif	/* USE_RUNTIME_EXCEPTIONS */

#ifndef RUNTIME_EXCEPTIONS_DEFINED__
	#define INIT_SIGEXCEPT() if(0)
	#define IMPLEMENT_SIGEXCEPT
	#define TRY_SIGEXCEPT
	//this is a little dirty, but it works...
	#define CATCH_SIGEXCEPT(signame) for(sigexcept_exception signame(sigexcept_exception::Other); false; )
#endif




#endif










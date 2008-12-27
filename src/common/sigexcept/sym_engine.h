/*
 Copyright (c) 2001 - 2002
 Author: Konstantin Boukreev 
 E-mail: konstantin@mail.primorye.ru 
 Created: 25.12.2001 19:41:00
 Version: 1.0.2

 Permission to use, copy, modify, distribute and sell this software
 and its documentation for any purpose is hereby granted without fee,
 provided that the above copyright notice appear in all copies and
 that both that copyright notice and this permission notice appear
 in supporting documentation.  Konstantin Boukreev makes no representations
 about the suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.
 
 sym_engine class incapsulate Symbol Handler and Debugging Service API
	
 the list of used API:
	SymInitialize, SymCleanup, SymGetLineFromAddr, SymGetModuleBase,
	SymGetSymbolInfo, SymGetSymFromAddr, SymGetSymFromName, SymGetSymNext,
	SymLoadModule, SymSetOptions
	StackWalk

 based on code\ideas from John Robbins's book "Debugging application"
 http://www.wintellect.com/robbins
*/

#ifndef _sym_engine_e4b31bc5_8e01_4cda_b5a4_905dde52ac01
#define _sym_engine_e4b31bc5_8e01_4cda_b5a4_905dde52ac01

#if _MSC_VER > 1000 
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN64
#error Win64 is not supported
#endif

#include <imagehlp.h>
#ifdef SYM_ENGINE_STREAMS
	#include <ostream>
	typedef std::ostream SymEngineBuffer;
#else
	#include <string>
	#include <vector>
	typedef std::vector< std::string > SymEngineBuffer;
#endif

class sym_engine
{
 public:
	sym_engine (unsigned);
	~sym_engine();

	void address(unsigned a)		{ m_address = a; }
	unsigned address(void) const	{ return m_address; }
	
	// symbol handler queries
	unsigned module  (char *, unsigned);
	unsigned symbol  (char *, unsigned, unsigned * = 0);
	unsigned fileline(char *, unsigned, unsigned *, unsigned * = 0);

	// stack walk
	bool stack_first (CONTEXT* pctx);
	bool stack_next  ();

	/*
		format argument
		%m  : module
		%f  : file
		%l  : line
		%ld : line's displacement
		%s  : symbol
		%sd : symbol's displacement
		
		for example	
		"%f(%l) : %m at %s\n"
		"%m.%s + %sd bytes, in %f:line %l\n"
	*/	
	static bool stack_trace(SymEngineBuffer&, CONTEXT *, unsigned skip = 0, const char * fmt = default_fmt());
	static bool stack_trace(SymEngineBuffer&, unsigned skip = 1, const char * fmt = default_fmt()); 	
	static bool stack_trace(SymEngineBuffer&, sym_engine&, CONTEXT *,
			unsigned skip = 1, const char * fmt = default_fmt());
 private:
	static const char * default_fmt() { return "%f(%l) : %m at %s\n"; }	
	static bool get_line_from_addr (HANDLE, unsigned, unsigned *, IMAGEHLP_LINE *);
	static unsigned get_module_basename (HMODULE, char *, unsigned);
	
	bool check();

 private:
	unsigned		m_address;
	bool			m_ok;
	STACKFRAME *	m_pframe;
	CONTEXT *		m_pctx;
	
 private:
	class guard
	{	
	 private:
		guard();
	 public:
		~guard();
		bool init();		
		bool fail() const { return m_ref == -1; }
		static guard & instance() 
		{
			static guard g; 
			return g;
		}
	private:		
		void clear();
		bool load_module(HANDLE, HMODULE);
		int  m_ref;
	};
};

// Change Log:
//		25.12.2001  Konstantin, Initial version.
//		11.01.2002	Konstantin, added skip arg to stack_trace()
//		11.01.2002	Konstantin, format argument to stack_trace()
//		14.01.2002	Konstantin, fixed bug in thread's wait logic
//		16.01.2002, Konstantin, fixed bug in stack_trace(std::ostream&, CONTEXT *, const char*) with skip value
//		16.01.2002, Konstantin, made guard as singelton


#endif //_sym_engine_e4b31bc5_8e01_4cda_b5a4_905dde52ac01


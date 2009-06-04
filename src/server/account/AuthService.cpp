/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

#include "EvemuPCH.h"

#ifdef SHOW_LOGIN_MESSAGE
//some adverts - may be customized
static const char *const loginMessage = 
"<html>"
	"<head>"
	"</head>"
	"<body>"
		"Welcome to <b>EVEmu Server " EVEMU_REVISION "</b>.<br>"
		"<br>"
		"You can find a lot of interesting info about this project at <a href=\"http://evemu.sourceforge.net/\">SoureForge.net</a> or at <a href=\"http://mmoforge.org/gf/project/evemu\">MMOForge.org</a>.<br>"
		"<br>"
		"You can also join our IRC channel at <b>irc.mmoforge.org:6667</b>, channel <b>#evemu</b>.<br>"
		"<br>"
		"Best wishes,<br>"
		"EVEmu development team"
	"</body>"
"</html>";
#endif /* SHOW_LOGIN_MESSAGE */

class AuthService::Dispatcher
: public PyCallableDispatcher<AuthService> {
public:
	Dispatcher(AuthService *c)
	: PyCallableDispatcher<AuthService>(c) {}
};


AuthService::AuthService(PyServiceMgr *mgr)
: PyService(mgr, "authentication"),
m_dispatch(new Dispatcher(this))
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(AuthService, Ping)
	PyCallable_REG_CALL(AuthService, GetPostAuthenticationMessage)
}

AuthService::~AuthService() {
	delete m_dispatch;
}


PyResult AuthService::Handle_Ping(PyCallArgs &call) {
	return(new PyRepInteger(Win32TimeNow()));
}


PyResult AuthService::Handle_GetPostAuthenticationMessage(PyCallArgs &call) {
	PyRep *result = NULL;
/*
	PyRepObject *o = new PyRepObject();
	result = o;
		o->type = "util.KeyVal";
		PyRepDict *obj_args = new PyRepDict();
		o->arguments = obj_args;
			obj_args->items[ new PyRepString("message") ] 
				= new PyRepString("BrowseIGB");
			PyRepDict *args = new PyRepDict();
			obj_args->items[ new PyRepString("args") ] = args;
				args->items[ new PyRepString("showStatusBar") ] = new PyRepInteger(0);
				args->items[ new PyRepString("center") ] = new PyRepInteger(1);
				args->items[ new PyRepString("showAddressBar") ] = new PyRepInteger(0);
				args->items[ new PyRepString("url") ] = new PyRepString(
					"http://www.eve-online.com/igb/login/?username=aaa&total"
					"Trial=1858&trialMax=3000&createDate=2006.07.04&daysLeft=8&"
					"trialLen=14&login=yes&totalPlayers=18034&");
				args->items[ new PyRepString("showOptions") ] = new PyRepInteger(0);
				args->items[ new PyRepString("showButtons") ] = new PyRepInteger(0);
				args->items[ new PyRepString("showModal") ] = new PyRepInteger(1);
*/

#ifdef SHOW_LOGIN_MESSAGE
	PyRepDict *args = new PyRepDict;
	args->add("message", loginMessage);
	result = new PyRepObject("util.KeyVal", args);
#else /* !SHOW_LOGIN_MESSAGE */
	result = new PyRepNone;
#endif /* !SHOW_LOGIN_MESSAGE */

	return result;
}



















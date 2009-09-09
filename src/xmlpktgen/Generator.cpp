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


#include "common.h"
#include "Generator.h"
#include "../common/logsys.h"


void Generator::Generate(FILE *into, TiXmlElement *root) {
	TiXmlNode *main_element = NULL;
	while( (main_element = root->IterateChildren( main_element )) ) {
		//pass through comments.
		if(main_element->Type() == TiXmlNode::COMMENT) {
			TiXmlComment *com = main_element->ToComment();
			fprintf(into, "\t/* %s */\n", com->Value());
			continue;
		}
		if(main_element->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		
		TiXmlElement *ele = main_element->ToElement();
		if(std::string("include") == ele->Value()) {
			Process_include(into, ele);
			continue;
		} else if(std::string("element") != ele->Value()) {
			_log(COMMON__ERROR, "Unexpected element '%s' in <Elements> on line %d. Ignore.", ele->Value(), ele->Row());
			continue;
		}
		
		Process_root(into, ele);
	}
}

void Generator::Process_include(FILE *into, TiXmlElement *element) {
}

const char *Generator::GetEncodeType(TiXmlElement *element) const {
	TiXmlElement * main = element->FirstChildElement();
	if(main == NULL)
		return("PyRep");
	std::map<std::string, std::string>::const_iterator res;
	res = m_eleTypes.find(main->Value());
	if(res == m_eleTypes.end())
		return("PyRep");
	return(res->second.c_str());
}


















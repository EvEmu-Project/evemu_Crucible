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

#include "CommonPCH.h"

#include "utils/XMLParser.h"

XMLParser::XMLParser() {
	ParseOkay = false;
}

bool XMLParser::ParseFile(const char *file, const char *root_ele) {
    std::map<std::string,ElementHandler>::iterator handler;
	TiXmlDocument doc( file );
	if(!doc.LoadFile()) {
		printf("Unable to load '%s': %s\n", file, doc.ErrorDesc());
		return false;
	}
	
	TiXmlElement *root = doc.FirstChildElement( root_ele );
	if(root == NULL) {
		printf("Unable to find root '%s' in %s\n",root_ele, file);
		return false;
	}

	ParseOkay=true;
	
	TiXmlNode *main_element = NULL;
	while( (main_element = root->IterateChildren( main_element )) ) {
		if(main_element->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		TiXmlElement *ele = (TiXmlElement *) main_element;

		handler=Handlers.find(ele->Value());
		if (handler!=Handlers.end() && handler->second) {
			ElementHandler h=handler->second;
			
			/*
			 * 
			 * This is kinda a sketchy operation here, since all of these
			 * element handler methods will be functions in child classes.
			 * This essentially causes us to do an un-checkable (and hence
			 * un-handle-properly-able) cast down to the child class. This
			 * WILL BREAK if any children classes do multiple inheritance.
			 * 
			 * 
			 */
			
			(this->*h)(ele);
		} else {
			//unhandled element.... do nothing for now
		}
		
	}
	
	return(ParseOkay);
}

const char *XMLParser::ParseTextBlock(TiXmlNode *within, const char *name, bool optional) {
	TiXmlElement * txt = within->FirstChildElement(name);
	if(txt == NULL) {
		if(!optional) {
			printf("Unable to find a '%s' element on %s element at line %d\n", name, within->Value(), within->Row());
			ParseOkay=false;
		}
		return NULL;
	}
	TiXmlNode *contents = txt->FirstChild();
	if(contents == NULL || contents->Type() != TiXmlNode::TEXT) {
		if(!optional)
			printf("Node '%s' was expected to be a text element in %s element at line %d\n", name, txt->Value(), txt->Row());
		return NULL;
	}
	return(contents->Value());
}

const char *XMLParser::GetText(TiXmlNode *within, bool optional) {
	TiXmlNode *contents = within->FirstChild();
	if(contents == NULL || contents->Type() != TiXmlNode::TEXT) {
		if(!optional) {
			printf("Node was expected to be a text element in %s element at line %d\n", within->Value(), within->Row());
			ParseOkay=false;
		}
		return NULL;
	}
	return(contents->Value());
}


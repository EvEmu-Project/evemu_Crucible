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

#ifndef XMLParser_H
#define XMLParser_H

#include "common.h"
#include "../../dep/tinyxml/tinyxml.h"
#include "../common/types.h"

#include <string>
#include <map>



/*
 * See note in XMLParser::ParseFile() before inheriting this class.
 */
class XMLParser {
public:
	typedef void (XMLParser::*ElementHandler)(TiXmlElement *ele);
	
	XMLParser();
	virtual ~XMLParser() {}
	
	bool ParseFile(const char *file, const char *root_ele);
	bool ParseStatus() const { return ParseOkay; }
	
protected:
	const char *ParseTextBlock(TiXmlNode *within, const char *name, bool optional = false);
	const char *GetText(TiXmlNode *within, bool optional = false);

	std::map<std::string, ElementHandler> Handlers;

	bool ParseOkay;
};


#endif





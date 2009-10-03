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

#ifndef __XMLPACKETGEN_H_INCL__
#define __XMLPACKETGEN_H_INCL__

#include "HeaderGenerator.h"
#include "ConstructGenerator.h"
#include "DestructGenerator.h"
#include "DumpGenerator.h"
#include "EncodeGenerator.h"
#include "DecodeGenerator.h"
#include "CloneGenerator.h"

class XMLPacketGen
{
public:
	XMLPacketGen();
	virtual ~XMLPacketGen();

	bool LoadFile(const char* xml_file);

	bool GenPackets(const char* header, const char* source);

protected:
	/**
	 * Special header wrapper generator, assures there's nothing
	 * but include and elementdef at top-level.
	 */
	class HeaderGenerator
	: public Generator<HeaderGenerator>
	{
	public:
		HeaderGenerator()
		{
			GenProcReg( HeaderGenerator, Elements, NULL );
			GenProcReg( HeaderGenerator, include, NULL );
			GenProcReg( HeaderGenerator, elementdef, NULL );
		}

	protected:
		GenProcDecl( Elements )
		{
			return Recurse( into, field );
		}
		GenProcDecl( include )
		{
			const char *file = field->Attribute("file");
			if(file == NULL)
			{
				_log(COMMON__ERROR, "field at line %d is missing the file attribute, skipping.", field->Row());
				return false;
			}

			fprintf(into, "#include \"%s\"\n", file);
			return true;
		}
		GenProcDecl( elementdef )
		{
			return mHeader.Generate( into, field );
		}

		ClassHeaderGenerator mHeader;
	};

	/**
	 * Special source wrapper generator, duplicates each field
	 * to all of source generators.
	 */
	class SourceGenerator
	: public Generator<SourceGenerator>
	{
	public:
		SourceGenerator()
		{
			GenProcReg( SourceGenerator, Elements, NULL );
			GenProcReg( SourceGenerator, include, NULL );
			GenProcReg( SourceGenerator, elementdef, NULL );
		}

	protected:
		GenProcDecl( Elements )
		{
			return Recurse( into, field );
		}
		GenProcDecl( include )
		{
			//ignore
			return true;
		}
		GenProcDecl( elementdef )
		{
			return (    mConstruct.Generate( into, field )
					 && mDestruct.Generate( into, field )
					 && mDump.Generate( into, field )
					 && mDecode.Generate( into, field )
					 && mEncode.Generate( into, field )
					 && mClone.Generate( into, field )     );
		}

		ClassConstructGenerator	mConstruct;
		ClassDestructGenerator	mDestruct;
		ClassDumpGenerator		mDump;
		ClassEncodeGenerator	mEncode;
		ClassDecodeGenerator	mDecode;
		ClassCloneGenerator		mClone;
	};

	TiXmlDocument mDoc;

	static const char* const s_mGenFileComment;
	static HeaderGenerator   s_mHeaderGenerator;
    static SourceGenerator   s_mSourceGenerator;

	static std::string FNameToDef(const char* buf);
};

#endif//__XMLPACKETGEN_H_INCL__

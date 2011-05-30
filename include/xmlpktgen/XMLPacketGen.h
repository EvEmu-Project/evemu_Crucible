/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2011 The EVEmu Team
	For the latest information visit http://evemu.org
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

/**
 * @brief XML Packet Generator class.
 *
 * Top-level class, controls the generation.
 *
 * @author Zhur, Bloody.Rabbit
 */
class XMLPacketGen
: public XMLParserEx
{
public:
    /**
     * @brief Primary constructor.
     *
     * @param[in] header Header file to be generated.
     * @param[in] source Source file to be generated.
     */
    XMLPacketGen( const char* header = "", const char* source = "" );
    /**
     * @brief Destructory; closes output files.
     */
    ~XMLPacketGen();

    /**
     * @brief Sets header file.
     *
     * @param[in] header Header file to be generated.
     */
    void SetHeaderFile( const char* header );

    /**
     * @brief Sets source file.
     *
     * @param[in] source Source file to be generated.
     */
    void SetSourceFile( const char* source );

protected:
    bool ParseElements( const TiXmlElement* field );
    bool ParseInclude( const TiXmlElement* field );
    bool ParseElementDef( const TiXmlElement* field );

private:
    /**
     * @brief Opens output files.
     *
     * @retval true  Open succeeded.
     * @retval false Open failed.
     */
    bool OpenFiles();

    FILE*       mHeaderFile;
    std::string mHeaderFileName;
    FILE*       mSourceFile;
    std::string mSourceFileName;

	ClassCloneGenerator		mClone;
	ClassConstructGenerator	mConstruct;
	ClassDecodeGenerator	mDecode;
	ClassDestructGenerator	mDestruct;
	ClassDumpGenerator		mDump;
	ClassEncodeGenerator	mEncode;
	ClassHeaderGenerator    mHeader;

	static std::string FNameToDef( const char* buf );

    static const char* const smGenFileComment;
};

#endif//__XMLPACKETGEN_H_INCL__

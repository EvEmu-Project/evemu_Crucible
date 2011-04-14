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

#ifndef __GENERATOR_H_INCL__
#define __GENERATOR_H_INCL__

/**
 * @brief Generic class for xmlpktgen's generators.
 *
 * All children classes must call RegisterProcessors
 * during construction!
 *
 * @author Zhur, Bloody.Rabbit
 */
class Generator
: public XMLParserEx
{
public:
    /**
     * @brief Primary constructor.
     *
     * @param[in] outputFile The output file.
     */
    Generator( FILE* outputFile = NULL );

    /**
     * @brief Sets output file.
     *
     * @param[in] outputFile New output file.
     */
    void SetOutputFile( FILE* outputFile ) { mOutputFile = outputFile; }

protected:
    /** Registers all the Generator processors. */
    void RegisterProcessors();

    virtual bool ProcessElementDef( const TiXmlElement* field ) = 0;
    virtual bool ProcessElement( const TiXmlElement* field ) = 0;
    virtual bool ProcessElementPtr( const TiXmlElement* field ) = 0;

    virtual bool ProcessRaw( const TiXmlElement* field ) = 0;
    virtual bool ProcessInt( const TiXmlElement* field ) = 0;
    virtual bool ProcessLong( const TiXmlElement* field ) = 0;
    virtual bool ProcessReal( const TiXmlElement* field ) = 0;
    virtual bool ProcessBool( const TiXmlElement* field ) = 0;
    virtual bool ProcessNone( const TiXmlElement* field ) = 0;
    virtual bool ProcessBuffer( const TiXmlElement* field ) = 0;

    virtual bool ProcessString( const TiXmlElement* field ) = 0;
    virtual bool ProcessStringInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessWString( const TiXmlElement* field ) = 0;
    virtual bool ProcessWStringInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessToken( const TiXmlElement* field ) = 0;
    virtual bool ProcessTokenInline( const TiXmlElement* field ) = 0;

    virtual bool ProcessObject( const TiXmlElement* field ) = 0;
    virtual bool ProcessObjectInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessObjectEx( const TiXmlElement* field ) = 0;

    virtual bool ProcessTuple( const TiXmlElement* field ) = 0;
    virtual bool ProcessTupleInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessList( const TiXmlElement* field ) = 0;
    virtual bool ProcessListInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessListInt( const TiXmlElement* field ) = 0;
    virtual bool ProcessListLong( const TiXmlElement* field ) = 0;
    virtual bool ProcessListStr( const TiXmlElement* field ) = 0;
    virtual bool ProcessDict( const TiXmlElement* field ) = 0;
    virtual bool ProcessDictInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessDictRaw( const TiXmlElement* field ) = 0;
    virtual bool ProcessDictInt( const TiXmlElement* field ) = 0;
    virtual bool ProcessDictStr( const TiXmlElement* field ) = 0;

    virtual bool ProcessSubStreamInline( const TiXmlElement* field ) = 0;
    virtual bool ProcessSubStructInline( const TiXmlElement* field ) = 0;

    /**
     * @brief Obtains encode type of given element.
     *
     * @param[in] element The element to be examined.
     *
     * @return The encode type of element.
     */
    static const char* GetEncodeType( const TiXmlElement* element );

    /** The current output file. */
    FILE* mOutputFile;

private:
    /** Loads encode types. */
    static void LoadEncTypes();

    /** True if encode types has been loaded. */
    static bool smEncTypesLoaded;
    /** Encode type map. */
    static std::map<std::string, std::string> smEncTypes;
};

#endif



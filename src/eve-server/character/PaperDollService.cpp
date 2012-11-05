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
    Author:     caytchen
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "character/PaperDollService.h"

PyCallable_Make_InnerDispatcher(PaperDollService)

PaperDollService::PaperDollService(PyServiceMgr* mgr)
: PyService(mgr, "paperDollServer"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PaperDollService, GetPaperDollData)
    PyCallable_REG_CALL(PaperDollService, ConvertAndSavePaperDoll)
    PyCallable_REG_CALL(PaperDollService, UpdateExistingCharacterFull)
    PyCallable_REG_CALL(PaperDollService, UpdateExistingCharacterLimited)
    PyCallable_REG_CALL(PaperDollService, GetPaperDollPortraitDataFor)
    PyCallable_REG_CALL(PaperDollService, GetMyPaperDollData)
}

PaperDollService::~PaperDollService() {
    delete m_dispatch;
}

PyResult PaperDollService::Handle_GetPaperDollData(PyCallArgs &call) {
    return new PyList;
}

PyResult PaperDollService::Handle_ConvertAndSavePaperDoll(PyCallArgs &call) {
    return NULL;
}

PyResult PaperDollService::Handle_UpdateExistingCharacterFull(PyCallArgs &call) {
    return NULL;
}

PyResult PaperDollService::Handle_UpdateExistingCharacterLimited(PyCallArgs &call) {
    return NULL;
}

PyResult PaperDollService::Handle_GetPaperDollPortraitDataFor(PyCallArgs &call) {
    return NULL;
}

PyResult PaperDollService::Handle_GetMyPaperDollData(PyCallArgs &call)
{
    DBRowDescriptor* colorsHeader = new DBRowDescriptor();
	colorsHeader->AddColumn("colorID", DBTYPE_I4);
	colorsHeader->AddColumn("colorNameA", DBTYPE_I4);
	colorsHeader->AddColumn("colorNameBC", DBTYPE_I4);
	colorsHeader->AddColumn("weight", DBTYPE_R4);
	colorsHeader->AddColumn("gloss", DBTYPE_R4);
	CRowSet* colorsRowset = new CRowSet(&colorsHeader);

	DBRowDescriptor* modifiersHeader = new DBRowDescriptor();
	modifiersHeader->AddColumn("modifierLocationID", DBTYPE_I4);
	modifiersHeader->AddColumn("paperdollResourceID", DBTYPE_I4);
	modifiersHeader->AddColumn("paperdollResourceVariation", DBTYPE_I4);
	CRowSet* modifiersRowset = new CRowSet(&modifiersHeader);
	
	DBRowDescriptor* appearanceHeader = new DBRowDescriptor();
	appearanceHeader->AddColumn("lastUpdate", DBTYPE_FILETIME);
	appearanceHeader->AddColumn("hairDarkness", DBTYPE_R4);
	CRowSet* appearanceRowset = new CRowSet(&appearanceHeader);

	DBRowDescriptor* sculptsHeader = new DBRowDescriptor();
	sculptsHeader->AddColumn("sculptLocationID", DBTYPE_I4);
	sculptsHeader->AddColumn("weightUpDown", DBTYPE_R4);
	sculptsHeader->AddColumn("weightLeftRight", DBTYPE_R4);
	sculptsHeader->AddColumn("weightForwardBack", DBTYPE_R4);
	CRowSet* sculptsRowset = new CRowSet(&sculptsHeader);

	//the below values work with amarr woman -- just for testing
	//colors
	PyPackedRow* skinTone = colorsRowset->NewRow();

	skinTone->SetField("colorID", new PyInt(5));
	skinTone->SetField("colorNameA", new PyInt(127));
	skinTone->SetField("colorNameBC", new PyInt(0));
	skinTone->SetField("weight", new PyFloat(1));
	skinTone->SetField("gloss", new PyFloat(1));
	
	
	PyPackedRow* lipstick = colorsRowset->NewRow();

	lipstick->SetField("colorID", new PyInt(8));
	lipstick->SetField("colorNameA", new PyInt(156));
	lipstick->SetField("colorNameBC", new PyInt(0));
	lipstick->SetField("weight", new PyFloat(1));
	lipstick->SetField("gloss", new PyFloat(1));
	
	//modifiers
	PyPackedRow* makeup = modifiersRowset->NewRow();

	makeup->SetField("modifierLocationID", new PyInt(3));
	makeup->SetField("paperdollResourceID", new PyInt(285));
	makeup->SetField("paperdollResourceVariation", new PyInt(0));

	PyPackedRow* hairModifier = modifiersRowset->NewRow();

	hairModifier->SetField("modifierLocationID", new PyInt(5));
	hairModifier->SetField("paperdollResourceID", new PyInt(50));
	hairModifier->SetField("paperdollResourceVariation", new PyInt(0));

	PyPackedRow* lipstickModifier = modifiersRowset->NewRow();

	lipstickModifier->SetField("modifierLocationID", new PyInt(7));
	lipstickModifier->SetField("paperdollResourceID", new PyInt(176));
	lipstickModifier->SetField("paperdollResourceVariation", new PyInt(0));

	PyPackedRow* glasses = modifiersRowset->NewRow();

	glasses->SetField("modifierLocationID", new PyInt(21));
	glasses->SetField("paperdollResourceID", new PyInt(310));
	glasses->SetField("paperdollResourceVariation", new PyInt(0));

	PyPackedRow* coat = modifiersRowset->NewRow();

	coat->SetField("modifierLocationID", new PyInt(10));
	coat->SetField("paperdollResourceID", new PyInt(302));
	coat->SetField("paperdollResourceVariation", new PyInt(0));
	
	PyPackedRow* unknown = modifiersRowset->NewRow();

	unknown->SetField("modifierLocationID", new PyInt(121));
	unknown->SetField("paperdollResourceID", new PyInt(1023));
	unknown->SetField("paperdollResourceVariation", new PyInt(0));

	PyPackedRow* unknown1 = modifiersRowset->NewRow();

	unknown1->SetField("modifierLocationID", new PyInt(122));
	unknown1->SetField("paperdollResourceID", new PyInt(1033));
	unknown1->SetField("paperdollResourceVariation", new PyInt(0));
	
	//sculpts
	PyPackedRow* tits = sculptsRowset->NewRow();

	tits->SetField("sculptLocationID", new PyInt(5));
	tits->SetField("weightUpDown", new PyFloat(0));
	tits->SetField("weightLeftRight", new PyFloat(0));
	tits->SetField("weightForwardBack", new PyFloat(1));
	

	PyDict* args = new PyDict;

	args->SetItemString( "colors", colorsRowset );
    args->SetItemString( "modifiers", modifiersRowset );
	args->SetItemString( "appearance", appearanceRowset );
	args->SetItemString( "sculpts", sculptsRowset );

    return new PyObject("util.KeyVal", args);
}

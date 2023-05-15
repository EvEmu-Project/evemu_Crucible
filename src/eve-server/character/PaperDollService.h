/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        caytchen
*/

#ifndef __PAPERDOLLSERVICE__H__INCL__
#define __PAPERDOLLSERVICE__H__INCL__

#include "services/Service.h"
#include "character/PaperDollDB.h"
#include "Client.h"

/**
 * \class PaperDollService
 *
 * @brief Deals with character image stuff
 *
 * Introduced with the sculpting and nextgen character creation. Seems to be only used for _recustomization_ of existing characters, so not a priority right now.
 *
 * @author caytchen
 * @date April 2011
 */
class PaperDollService : public Service <PaperDollService> {
public:
    PaperDollService();

protected:
    PyResult GetPaperDollData(PyCallArgs& call, PyInt* characterID);
    PyResult ConvertAndSavePaperDoll(PyCallArgs& call);
    PyResult UpdateExistingCharacterFull(PyCallArgs& call, PyInt* characterID, PyRep* dollInfo, PyRep* portraitInfo, PyBool* dollExists);
    PyResult UpdateExistingCharacterLimited(PyCallArgs& call, PyInt* characterID, PyRep* dollData, PyRep* portraitInfo, PyBool* dollExists);
    PyResult GetPaperDollPortraitDataFor(PyCallArgs& call, PyInt* characterID);
    PyResult GetMyPaperDollData(PyCallArgs& call, PyInt* characterID);

private:
	PaperDollDB m_db;
};

#endif // __PAPERDOLLSERVICE__H__INCL__
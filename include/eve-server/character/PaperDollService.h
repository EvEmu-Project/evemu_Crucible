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
    Author:        caytchen
*/

#ifndef __PAPERDOLLSERVICE__H__INCL__
#define __PAPERDOLLSERVICE__H__INCL__

#include "PyService.h"
#include "character/PaperDollDB.h"
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
class PaperDollService : public PyService {
public:
    PaperDollService(PyServiceMgr* mgr);
    virtual ~PaperDollService();

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;

	PaperDollDB m_db;

    PyCallable_DECL_CALL(GetPaperDollData)
    PyCallable_DECL_CALL(ConvertAndSavePaperDoll)
    PyCallable_DECL_CALL(UpdateExistingCharacterFull)
    PyCallable_DECL_CALL(UpdateExistingCharacterLimited)
    PyCallable_DECL_CALL(GetPaperDollPortraitDataFor)
    PyCallable_DECL_CALL(GetMyPaperDollData)
};

#endif // __PAPERDOLLSERVICE__H__INCL__
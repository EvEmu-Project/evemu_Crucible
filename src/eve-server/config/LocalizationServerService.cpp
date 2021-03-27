/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        ozatomic
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "config/LocalizationServerService.h"

PyCallable_Make_InnerDispatcher(LocalizationServerService)

LocalizationServerService::LocalizationServerService( PyServiceMgr *mgr )
: PyService(mgr, "localizationServer"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(LocalizationServerService, GetAllTextChanges);
    PyCallable_REG_CALL(LocalizationServerService, UpdateLocalizationQASettings);

}

LocalizationServerService::~LocalizationServerService() {
    delete m_dispatch;
}

PyResult LocalizationServerService::Handle_GetAllTextChanges(PyCallArgs &call)
{
/*
                cacheData = sm.RemoteSvc('localizationServer').GetAllTextChanges(hashData)
                localization.LogInfo('Localization Client: done asking for initial text and label data from server')
                if cacheData is not None:
                    localization.LogInfo('Localization Client: updating internal memory with new text and labels.')
                    messagePerLanguage, metaDataPerLanguage, labelsDict = cacheData
            */
    return new PyNone();
}

PyResult LocalizationServerService::Handle_UpdateLocalizationQASettings(PyCallArgs &call)
{
    /*
     *     sm.RemoteSvc('localizationServer').UpdateLocalizationQASettings(showMessageID=showMessageID, enableBoundaryMarkers=enableBoundaryMarkers)
     * sm.RemoteSvc('localizationServer').UpdateLocalizationQASettings(showHardcodedStrings=prefs.GetValue('showHardcodedStrings', 0), showMessageID=prefs.GetValue('showMessageID', 0), enableBoundaryMarkers=prefs.GetValue('enableBoundaryMarkers', 0), characterReplacementMethod=prefs.GetValue('characterReplacementMethod', 0), enableTextExpansion=prefs.GetValue('enableTextExpansion', 0))
     */

    return new PyNone();
}

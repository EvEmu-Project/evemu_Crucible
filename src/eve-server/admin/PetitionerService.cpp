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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/PetitionerService.h"


PyCallable_Make_InnerDispatcher(PetitionerService)

PetitionerService::PetitionerService(PyServiceMgr *mgr)
: PyService(mgr, "petitioner"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PetitionerService, GetCategories);
    PyCallable_REG_CALL(PetitionerService, GetCategoryHierarchicalInfo);
    PyCallable_REG_CALL(PetitionerService, GetUnreadMessages);
}

PetitionerService::~PetitionerService() {
    delete m_dispatch;
}


/*
 *
 *
        catCountry = sm.RemoteSvc('petitioner').GetUserCatalogCountry()

        can = sm.RemoteSvc('petitioner').MayPetition(categoryID, OocCharacterID)

        self.properties = sm.RemoteSvc('petitioner').GetCategoryProperties(self.category[0])
        populationInfo = sm.RemoteSvc('petitioner').PropertyPopulationInfo(property.inputInfo, self.OocCharacterID)
        populationRecords = sm.RemoteSvc('petitioner').GetClientPickerInfo(filterString, elementName)
                queues = sm.RemoteSvc('petitioner').GetQueues()

                self.categories = sm.RemoteSvc('petitioner').GetCategories()
                sm.RemoteSvc('petitioner').CreatePetition(subject, petition, categoryID, retval, OocCharacterID, combatLog=combatLog, chatLog=chatLog):
                sm.RemoteSvc('petitioner').CreatePetition(subject, petition, categoryID, None, self.OocCharacterID, chatLog, combatLog, propertyList)

        sm.RemoteSvc('petitioner').MarkAsRead(messageID)

        newMessages = sm.RemoteSvc('petitioner').GetUnreadMessages()
        self.NewMessage(newMessages[0].petitionID, newMessages[0].text, newMessages[0].messageID)

        self.mine = sm.RemoteSvc('petitioner').GetMyPetitionsEx()

        sm.RemoteSvc('petitioner').EscalatePetition(petitionid, escalatesTo)
        sm.RemoteSvc('petitioner').ClaimPetition(p.petitionID)
        sm.RemoteSvc('petitioner').UnClaimPetition(petitionid)
        mp = sm.RemoteSvc('petitioner').GetClaimedPetitions()
        for p in mp:
            if p.petitionerID not in owners:


        parentCategoryDict, childCategoryDict, descriptionDict, self.billingCategories = sm.RemoteSvc('petitioner').GetCategoryHierarchicalInfo()

        sm.RemoteSvc('petitioner').UpdatePetitionRating(p.petitionID, responseTimeRating, helpfulnessRating, attitudeRating, newComment)

        sm.RemoteSvc('petitioner').AddPetitionRating(p.petitionID, responseTimeRating, helpfulnessRating, attitudeRating, newComment, wnd.sr.ratingtime)


        mp = sm.RemoteSvc('petitioner').GetPetitionQueue(queueID)
        for p in mp:
            if p.petitionerID and p.petitionerID not in owners:
 *

 pmsgs = sm.RemoteSvc('petitioner').GetPetitionMessages(p.petitionID)
 for pm in pmsgs:
     if pm.senderID is not None and pm.senderID not in owners:
 *

 plogs = sm.RemoteSvc('petitioner').GetLog(p.petitionID)
 texts = sm.RemoteSvc('petitioner').GetEvents()
 *

 sm.RemoteSvc('petitioner').PetitioneeChat(petitionid, message, comment)
 sm.RemoteSvc('petitioner').PetitionerChat(petitionid, message)


 sm.RemoteSvc('petitioner').CancelPetition(petitionid)
 sm.RemoteSvc('petitioner').ClosePetition(petitionid)
 */


PyResult PetitionerService::Handle_GetCategories( PyCallArgs& call )
{
    uint8 size(call.tuple->size());
    sLog.White( "PetitionerService::Handle_GetCategories()", "size=%u ", size );

    PyList* result = new PyList();
    result->AddItemString( "Test Cat" );
    result->AddItemString( "Test Cat2" );

    return result;
}

PyResult PetitionerService::Handle_GetCategoryHierarchicalInfo( PyCallArgs& call )
{
    uint8 size(call.tuple->size());
    sLog.White( "PetitionerService::Handle_GetCategoryHierarchicalInfo()", "size=%u ", size );

    return new PyList();
}

//00:28:58 L PetitionerService::Handle_GetUnreadMessages(): size=0
PyResult PetitionerService::Handle_GetUnreadMessages( PyCallArgs& call )
{
    //unknown...
    return new PyList();
}

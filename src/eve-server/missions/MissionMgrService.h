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
    Author:        Zhur
*/
#ifndef __MISSIONMGR_SERVICE_H_INCL__
#define __MISSIONMGR_SERVICE_H_INCL__

#include "services/Service.h"
#include "missions/MissionDB.h"

class MissionMgrService : public Service <MissionMgrService>
{
public:
    MissionMgrService();

protected:
    //MissionDB m_db;
    PyResult GetMyCourierMissions(PyCallArgs& call);
};

#endif

/*{'messageKey': 'AgtBlackMarketMissingCredits', 'dataID': 13413909, 'suppressable': False, 'bodyID': None, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 8}
 * {'messageKey': 'AgtBlackMarketMissingItems', 'dataID': 8704427, 'suppressable': False, 'bodyID': None, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 9}
 * {'messageKey': 'AgtBlackMarketNothingOffered', 'dataID': 17885408, 'suppressable': False, 'bodyID': 260223, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260222, 'messageID': 10}
 * {'messageKey': 'AgtCompleteMissionMissingCollateral', 'dataID': 17885971, 'suppressable': False, 'bodyID': 260434, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260433, 'messageID': 11}
 * {'messageKey': 'AgtCompleteMissionUnfetchedItem', 'dataID': 17885725, 'suppressable': False, 'bodyID': 260343, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260342, 'messageID': 13}
 * {'messageKey': 'AgtCompleteMissionUnkilledNPC', 'dataID': 17885730, 'suppressable': False, 'bodyID': 260345, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260344, 'messageID': 14}
 * {'messageKey': 'AgtConfirmDeclineMission', 'dataID': 17885976, 'suppressable': False, 'bodyID': 260436, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 260435, 'messageID': 15}
 * {'messageKey': 'AgtInteractLocked', 'dataID': 17885735, 'suppressable': False, 'bodyID': 260347, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260346, 'messageID': 16}
 * {'messageKey': 'AgtMissionNoMissionDone', 'dataID': 17885413, 'suppressable': False, 'bodyID': 260225, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260224, 'messageID': 17}
 * {'messageKey': 'AgtMissionNoneOffered', 'dataID': 17885418, 'suppressable': False, 'bodyID': 260227, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260226, 'messageID': 18}
 * {'messageKey': 'AgtMissionOfferWarning', 'dataID': 17885720, 'suppressable': True, 'bodyID': 260341, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 260340, 'messageID': 19}
 * {'messageKey': 'AgtNotAnAgent4', 'dataID': 17885765, 'suppressable': False, 'bodyID': 260359, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260358, 'messageID': 21}
 * {'messageKey': 'AgtYouHaveAlreadyAcceptedTheMission', 'dataID': 17885423, 'suppressable': False, 'bodyID': 260229, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 260228, 'messageID': 22}
 * 
 * {'messageKey': 'MisMissionExpired', 'dataID': 17882915, 'suppressable': False, 'bodyID': 259310, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259309, 'messageID': 1176}
 * {'messageKey': 'MisNoItems', 'dataID': 17882920, 'suppressable': False, 'bodyID': 259312, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259311, 'messageID': 1177}
 * {'messageKey': 'MisNoShipsInCourier', 'dataID': 17882925, 'suppressable': False, 'bodyID': 259314, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259313, 'messageID': 1178}
 * {'messageKey': 'MisNoSuchMission', 'dataID': 17882930, 'suppressable': False, 'bodyID': 259316, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259315, 'messageID': 1179}
 * {'messageKey': 'MisNotAvailable', 'dataID': 17882933, 'suppressable': False, 'bodyID': 259317, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1180}
 * {'messageKey': 'MisPackageNotHere', 'dataID': 17882938, 'suppressable': False, 'bodyID': 259319, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259318, 'messageID': 1181}
 * {'messageKey': 'MisPilotOnBoard', 'dataID': 17882941, 'suppressable': False, 'bodyID': 259320, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1183}
 * {'messageKey': 'MisRewardLessAccept', 'dataID': 17882946, 'suppressable': False, 'bodyID': 259322, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259321, 'messageID': 1184}
 * {'messageKey': 'MissDroppedOffCredit', 'dataID': 13413911, 'suppressable': False, 'bodyID': None, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1186}
 * {'messageKey': 'MissDroppedOffCredits', 'dataID': 13413913, 'suppressable': False, 'bodyID': None, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1187}
 * {'messageKey': 'MissMissionAcceptedHelp', 'dataID': 17882951, 'suppressable': True, 'bodyID': 259324, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259323, 'messageID': 1189}
 * {'messageKey': 'MissObjectiveCompleted', 'dataID': 17883253, 'suppressable': False, 'bodyID': 259438, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259437, 'messageID': 1191}
 * {'messageKey': 'MissingRequiredClothing', 'dataID': 17876041, 'suppressable': False, 'bodyID': 256730, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3484}
 * {'messageKey': 'MissingRequiredField', 'dataID': 17883145, 'suppressable': False, 'bodyID': 259399, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259398, 'messageID': 1196}
 * {'messageKey': 'MissingRoleStationMgt', 'dataID': 17880810, 'suppressable': False, 'bodyID': 258535, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258534, 'messageID': 1941}
 * {'messageKey': 'MissionEnter', 'dataID': 2987079, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': 'wise:/msg_MissionEnter_play', 'urlIcon': '', 'titleID': None, 'messageID': 1197}
 * {'messageKey': 'MissionItemAlreadyThere', 'dataID': 17882956, 'suppressable': False, 'bodyID': 259326, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259325, 'messageID': 1199}
 * {'messageKey': 'MissionNotIssued', 'dataID': 17882959, 'suppressable': False, 'bodyID': 259327, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1201}
 * {'messageKey': 'MissionSelect', 'dataID': 2987080, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': 'wise:/msg_MissionSelect_play', 'urlIcon': '', 'titleID': None, 'messageID': 1202}
 * {'messageKey': 'NoValidItemForMission', 'dataID': 17881127, 'suppressable': False, 'bodyID': 258659, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258658, 'messageID': 1947}
 * {'messageKey': 'NotYourMission', 'dataID': 17882089, 'suppressable': False, 'bodyID': 259004, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1359}
 * {'messageKey': 'NotYourMissionToAbort', 'dataID': 17882092, 'suppressable': False, 'bodyID': 259005, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1360}
 *
 */
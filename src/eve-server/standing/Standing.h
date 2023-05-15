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
    Updates:        Allan
*/


#ifndef __STANDING2_SERVICE_H_INCL__
#define __STANDING2_SERVICE_H_INCL__

#include "services/Service.h"

#include "cache/ObjCacheService.h"
#include "StandingMgr.h"
#include "inventory/ItemFactory.h"
#include "character/Character.h"
#include "inventory/ItemRef.h"

class Standing : public Service <Standing>
{
public:
    Standing();

protected:
    StandingDB m_db;

    PyResult GetCharStandings(PyCallArgs& call);
    PyResult GetCorpStandings(PyCallArgs& call);
    PyResult GetNPCNPCStandings(PyCallArgs& call);
    PyResult GetSecurityRating(PyCallArgs& call, PyInt* ownerID);
    PyResult GetMyKillRights(PyCallArgs& call);
    PyResult GetStandingTransactions(PyCallArgs& call, PyInt* fromID, PyInt* toID, PyInt* direction, PyInt* eventID, PyInt* eventType, PyLong* eventDateTime);
    PyResult GetStandingCompositions(PyCallArgs& call, PyInt* fromID, PyInt* toID);
};

#endif

/*
(235078, `Standing Decay`)
(235079, `All standings decay by a certain amount on a regular basis.
An exception to this rule is made for players that haven't logged in recently.`)
(235080, `Derived Modification`)
(235081, `{name} was pleased by actions on you performed for {name}'s friends, or against {name}'s enemies`)
(235082, `Derived Modification`)
(235083, `{name} was displeased by actions on you performed against {name}'s friends, or for {name}'s enemies.`)
(235084, `Combat - Aggression`)
(235085, `This penalty was incurred for attacking {ownerName}'s {[item]typeID.name} in {[location]locationID.name}`)
(235086, `Combat - Assistance`)
(235087, `This penalty was incurred for Assisting {name}'s {[item]typeID.name} in {[location]locationID.name} `)
(235088, `Combat - Ship Kill`)
(235089, `This penalty was incurred for destroying {name}'s {[item]typeID.name} in {[location]locationID.name}`)
(235090, `Property Damage`)
(235091, `This penalty was incurred for destroying {name}'s {[item]typeID.name} in {[location]locationID.name}`)
(235092, `Combat - Pod Kill`)
(235093, `This penalty was incurred for podding {name} in {locationName}`)
(235094, `GM Intervention`)
(235095, `This modification was performed directly by {name}. The reason specified was: {message}`)
(235096, `This standing was reset by a GM.`)
(235097, `Player Set`)
(235098, `This player standing was set by the player himself/herself. The reason specified was: {message}`)
(235099, `Corp Set`)
(235100, `This player corp standing was set by {name}. The reason specified was: {message}`)
(235101, `Mission Completed - {message}`)
(235102, `This standing increase was granted for the successful completion of the mission '{message}'`)
(235103, `Mission Declined - {message}`)
(235104, `This standing penalty was incurred by declining the mission '{message}'`)
(235105, `Mission Failed - {message}`)
(235106, `This standing penalty was incurred for failing mission '{message}'`)
(235107, `Mission Offer Expired - {message}`)
(235108, `This standing penalty was incurred for not accepting a mission`)
(235109, `This standing penalty was incurred for not accepting mission '{message}'`)
(235110, `Mission Bonus - {message}`)
(235111, `This standing change was granted as a bonus within the mission '{message}'`)
(235112, `Mission Penalty - {message}`)
(235113, `This standing change was granted as a penalty within the mission '{message}'`)
(235114, `Law Enforcement - Security Status Gain`)
(235115, `This standing change was granted by CONCORD as an award for actions performed against {name}`)
(235116, `Promotion`)
(235117, `This standing change was granted for promotion to rank {rankName} within the {corpName}`)
(235118, `This penalty was incurred for destroying {[item]typeID.name} belonging to a member of the {factionName} in {[location]locationID.name}`)
(235119, `This penalty was incurred for podding a member of the {factionName} in {[location]locationID.name}`)
(235120, `This penalty was incurred for attacking {[item]typeID.name} belonging to a member of the {factionName} in {[location]locationID.name}`)
(235121, `This penalty was incurred for assisting {[item]typeID.name} belonging to a member of the {factionName} when attacking a member of your militia in {[location]locationID.name}`)
(235122, `This penalty was incurred for destroying {[item]typeID.name} belonging to a member of the {factionName} in {[location]locationID.name}`)
(235123, `{[timeinterval]time.writtenForm, from=hour, to=second} ago`)
(235124, `{[timeinterval]time.writtenForm, from=day, to=second} ago`)
(235125, `Right Now`)
(235127, `Tactical site defended`)
(235128, `This standing change was granted for helping the {factionName} defend a tactical site from the {enemyFactionName}`)
(235129, `Tactical site conquered`)
(235130, `This standing change was granted for helping the {factionName} conquer a tactical site from the {enemyFactionName}`)
(235131, `Letter of Recommendation used`)
(235132, `A Letter of Recommendation used to instantly join the war effort for this faction.`)
(235133, `Standing Change`)
(235134, `This standing increase was granted as part of the capsuleer graduation process.`)
(235135, `Contraband`)
(235136, `Your standing was lowered with the {factionName} for transporting contraband in {systemName}.`)

*/

/*
 * def GetStandingEventTypes():
 *    return [(localization.GetByLabel('UI/Generic/StandingNames/agentByoff'), logConst.eventAgentBuyOff),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentDonation'), logConst.eventAgentDonation),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionBonus'), logConst.eventAgentMissionBonus),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionComplete'), logConst.eventAgentMissionCompleted),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionDeclined'), logConst.eventAgentMissionDeclined),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionFailed'), logConst.eventAgentMissionFailed),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionExpired'), logConst.eventAgentMissionOfferExpired),
 *     (localization.GetByLabel('UI/Generic/StandingNames/combatAgression'), logConst.eventCombatAggression),
 *     (localization.GetByLabel('UI/Generic/StandingNames/combatOther'), logConst.eventCombatOther),
 *     (localization.GetByLabel('UI/Generic/StandingNames/combatPodKill'), logConst.eventCombatPodKill),
 *     (localization.GetByLabel('UI/Generic/StandingNames/combatShipKill'), logConst.eventCombatShipKill),
 *     (localization.GetByLabel('UI/Generic/StandingNames/decay'), logConst.eventDecay),
 *     (localization.GetByLabel('UI/Generic/StandingNames/derivedNegitive'), logConst.eventDerivedModificationNegative),
 *     (localization.GetByLabel('UI/Generic/StandingNames/derivedPositive'), logConst.eventDerivedModificationPositive),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentInital'), logConst.eventInitialCorpAgent),
 *     (localization.GetByLabel('UI/Generic/StandingNames/factionInital'), logConst.eventInitialFactionAlly),
 *     (localization.GetByLabel('UI/Generic/StandingNames/factionInitalCorp'), logConst.eventInitialFactionCorp),
 *     (localization.GetByLabel('UI/Generic/StandingNames/factionInitalEnemy'), logConst.eventInitialFactionEnemy),
 *     (localization.GetByLabel('UI/Generic/StandingNames/lawEnforcement'), logConst.eventPirateKillSecurityStatus),
 *     (localization.GetByLabel('UI/Generic/StandingNames/playerCorpSetStandings'), logConst.eventPlayerCorpSetStanding),
 *     (localization.GetByLabel('UI/Generic/StandingNames/playerSetStandings'), logConst.eventPlayerSetStanding),
 *     (localization.GetByLabel('UI/Generic/StandingNames/entityRecacluate'), logConst.eventRecalcEntityKills),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionRecaculateFailed'), logConst.eventRecalcMissionFailure),
 *     (localization.GetByLabel('UI/Generic/StandingNames/agentMissionRecaculateSucess'), logConst.eventRecalcMissionSuccess),
 *     (localization.GetByLabel('UI/Generic/StandingNames/entityRecaculatePirateKills'), logConst.eventRecalcPirateKills),
 *     (localization.GetByLabel('UI/Generic/StandingNames/playerRecaculateSetStandings'), logConst.eventRecalcPlayerSetStanding),
 *     (localization.GetByLabel('UI/Generic/StandingNames/GMSlashSet'), logConst.eventSlashSetStanding),
 *     (localization.GetByLabel('UI/Generic/StandingNames/standingReset'), logConst.eventStandingReset),
 *     (localization.GetByLabel('UI/Generic/StandingNames/tutorialInitial'), logConst.eventTutorialAgentInitial),
 *     (localization.GetByLabel('UI/Generic/StandingNames/standingsUpdate'), logConst.eventUpdateStanding)]
 *
 */

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
    Author:     Zhur
*/

#ifndef __EVEUTILS_H__
#define __EVEUTILS_H__

#include "EvilNumber.h"
#include "python/PyRep.h"

class UserError;

//makes ccp_exceptions.UserError exception
//this function is deprecated; use directly UserError class instead.
extern UserError *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args = std::map<std::string, PyRep *>());
//makes UserError with type "CustomError"
extern UserError *MakeCustomError(const char *fmt, ...);

/**
 * @brief Checks whether string is printable.
 *
 * @param[in] str String to be checked.
 *
 * @retval true  The string is printable.
 * @retval false The string is not printable.
 */
bool IsPrintable( const PyString* str );
/**
 * @brief Checks whether string is printable.
 *
 * @param[in] str String to be checked.
 *
 * @retval true  The string is printable.
 * @retval false The string is not printable.
 */
bool IsPrintable( const PyWString* str );

/**
 * Checks compatibility between DBTYPE and PyRep.
 *
 * @param[in] type DBTYPE to check.
 * @param[in] rep PyRep to check.
 * @return True if arguments are compatible, false if not.
 */
bool DBTYPE_IsCompatible( DBTYPE type, const PyRep* rep );


//////////////////////////////////////////////////////////////////////////////////////////
//
// EVE Math Equations for in-game features
// (pulled directly from http://wiki.eve-id.net/Equations)
//
//////////////////////////////////////////////////////////////////////////////////////////

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ME_EffectOnWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor, EvilNumber MaterialEfficiency );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber StationTaxesForReprocessing( EvilNumber CharacterStandingWithStationOwner );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber AgentEffectiveQuality( EvilNumber AgentQuality, EvilNumber NegotiationSkillLevel, EvilNumber AgentPersonalStanding );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber EffectiveStanding( EvilNumber YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber RequiredAgentStanding( EvilNumber AgentLevel, EvilNumber AgentQuality );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber MissionStandingIncrease( EvilNumber BaseMissionIncrease, EvilNumber YourSocialSkillLevel );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber AgentEfficiency( EvilNumber AgentLevel, EvilNumber AgentQuality );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber SkillPointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber SkillPointsPerMinute( EvilNumber EffectivePrimaryAttribute, EvilNumber EffectiveSecondaryAttribute );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber AlignTimeInSeconds( EvilNumber InertiaModifier, EvilNumber Mass );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber SkillStartingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber SkillEndingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow );

#endif

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
EvilNumber ME_EffectOnWaste(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ME_LevelToEliminateWaste(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber WasteSkillBased(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ME_ResearchTime(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber PE_ResearchTime(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber BluePrintCopyTime(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ProductionTime(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber StationTaxesForReprocessing(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber EffectiveRefiningYield(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber BlueprintInventionTime(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber BlueprintInventionChance(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber ResearchPointsPerDay(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber AgentEffectiveQuality(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber EffectiveStanding(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber RequiredAgentStanding(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber MissionStandingIncrease(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber AgentEfficiency(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber SkillPointsAtLevel(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber EffectiveAttribute(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber SkillPointsPerMinute(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber TargetingLockTime(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber AlignTimeInSeconds(  );

/**
 * ?
 *
 * @param[in] ?
 * @return ?
 */
EvilNumber TradeBrokerFee(  );

#endif




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

#include "EVECommonPCH.h"

#include "python/classes/PyExceptions.h"
#include "utils/EVEUtils.h"

#if defined (MSVC)
#include "boost/math/special_functions.hpp"
#define asinh boost::math::asinh
#else
#include <math.h>
#endif

UserError *MakeUserError(const char *exceptionType, const std::map<std::string, PyRep *> &args)
{
	UserError *err = new UserError( exceptionType );

	std::map<std::string, PyRep *>::const_iterator cur, end;
	cur = args.begin();
	end = args.end();
	for(; cur != end; cur++)
		err->AddKeyword( cur->first.c_str(), cur->second );

	return err;
}

UserError* MakeCustomError( const char* fmt, ... )
{
    va_list va;
    va_start( va, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, va );
    assert( str );

    va_end( va );

	UserError* err = new UserError( "CustomError" );
	err->AddKeyword( "error", new PyString( str ) );

    SafeFree( str );

    return err;
}

bool IsPrintable( const PyString* str )
{
    return IsPrintable( str->content() );
}

bool IsPrintable( const PyWString* str )
{
    // how to do it correctly?
    return IsPrintable( str->content() );
}

bool DBTYPE_IsCompatible( DBTYPE type, const PyRep* rep )
{
// Helper macro, checks type and range
#define CheckTypeRangeUnsigned( type, lower_bound, upper_bound ) \
    ( rep->Is##type() && (uint64)rep->As##type()->value() >= lower_bound && (uint64)rep->As##type()->value() <= upper_bound )
#define CheckTypeRange( type, lower_bound, upper_bound ) \
    ( rep->Is##type() && rep->As##type()->value() >= lower_bound && rep->As##type()->value() <= upper_bound )

    if( rep->IsNone() )
        // represents NULL
        return true;

    return true;

    switch( type )
    {
        case DBTYPE_UI8:
        case DBTYPE_CY:
        case DBTYPE_FILETIME:
            return (   CheckTypeRangeUnsigned( Int, 0LL, 0xFFFFFFFFFFFFFFFFLL )
                    || CheckTypeRangeUnsigned( Long, 0LL, 0xFFFFFFFFFFFFFFFFLL )
                    || CheckTypeRangeUnsigned( Float, 0LL, 0xFFFFFFFFFFFFFFFFLL ) );
        case DBTYPE_UI4:
            return (   CheckTypeRangeUnsigned( Int, 0L, 0xFFFFFFFFL )
                    || CheckTypeRangeUnsigned( Long, 0L, 0xFFFFFFFFL )
                    || CheckTypeRangeUnsigned( Float, 0L, 0xFFFFFFFFL ) );
        case DBTYPE_UI2:
            return (   CheckTypeRangeUnsigned( Int, 0, 0xFFFF )
                    || CheckTypeRangeUnsigned( Long, 0, 0xFFFF )
                    || CheckTypeRangeUnsigned( Float, 0, 0xFFFF ) );
        case DBTYPE_UI1:
            return (   CheckTypeRangeUnsigned( Int, 0, 0xFF )
                    || CheckTypeRangeUnsigned( Long, 0, 0xFF )
                    || CheckTypeRangeUnsigned( Float, 0, 0xFF ) );

        case DBTYPE_I8:
            return (   CheckTypeRange( Int, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL )
                    || CheckTypeRange( Long, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL )
                    || CheckTypeRange( Float, -0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL ) );
        case DBTYPE_I4:
            return (   CheckTypeRange( Int, -0x7FFFFFFFL, 0x7FFFFFFFL )
                    || CheckTypeRange( Long, -0x7FFFFFFFL, 0x7FFFFFFFL )
                    || CheckTypeRange( Float, -0x7FFFFFFFL, 0x7FFFFFFFL ) );
        case DBTYPE_I2:
            return (   CheckTypeRange( Int, -0x7FFF, 0x7FFF )
                    || CheckTypeRange( Long, -0x7FFF, 0x7FFF )
                    || CheckTypeRange( Float, -0x7FFF, 0x7FFF ) );
        case DBTYPE_I1:
            return (   CheckTypeRange( Int, -0x7F, 0x7F )
                    || CheckTypeRange( Long, -0x7F, 0x7F )
                    || CheckTypeRange( Float, -0x7F, 0x7F ) );

        case DBTYPE_R8:
            return (   CheckTypeRange( Int, -DBL_MAX, DBL_MAX )
                    || CheckTypeRange( Long, -DBL_MAX, DBL_MAX )
                    || CheckTypeRange( Float, -DBL_MAX, DBL_MAX ) );
        case DBTYPE_R4:
            return (   CheckTypeRange( Int, -FLT_MAX, FLT_MAX )
                    || CheckTypeRange( Long, -FLT_MAX, FLT_MAX )
                    || CheckTypeRange( Float, -FLT_MAX, FLT_MAX ) );

        case DBTYPE_BOOL:
            return rep->IsBool();

        case DBTYPE_BYTES:
            return rep->IsBuffer();

			// this looks like a horrible hack, and it is - but one that is used on live!
			// this works because STR type stuff is just tacked on to the marshal object
        case DBTYPE_STR:
			return true;
            //return rep->IsString();

        case DBTYPE_WSTR:
            return rep->IsWString();
    }

    return false;

#undef CheckTypeRange
#undef CheckTypeRangeUnsigned
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// EVE Math Equations for in-game features
// (pulled directly from http://wiki.eve-id.net/Equations)
//
//////////////////////////////////////////////////////////////////////////////////////////

EvilNumber ME_EffectOnWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor, EvilNumber MaterialEfficiency )
{
    EvilNumber ME_Factor(0.0);

    if( MaterialEfficiency >= 0 )
        ME_Factor = (1.0 / (MaterialEfficiency.get_float() + 1.0));
    else
        ME_Factor = (1.0 - MaterialEfficiency.get_float());

    return (floor(0.5 + (MaterialAmount.get_float() * (BaseWasteFactor.get_float() / 100.0) * ME_Factor.get_float())));
}

EvilNumber ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor )
{
    return (floor(0.02 * BaseWasteFactor.get_float() * MaterialAmount.get_float()));
}

EvilNumber WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency )
{
    return (floor(0.5 + (MaterialAmount.get_float() * ((25.0 - (5.0 * ProductionEfficiency.get_float())) / 100.0))));
}

EvilNumber ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
{
    return (BlueprintBaseResearchTime.get_float() * (1.0 - (0.05 * MetallurgySkillLevel.get_float()))
        * ResearchSlotModifier.get_float() * ImplantModifier.get_float());
}

EvilNumber PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
{
    return (BlueprintBaseResearchTime.get_float() * (1.0 - (0.05 * ResearchSkillLevel.get_float()))
        * ResearchSlotModifier.get_float() * ImplantModifier.get_float());
}

EvilNumber BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier )
{
    return (BlueprintBaseCopyTime.get_float() * (1.0 - (0.05 * ScienceSkillLevel.get_float()))
        * CopySlotModifier.get_float() * ImplantModifier.get_float());
}

EvilNumber ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier )
{
    return (1.0 - (0.04 * IndustrySkillLevel.get_float()) * ImplantModifier.get_float() * ProductionSlotModifier.get_float());
}

EvilNumber ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier )
{
    EvilNumber PE_Factor(0.0);

    if( ProductionEfficiency >= 0.0 )
        PE_Factor = (ProductionEfficiency.get_float() / (1.0 + ProductionEfficiency.get_float()));
    else
        PE_Factor = (ProductionEfficiency.get_float() - 1.0);

    return (BaseProductionTime.get_float()
        * (1.0 - (ProductivityModifier.get_float() / BaseProductionTime.get_float())
        * (PE_Factor.get_float()))
        * ProductionTimeModifier.get_float());
}

EvilNumber StationTaxesForReprocessing( EvilNumber CharacterStandingWithStationOwner )
{
    return (5.0 - 0.75 * CharacterStandingWithStationOwner.get_float());
}

EvilNumber EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel )
{
    return (StationEquipmentYield.get_float() + 0.375 * (1 + (RefiningSkillLevel.get_float() * 0.02))
        * (1 + (RefiningEfficiencySkillLevel.get_float() * 0.04))
        * (1 + (OreSpecificProcessingSkillLevel.get_float() * 0.05)));
}

EvilNumber BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier )
{
    return BlueprintBaseInventionTime * InventionSlotModifier * ImplantModifier;
}

EvilNumber BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier )
{
    return (BaseChance.get_float() * (1+0.01*EncryptionSkillLevel.get_float())
        * (1+(DataCore1SkillLevel.get_float()+DataCore2SkillLevel.get_float())
        * (0.1 / (5 - MetaLevel.get_float())) * DecryptorModifier.get_float()));
}

EvilNumber ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel )
{
    return (Multiplier.get_float() * (1 + (AgentEffectiveQuality.get_float() / 100.0))
        * pow(YourResearchSkillLevel.get_float() + AgentResearchSkillLevel.get_float(),2));
}

EvilNumber AgentEffectiveQuality( EvilNumber AgentQuality, EvilNumber NegotiationSkillLevel, EvilNumber AgentPersonalStanding )
{
    return (AgentQuality.get_float() + (5.0 * NegotiationSkillLevel.get_float()) + AgentPersonalStanding.get_float());
}

EvilNumber EffectiveStanding( EvilNumber YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel )
{
    EvilNumber SkillLevel(0.0);

    if( YourStanding < 0.0 )
        SkillLevel = DiplomacySkillLevel;
    else
        SkillLevel = ConnectionsSkillLevel;

    return (YourStanding.get_float() + ((10.0 - YourStanding.get_float()) * (0.04 * (SkillLevel.get_float()))));
}

EvilNumber RequiredAgentStanding( EvilNumber AgentLevel, EvilNumber AgentQuality )
{
    return (((AgentLevel.get_float() - 1) * 2) + (AgentQuality.get_float()/20.0));
}

EvilNumber MissionStandingIncrease( EvilNumber BaseMissionIncrease, EvilNumber YourSocialSkillLevel )
{
	return (BaseMissionIncrease * (1 + 0.05 * YourSocialSkillLevel.get_float()));
}

EvilNumber AgentEfficiency( EvilNumber AgentLevel, EvilNumber AgentQuality )
{
    return (0.01 * ((8 * AgentLevel) + (0.1 * AgentQuality) - 4));
}

EvilNumber SkillPointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank )
{
    return (pow( 2, (2.5 * SkillLevel.get_float()) - 2.5 ) * 250.0 * SkillRank);
}

EvilNumber EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus )
{
    return (BaseAttribute + ImplantAttributeBonus);
}

EvilNumber SkillPointsPerMinute( EvilNumber EffectivePrimaryAttribute, EvilNumber EffectiveSecondaryAttribute )
{
    return (EffectivePrimaryAttribute + (0.5 * EffectiveSecondaryAttribute));
}

EvilNumber TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius )
{
    return (40000.0 / (YourEffectiveScanResolution.get_float() * pow(asinh(TargetEffectiveSignatureRadius.get_float()),2)));
}

EvilNumber AlignTimeInSeconds( EvilNumber InertiaModifier, EvilNumber Mass )
{
    return ((log(2.0) * InertiaModifier * Mass) / 500000);
}

EvilNumber TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding )
{
    return (100.0 * ((0.01 - 0.0005 * BrokerRelationsSkillLevel.get_float())
        / (pow( 2, (0.14 * FactionStanding.get_float() + 0.06 * CorporationStanding.get_float()) ))));
}

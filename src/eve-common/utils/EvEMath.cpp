//////////////////////////////////////////////////////////////////////////////////////////
//
// EVE Math Equations for in-game features
// (pulled directly from client code and http://wiki.eve-id.net/Equations)
//
// Latest Update:       Allan  13Nov20
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "../eve-common.h"

double EvEMath::Units::MetersToLightYears(double meters) {
    return meters / 9460730472580800;
}

// skill Equations
uint32 EvEMath::Skill::PointsAtLevel(uint8 level, float rank)
{
    if (level > EvESkill::MAXSKILLLEVEL)
        level = EvESkill::MAXSKILLLEVEL;
    float ret = pow(sqrt(32), (level -1)) * EvESkill::skillPointMultiplier * rank;
    return (uint32)ceil(ret);
}

uint8 EvEMath::Skill::LevelForPoints(uint32 currentSP, uint8 rank)
{
    uint16 baseSLC = rank * EvESkill::skillPointMultiplier;
    if (baseSLC == 0)
        return 0;
    if (baseSLC > currentSP)
        return 0;
    int8 ret = log(currentSP / baseSLC) / EvESkill::DIVCONSTANT + 1;
    return (uint8)EvE::min(ret, EvESkill::MAXSKILLLEVEL);
}

uint8 EvEMath::Skill::PointsPerMinute(uint8 pAttr, uint8 sAttr)
{
    return (pAttr + (0.5f * sAttr));
}

int64 EvEMath::Skill::StartTime(uint32 currentSP, uint32 nextSP, uint8 SPMin, int64 timeNow)
{
    return (timeNow - (((nextSP - currentSP) / SPMin) * EvE::Time::Minute));
}

int64 EvEMath::Skill::EndTime(uint32 currentSP, uint32 nextSP, uint8 SPMin, int64 timeNow)
{
    if (currentSP >= nextSP)
        return 0;
    return ((((nextSP - currentSP) / SPMin) * EvE::Time::Minute) + timeNow);
}


//RAM Equations
int32 EvEMath::RAM::ProductionTime(uint32 BaseTime, float bpProductivityModifier, float ProductionLevel, float TimeModifier/*1*/ )
{
    float PE_Factor(0.0f);
    if (ProductionLevel >= 0.0f)
        PE_Factor = (ProductionLevel / (1.0f + ProductionLevel));
    else
        PE_Factor = (ProductionLevel - 1.0f);

    float effModifier(1.0f);
    effModifier -= ((bpProductivityModifier / BaseTime) * PE_Factor);
    return (BaseTime * effModifier * TimeModifier);
}

int32 EvEMath::RAM::ME_ResearchTime(uint32 BaseTime, uint8 MetallurgyLevel, float SlotModifier/*1*/, float ImplantModifier/*1*/ )
{
    return (BaseTime * (13.0f - (0.05f * MetallurgyLevel)) * SlotModifier * ImplantModifier);
}

int32 EvEMath::RAM::PE_ResearchTime(uint32 BaseTime, uint8 ResearchLevel, float SlotModifier/*1*/, float ImplantModifier/*1*/ )
{
    return (BaseTime * (15.0f - (0.05f * ResearchLevel)) * SlotModifier * ImplantModifier);
}

int32 EvEMath::RAM::RE_ResearchTime(uint32 BaseTime, uint8 ResearchLevel, float SlotModifier, float ImplantModifier)
{
    // base RE time is 60 min.  we may update/modify this later as system matures
    return 60;
}

int32 EvEMath::RAM::CopyTime(uint16 BaseTime, uint8 ScienceLevel, float SlotModifier/*1*/, float ImplantModifier/*1*/ )
{
    return (BaseTime * (1.0f - (0.05f * ScienceLevel)) * SlotModifier * ImplantModifier);
}

int32 EvEMath::RAM::InventionTime(uint32 BaseTime, uint8 AdvLabLevel, float SlotModifier/*1*/, float ImplantModifier/*1*/ )
{
     return BaseTime * (1.0f - (0.03f * AdvLabLevel)) * SlotModifier * ImplantModifier;
}

float EvEMath::RAM::ME_EffectOnWaste( float MaterialAmount, float BaseWasteFactor, float MaterialEfficiency )
{
    float ME_Factor(0.0f);
    if (MaterialEfficiency >= 0.0f)
        ME_Factor = 1.0f / (1.0f + MaterialEfficiency);
    else
        ME_Factor = 1.0f - MaterialEfficiency;

    return (floor(0.5f + (MaterialAmount * (BaseWasteFactor / 100.0f) * ME_Factor)));
}

uint32 EvEMath::RAM::PerfectME(uint32 MaterialAmount, uint8 BaseWasteFactor)
{
    return floor(0.02f * BaseWasteFactor * MaterialAmount);
}

float EvEMath::RAM::ResearchPointsPerDay( float Multiplier, float AgentEffectiveQuality, uint8 CharSkillLevel, uint8 AgentSkillLevel )
{
     return (Multiplier * (2.0f + (AgentEffectiveQuality / 100.0f)) * pow(CharSkillLevel + AgentSkillLevel,2));
}


float EvEMath::RAM::WasteSkillBased( uint32 MaterialAmount, float ProductionEfficiency )
{
	 return (floor(0.5f + (MaterialAmount * ((25.0f - (5.0f * ProductionEfficiency)) / 100.0f))));
}

float EvEMath::RAM::InventionChance( float BaseChance, uint8 EncryptionLevel, uint8 DataCore1SkillLevel, uint8 DataCore2SkillLevel, uint8 MetaLevel, float DecryptorModifier )
{
     return (BaseChance * (1 + 0.11f * EncryptionLevel) * (1.0f + (DataCore1SkillLevel + DataCore2SkillLevel)
     * (0.8f / (5.0f - MetaLevel)) * DecryptorModifier));
     // fuzzysteve's formula (23may13)
     // base * (1+0.01*EncryptionLevel) * (1+ (skill*(0.1/(5-metalevel)))) * max(decryptor,1)
     // skill = sum of sciences skill lvls (as required by bpc)

     // decryptor formula
     //  min(max(rounddown((inputT1BPCruns/T1MaxPerBPC)*(T2MaxPerBPC/10)),1)+decryptorRunBonus, T2MaxPerBPC)

     //  new formula (pre phoebe)
     // base * skill * decryptor
     //  skill = 1 + EncryptionLevel/40 + (DataCore1SkillLevel + DataCore2SkillLevel)/30
}

float EvEMath::Refine::StationTaxesForReprocessing(float CharacterStandingWithStationOwner )
{
    return 5.0f - (0.75f * CharacterStandingWithStationOwner);
}

float EvEMath::Refine::EffectiveRefiningYield(float EquipmentYield, uint8 RefiningLevel, uint8 RefiningEfficiencyLevel/*0*/, uint8 OreProcessingLevel/*0*/)
{
    return (EquipmentYield + 0.375f * (1.0f + (RefiningLevel * 0.02f))
            * (1.0f + (RefiningEfficiencyLevel * 0.04f)) * (1.0f + (OreProcessingLevel * 0.05f)));
}


// Agent Equations
float EvEMath::Agent::EffectiveQuality(int8 AgentQuality, uint8 NegotiationLevel, float AgentPersonalStanding)
{
    return (AgentQuality + (5.0f * NegotiationLevel) + AgentPersonalStanding);
}

float EvEMath::Agent::EffectiveStanding(float YourStanding, double standingBonus)
{
    return (1.0f - (1.0f - YourStanding / 10.0f) * (1.0f - standingBonus / 10.0f)) * 10.0f;
}

float EvEMath::Agent::RequiredStanding( uint8 AgentLevel, int8 AgentQuality )
{
    return (((AgentLevel - 1.0f) * 2.0f) + (AgentQuality/20.0f));
}

float EvEMath::Agent::MissionStandingIncrease( float BaseMissionIncrease, uint8 YourSocialSkillLevel )
{
    return (BaseMissionIncrease * (1.0f + 0.05f * YourSocialSkillLevel));
}

float EvEMath::Agent::Efficiency( uint8 AgentLevel, int8 AgentQuality )
{
    return (0.01f * ((8.0f * AgentLevel) + (0.1f * AgentQuality) - 4.0f));
}

float EvEMath::Agent::AgentStandingIncrease(float CurrentStanding, float PercentIncrease)
{
    return (((10.0f - CurrentStanding) * PercentIncrease) + CurrentStanding);
}

float EvEMath::Agent::GetStandingBonus(float fromStanding, uint32 fromFactionID, uint8 ConnectionsSkillLevel, uint8 DiplomacySkillLevel, uint8 CriminalConnectionsSkillLevel)
{
    float bonus(0.0f);
    if (fromStanding < 0.0f) {
        bonus = DiplomacySkillLevel * 0.4f;
    } else if (fromStanding > 0.0f) {
        switch (fromFactionID) {
            case 500010:
            case 500011:
            case 500012:
            case 500019:
            case 500020: {
                bonus = CriminalConnectionsSkillLevel * 0.4f;
            } break;
            default: {
                bonus = ConnectionsSkillLevel * 0.4f;
            } break;
        }
    }
    return bonus;
}

float EvEMath::Market::BrokerFee(uint8 brSkillLvl, float fStanding, float cStanding, float orderValue)
{
    float wStanding = (0.7f * fStanding + 0.3f * cStanding) / 10.0f;
    float fee = 0.01f * (1.0f - (0.05f * brSkillLvl)) * pow(2, -2 * wStanding);
    return EvE::max(fee * orderValue, 100.0f);
}

float EvEMath::Market::RelistFee(float oldPrice, float newPrice, float brokerPercent/*0.01*/, float discount/*0*/)
{
    // this needs a 'Relist Discount' but no clue where to find data for it yet
    return EvE::max(brokerPercent * (newPrice -oldPrice)) + (1 -discount) *brokerPercent *newPrice;
}

float EvEMath::Market::SalesTax(float baseSalesTax, uint8 accountingLvl/*0*/, uint8 taxEvasionLvl/*0*/)
{
    /** @todo  add skillTaxEvasion to this formula; its not calculated in client... */
    float maximumTax = baseSalesTax / 100.0f;
    float tax = maximumTax * (1 - 0.1f * accountingLvl);
    return EvE::min(tax, maximumTax);
}

void EvEMath::PI::Dijkstra(uint32 sourcePin, uint32 destinationPin)
{
    // not used yet...
}


/*
 *
 * Research Points Per Day
 * Research_Points_Per_Day = Multiplier * ((1 + (Agent_Effective_Quality / 100)) * ((Your_Skill + Agent_Skill) ^ 2))
 * Multiplier is a specific multiplier for the research field you want to do research in. Like 3x for starship engineering
 * Your_Skill is your skill level in the research field
 * Agent_Skill is the agent's skill level in the research field
 *
 *
 * Station take when refining/reprocessing
 * Station_Take = Max((5 - (0.75 * Your_Standing)), 0)
 * For the station to take 0% you need a standing to the station owner of at least: 5 / 0.75 = 6.67
 *
 *
 * Agent Effective Quality - Removed in Incursion 1.5 (2011-05-19)
 * Agent_Effective_Quality = Agent_Quality + (5 * Negotiation_Skill_Level) + Round_Down(Effective_Standing)
 * Effective_Standing is the highest effective of either personal, corp. or faction standing.
 *
 *
 * Blueprint Material Requirement - Before Crius 1.0 (2014-07-22)
 * Required_Amount = Round(Base_Amount * ((1 + (Default_Blueprint_Waste_Factor / (1 + Blueprint_Material_Level))) + (0.25 - (0.05 * Production_Efficiency_Skill_Level))), 0)
 *
 *
 * Invention Chance - Before Phoebe 1.0 (2014-11-04)
 * Invention_Chance = Base_Chance * (1 + (0.01 * Encryption_Skill_Level)) * (1 + ((Datacore_1_Skill_Level + Datacore_2_Skill_Level) * (0.1 / (5 - Meta_Level)))) * Decryptor_Modifier
 * Meta_Level of the base items used. No base items is the same as metalevel 0 = useless.
 * Decryptor_Modifier is optional :-)
 *
 *
 * Reverse Engineering Chance - Merged with Invention in Phoebe 1.0 (2014-11-04)
 * Reverse_Chance = Base_Chance * (1 + (0.01 * Reverse_Engineering_Skill_Level)) * (1 + (0.1 * (Datacore_1_Skill_Level + Datacore_2_Skill_Level)))
 */
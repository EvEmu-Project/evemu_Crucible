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
    Author:        Luck
*/

#ifndef MODULE_EFFECTS_H
#define MODULE_EFFECTS_H

#include "ship/modules/ModuleDB.h"
#include "ship/modules/ModuleDefs.h"
#include "utils/Singleton.h"


// ////////////////////// Effects Classs ////////////////////////////
typedef std::vector<uint32> typeTargetGroupIDlist;

class MEffect
{
public:
    MEffect(uint32 effectID);
    ~MEffect();

    //accessors for selected effect
    uint32 GetEffectID()                                        { return (m_EffectID == 0) ? 0 : m_EffectID; }
    std::string GetEffectName()                                    { return (m_EffectID == 0) ? std::string("") : m_EffectName; }
    uint32 GetEffectCategory()                                    { return (m_EffectID == 0) ? 0 : m_EffectCategory; }
    uint32 GetPreExpression()                                    { return (m_EffectID == 0) ? 0 : m_PreExpression; }
    uint32 GetPostExpression()                                    { return (m_EffectID == 0) ? 0 : m_PostExpression; }
    std::string GetDescription()                                { return (m_EffectID == 0) ? std::string("") : m_Description; }
    std::string GetGuid()                                        { return (m_EffectID == 0) ? std::string("") : m_Guid; }
    uint32 GetIconID()                                            { return (m_EffectID == 0) ? 0 : m_IconID; }
    bool GetIsOffensive()                                        { return (m_EffectID == 0) ? false : m_IsOffensive == 1; }
    bool GetIsAssistance()                                        { return (m_EffectID == 0) ? false : m_IsAssistance == 1; }
    uint32 GetDurationAttributeID()                                { return (m_EffectID == 0) ? 0 : m_DurationAttributeID; }
    uint32 GetTrackingSpeedAttributeID()                        { return (m_EffectID == 0) ? 0 : m_TrackingSpeedAttributeID; }
    uint32 GetDischargeAttributeID()                            { return (m_EffectID == 0) ? 0 : m_DischargeAttributeID; }
    uint32 GetRangeAttributeID()                                { return (m_EffectID == 0) ? 0 : m_RangeAttributeID; }
    uint32 GetFalloffAttributeID()                                { return (m_EffectID == 0) ? 0 : m_FalloffAttributeID; }
    bool GetDisallowAutoRepeat()                                { return (m_EffectID == 0) ? false : m_DisallowAutoRepeat == 1; }
    bool GetIsPublished()                                        { return (m_EffectID == 0) ? false : m_Published == 1; }
    std::string GetDisplayName()                                { return (m_EffectID == 0) ? std::string("") : m_DisplayName; }
    bool GetIsWarpSafe()                                        { return (m_EffectID == 0) ? false : m_IsWarpSafe == 1; }
    bool GetRangeChance()                                        { return (m_EffectID == 0) ? false : m_RangeChance == 1; }
    bool GetPropulsionChance()                                    { return (m_EffectID == 0) ? false : m_PropulsionChance == 1; }
    bool GetElectronicChance()                                    { return (m_EffectID == 0) ? false : m_ElectronicChance == 1; }
    uint32 GetDistribution()                                    { return (m_EffectID == 0) ? 0 : m_Distribution; }
    std::string GetSfxName()                                    { return (m_EffectID == 0) ? std::string("") : m_DisplayName; }
    uint32 GetNpcUsageChanceAttributeID()                        { return (m_EffectID == 0) ? 0 : m_NpcUsageChanceAttributeID; }
    uint32 GetNpcActivationChanceAttributeID()                    { return (m_EffectID == 0) ? 0 : m_NpcActivationChanceAttributeID; }
    uint32 GetFittingUsageChanceAttributeID()                    { return (m_EffectID == 0) ? 0 : m_FittingUsageChanceAttributeID; }

    //accessors for the effects targetAttributeID, sourceAttributeID and calculation type:
    uint32 GetSizeOfAttributeList()                                { return (m_EffectID == 0) ? 0 : m_numOfIDs; }
    uint32 GetTargetAttributeID(uint32 index)                    { return (m_EffectID == 0) ? 0 : m_TargetAttributeIDs[index]; }
    uint32 GetSourceAttributeID(uint32 index)                    { return (m_EffectID == 0) ? 0 : m_SourceAttributeIDs[index]; }
    EVECalculationType GetCalculationType(uint32 index)            { return (m_EffectID == 0) ? (EVECalculationType)0 : (EVECalculationType)m_CalculationTypeIDs[index];}
    EVECalculationType GetReverseCalculationType(uint32 index)    { return (m_EffectID == 0) ? (EVECalculationType)0 : (EVECalculationType)m_ReverseCalculationTypeIDs[index];}

    uint32 GetModuleStateWhenEffectApplied()                    { return (m_EffectID == 0) ? 0 : m_EffectAppliedWhenID; }
    uint32 GetTargetTypeToWhichEffectApplied(uint32 index)        { return (m_EffectID == 0) ? 0 : m_EffectAppliedToTargetIDs[index]; }
    uint32 GetEffectApplicationType(uint32 index)               { return (m_EffectID == 0) ? 0 : m_EffectApplicationTypeIDs[index]; }
    uint32 GetTargetEquipmentType(uint32 index)                 { return (m_EffectID == 0) ? 0 : m_TargetEquipmentTypeIDs[index]; }
    typeTargetGroupIDlist * GetTargetGroupIDlist(uint32 index)    { return (m_EffectID == 0) ? 0 : m_TargetGroupIDlists.find(index)->second; }
    uint32 GetStackingPenaltyApplied(uint32 index)              { return (m_EffectID == 0) ? 0 : m_StackingPenaltyAppliedIDs[index]; }
    uint32 GetNullifyOnlineEffectEnable()                       { return (m_EffectID == 0) ? 0 : m_NullifyOnlineEffectEnable; }
    uint32 GetNullifiedOnlineEffectID()                         { return (m_EffectID == 0) ? 0 : m_NullifiedOnlineEffectID; }

	bool IsEffectLoaded() { return m_EffectLoaded; }

private:
    void _Populate(uint32 effectID);

    int m_EffectID;
    std::string m_EffectName;
    int m_EffectCategory;
    int m_PreExpression;
    int m_PostExpression;
    std::string m_Description;
    std::string m_Guid;
    int m_IconID;
    int m_IsOffensive;
    int m_IsAssistance;
    int m_DurationAttributeID;
    int m_TrackingSpeedAttributeID;
    int m_DischargeAttributeID;
    int m_RangeAttributeID;
    int m_FalloffAttributeID;
    int m_DisallowAutoRepeat;
    int m_Published;
    std::string m_DisplayName;
    int m_IsWarpSafe;
    int m_RangeChance;
    int m_ElectronicChance;
    int m_PropulsionChance;
    int m_Distribution;
    std::string m_SfxName;
    int m_NpcUsageChanceAttributeID;
    int m_NpcActivationChanceAttributeID;
    int m_FittingUsageChanceAttributeID;

    int m_numOfIDs;
    int * m_TargetAttributeIDs;
    int * m_SourceAttributeIDs;
    int * m_CalculationTypeIDs;
    int * m_ReverseCalculationTypeIDs;
    int * m_EffectAppliedToTargetIDs;
    int * m_EffectAppliedBehaviorIDs;
    int * m_EffectApplicationTypeIDs;
    int * m_TargetEquipmentTypeIDs;
    std::map<uint32, typeTargetGroupIDlist *> m_TargetGroupIDlists;
    int * m_StackingPenaltyAppliedIDs;
    int m_EffectAppliedWhenID;
    int m_NullifyOnlineEffectEnable;
    int m_NullifiedOnlineEffectID;

	bool m_EffectLoaded;
};


// This class is a singleton object, containing all Effects loaded from dgmEffects table as memory objects of type MEffect:
class DGM_Effects_Table
: public Singleton< DGM_Effects_Table >
{
public:
    DGM_Effects_Table();
    ~DGM_Effects_Table();

    // Initializes the Table:
    int Initialize();

    // Returns pointer to MEffect object corresponding to the effectID supplied:
    MEffect * GetEffect(uint32 effectID);

protected:
    void _Populate();

    std::map<uint32, MEffect *> m_EffectsMap;
};

#define sDGM_Effects_Table \
    ( DGM_Effects_Table::get() )
//////////////////////////////////////////////////////////////////////////


// ////////////////////// ModuleEffects Class ////////////////////////////

//class contained by all modules that is populated on construction of the module
//this will contain all information about the effects of the module
class ModuleEffects
{
public:
    ModuleEffects(uint32 typeID);
    ~ModuleEffects();

    //this will need to be reworked to implement a singleton architecture...i'll do it later -luck

    //useful accessors - probably a better way to do this, but at least it's fast
    bool isHighSlot();
    bool isMediumSlot();
    bool isLowSlot();
    bool HasEffect(uint32 effectID);
    bool HasDefaultEffect() { return ( (m_defaultEffect != NULL) ? true : false ); }
    MEffect * GetDefaultEffect() { return m_defaultEffect; }
    MEffect * GetEffect(uint32 effectID);

    std::map<uint32, MEffect *>::const_iterator GetPersistentEffectsConstIterator() { return m_PersistentEffects.begin(); }
    std::map<uint32, MEffect *>::const_iterator GetOnlineEffectsConstIterator() { return m_OnlineEffects.begin(); }
    std::map<uint32, MEffect *>::const_iterator GetActiveEffectsConstIterator() { return m_ActiveEffects.begin(); }
    std::map<uint32, MEffect *>::const_iterator GetOverloadEffectsConstIterator() { return m_OverloadEffects.begin(); }

private:

    void _populate(uint32 typeID);

    //data members
    std::map<uint32, MEffect *> m_PersistentEffects;
    std::map<uint32, MEffect *> m_OnlineEffects;
    std::map<uint32, MEffect *> m_ActiveEffects;
    std::map<uint32, MEffect *> m_OverloadEffects;
    MEffect * m_defaultEffect;

    uint32 m_typeID;

    //cached stuff
    bool m_HighPower, m_MediumPower, m_LowPower, m_Cached;

};

#endif /* MODULE_EFFECTS_H */
//////////////////////////////////////////////////////////////////////////

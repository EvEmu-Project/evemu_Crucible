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
Author: Bloody.Rabbit
Rewrite:    Allan
*/

#include "eve-server.h"

#include "character/Character.h"
#include "character/Skill.h"
#include "inventory/AttributeEnum.h"

/*
 * SKILL__ERROR
 * SKILL__WARNING
 * SKILL__MESSAGE
 * SKILL__INFO
 * SKILL__DEBUG
 * SKILL__TRACE
 */

Skill::Skill(uint32 _skillID, const ItemType& _type, const ItemData& _data)
: InventoryItem(_skillID, _type, _data)
{
}

SkillRef Skill::Load( uint32 skillID)
{
    return InventoryItem::Load<Skill>(skillID );
}

SkillRef Skill::Spawn( ItemData &data)
{
    uint32 skillID(InventoryItem::CreateItemID(data));
    if ( skillID == 0 )
        return SkillRef(nullptr);

    SkillRef skillRef(Skill::Load(skillID));
    if (skillRef.get() == nullptr) {
        // make error msg here for failure to load skill?
        return SkillRef(nullptr);
    }

    skillRef->SaveItem();
    return skillRef;
}

bool Skill::IsTraining() {
    return (flag() == flagSkillInTraining);
}

uint8 Skill::GetLevelForSP(uint32 currentSP) {
    return EvEMath::Skill::LevelForPoints(currentSP, GetAttribute(AttrSkillTimeConstant).get_uint32());
}

uint32 Skill::GetSPForLevel(uint8 level) {
    return EvEMath::Skill::PointsAtLevel(level, GetAttribute(AttrSkillTimeConstant).get_float());
}

uint32 Skill::GetCurrentSP(Character* ch, int64 startTime/*0*/)
{
    uint32 currentSP(GetAttribute(AttrSkillPoints).get_uint32());
    if (flag() == flagSkill)
        return currentSP;

    if (startTime == 0)
        return currentSP;

    if (startTime > GetFileTimeNow())
        return currentSP;

    uint8 level = GetAttribute(AttrSkillLevel).get_uint32() + 1;
    if (level > EvESkill::MAXSKILLLEVEL)
        level = EvESkill::MAXSKILLLEVEL;

    /** @todo this isnt completely right.... */
    // at this point, the skill is in training.  calculate accumulated sp and return
    uint32 delta(0);
    uint32 timeElapsed((GetFileTimeNow() - startTime) / EvE::Time::Second);
    if (timeElapsed > 60) {
        // skill in training - return updated SP based on elapsed training
        delta = (timeElapsed / 60) * ch->GetSPPerMin(this);
        currentSP += delta;
    }

    _log(SKILL__TRACE, "Skill::GetCurrentSP() for %s is %u - delta: %u, elapsed time: %us", \
            name(), currentSP, delta, timeElapsed);

    return currentSP;
}

uint32 Skill::GetRemainingSP(Character* ch, int64 curTime/*0*/)
{
    uint8 level = GetAttribute(AttrSkillLevel).get_uint32() + 1;
    if (level > EvESkill::MAXSKILLLEVEL)
        return 0;

    if (curTime == 0)
        curTime = GetFileTimeNow();

    // get full sp needed for next level
    uint32 remainingSP(GetSPForLevel(level) - GetAttribute(AttrSkillPoints).get_uint32());

    float timeLeft((ch->GetEndOfTraining() - curTime) / EvE::Time::Second);
    // if remaining time > 1m, subtract spm from total to get remaining
    if (timeLeft > 60)
        remainingSP -= ((timeLeft / 60) * ch->GetSPPerMin(this));

    return remainingSP;
}

uint32 Skill::GetTrainingTime(Character* ch, int64 startTime/*0*/)
{
    uint8 level = GetAttribute(AttrSkillLevel).get_uint32() + 1;
    if (level > EvESkill::MAXSKILLLEVEL)
        return 0;

    // get full sp needed for next level
    uint32 remainingSP(GetSPForLevel(level) - GetAttribute(AttrSkillPoints).get_uint32());
    // divide by spm to get time and convert to seconds
    uint32 timeLeft((remainingSP / ch->GetSPPerMin(this)) * 60);

    if (startTime == 0)
        return timeLeft;

    uint32 delta = (uint32)ceil((GetFileTimeNow() -  startTime) / EvE::Time::Second);
    if (delta < 1)
        return timeLeft;

    // this skill is currently training.  subtract accumulated time from total and return
    return timeLeft - delta;
}

void Skill::VerifyAttribs()
{
    if (!isSingleton())
        ChangeSingleton(true);
    if (GetAttribute(AttrSkillLevel).get_type() != evil_number_int) {
        _log(SKILL__INFO, "Skill %s level type != int.  Fixing...", name());
        SetAttribute(AttrSkillLevel, GetAttribute(AttrSkillLevel).get_uint32(), false);
    }
    if (GetAttribute(AttrSkillPoints).get_type() != evil_number_int) {
        _log(SKILL__INFO, "Skill %s sp type != int.  Fixing...", name());
        SetAttribute(AttrSkillPoints, GetAttribute(AttrSkillPoints).get_uint32(), false);
    }
}

void Skill::VerifySP()
{
    if (is_log_enabled(SKILL__MESSAGE))
        _log(SKILL__MESSAGE, "Begin SP check for %s. level %u: CurrentSP: %u", \
                name(), GetAttribute(AttrSkillLevel).get_uint32(), GetAttribute(AttrSkillPoints).get_uint32());

    if (GetAttribute(AttrSkillPoints) == EvilZero)
        return;

    uint8 level(GetAttribute(AttrSkillLevel).get_uint32() + 1);
    if (level > EvESkill::MAXSKILLLEVEL) {
        level = EvESkill::MAXSKILLLEVEL;
        SetAttribute(AttrSkillLevel, level, false);
    }
    uint32 spThisLevel(GetSPForLevel(level - 1));
    uint32 spCurrent(GetAttribute(AttrSkillPoints).get_uint32());
    if (spCurrent < spThisLevel) {
        _log(SKILL__WARNING, "Skill %s points low.  Updating from %u to %u", name(), spCurrent, spThisLevel);
        SetAttribute(AttrSkillPoints, spThisLevel, false);
        // hit it again to be sure it's fixed
        VerifySP();
        return;
    }
    uint32 spNextLevel(GetSPForLevel(level));
    if (spCurrent > spNextLevel) {
        SetAttribute(AttrSkillLevel, level);
        if (level > 4) {
            _log(SKILL__WARNING, " %s - Skillpoints high for L5. Updating SP from %u to %u.", \
                name(), spCurrent, spNextLevel);
        } else {
            _log(SKILL__WARNING, " %s - Skillpoints high. Updating level from %u to %u and SP from %u to %u.", \
                name(), level - 1, level, spCurrent, spNextLevel);
        }
        SetAttribute(AttrSkillPoints, spNextLevel, false);
        // hit it again to be sure it's fixed
        VerifySP();
    }
}

bool Skill::SkillPrereqsComplete(Character &ch) {
    bool test(true);
    EvilNumber skillID(0);
    if (HasAttribute(AttrRequiredSkill1, skillID)) {
        if (GetAttribute(AttrRequiredSkill1Level) > ch.GetSkillLevel(skillID.get_uint32()))
            test = false;
        if (HasAttribute(AttrRequiredSkill2, skillID)) {
            if (GetAttribute(AttrRequiredSkill2Level) > ch.GetSkillLevel(skillID.get_uint32()))
                test = false;
            if (HasAttribute(AttrRequiredSkill3, skillID)) {
                if (GetAttribute(AttrRequiredSkill3Level) > ch.GetSkillLevel(skillID.get_uint32()))
                    test = false;
                if (HasAttribute(AttrRequiredSkill4, skillID)) {
                    if (GetAttribute(AttrRequiredSkill4Level) > ch.GetSkillLevel(skillID.get_uint32()))
                        test = false;
                }
            }
        }
    }

    return test;
}

bool Skill::FitModuleSkillCheck(InventoryItemRef iRef, CharacterRef cRef) {
    bool test(true);
    EvilNumber skillID(0);
    if (iRef->HasAttribute(AttrRequiredSkill1, skillID)) {//Primary Skill
        if ( iRef->GetAttribute(AttrRequiredSkill1Level) > cRef->GetSkillLevel(skillID.get_uint32()))
            test = false;
        if (iRef->HasAttribute(AttrRequiredSkill2, skillID)) {//Secondary Skill
            if ( iRef->GetAttribute(AttrRequiredSkill2Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                test = false;
            if (iRef->HasAttribute(AttrRequiredSkill3, skillID)) {//Tertiary Skill
                if ( iRef->GetAttribute(AttrRequiredSkill3Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                    test = false;
                if (iRef->HasAttribute(AttrRequiredSkill4, skillID)) {//Quarternary Skill
                    if ( iRef->GetAttribute(AttrRequiredSkill4Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                        test = false;
                    if (iRef->HasAttribute(AttrRequiredSkill5, skillID)) {//Quinary Skill
                        if ( iRef->GetAttribute(AttrRequiredSkill5Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                            test = false;
                        if (iRef->HasAttribute(AttrRequiredSkill6, skillID)) {//Senary Skill
                            if ( iRef->GetAttribute(AttrRequiredSkill6Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                                test = false;
                        }
                    }
                }
            }
        }
    }

    return test;
}
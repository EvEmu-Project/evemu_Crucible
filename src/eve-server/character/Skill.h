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
    Author:        Bloody.Rabbit
    Updates:    Allan
*/

#ifndef EVE_SERVER_SKILLS_SKILL_H
#define EVE_SERVER_SKILLS_SKILL_H


#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "../../eve-core/utils/misc.h"
#include "inventory/InventoryItem.h"

class Character;

/**
 * \class Skill
 *
 * @brief InventoryItem for skill.
 *
 * Skill class where all skill stuff is placed.
 *
 * @author Bloody.Rabbit
 * @date June 2009
 */
class Skill
: public InventoryItem
{
    friend class InventoryItem;
public:
    ~Skill() { /* do nothing here */ }

    /**
     * Loads skill.
     *
     * @param[in] factory
     * @param[in] skillID ID of skill to load.
     * @return Pointer to new Skill object; NULL if fails.
     */
    static SkillRef Load( uint32 skillID);
    /**
     * Spawns new skill.
     *
     * @param[in] factory
     * @param[in] data Item data of new skill.
     * @return Pointer to new Skill object; NULL if fails.
     */
    static SkillRef Spawn( ItemData &data);

    bool IsTraining();

    uint8 GetLevelForSP(uint32 currentSP);
    uint32 GetSPForLevel(uint8 level);
    uint32 GetCurrentSP(Character* ch, int64 startTime=0);      // requires startTime for skill in training
    // returns remaining sp to next level of this skill
    uint32 GetRemainingSP(Character* ch, int64 curTime=0);      // requires Skill* = m_skillQueue.front()
    // returns remaining time in seconds to train this skill to next level
    uint32 GetTrainingTime(Character* ch, int64 startTime=0);
    bool SkillPrereqsComplete(Character &ch);
    static bool FitModuleSkillCheck(InventoryItemRef item, CharacterRef ch);

    void VerifySP();
    void VerifyAttribs();


protected:
    Skill(uint32 _skillID, const ItemType &_type, const ItemData &_data );

    /*
     * Member functions
     */
    using InventoryItem::_Load;

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 skillID, const ItemType &type, const ItemData &data) {
        if (type.categoryID() != EVEDB::invCategories::Skill) {
            _log(ITEM__ERROR, "Trying to load %s as Skill.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.debug.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }

        return SkillRef( new Skill(skillID, type, data ) );
    }
};

#endif  // EVE_SERVER_SKILLS_SKILL_H

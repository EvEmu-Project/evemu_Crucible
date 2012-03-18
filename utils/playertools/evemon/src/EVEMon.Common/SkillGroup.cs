using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.IO;
using System.Reflection;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a skills group
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class SkillGroup : ReadonlyKeyedCollection<string, Skill>
    {
        private Character m_character;
        private StaticSkillGroup m_group;

        /// <summary>
        /// Constructor, only used by SkillCollection
        /// </summary>
        /// <param name="character"></param>
        /// <param name="group"></param>
        internal SkillGroup(Character character, StaticSkillGroup src)
        {
            m_group = src;
            m_character = character;

            foreach (var srcSkill in src)
            {
                m_items[srcSkill.Name] = new Skill(character, this, srcSkill);
            }
        }

        /// <summary>
        /// Gets the static data associated with this group
        /// </summary>
        public StaticSkillGroup StaticData
        {
            get { return m_group; }
        }

        /// <summary>
        /// Gets the group's ID
        /// </summary>
        public long ID
        {
            get { return m_group.ID; }
        }

        /// <summary>
        /// Gets the group's name
        /// </summary>
        public string Name
        {
            get { return m_group.Name; }
        }

        /// <summary>
        /// Gets the skill with the provided name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public Skill this[string name]
        {
            get { return GetByKey(name); }
        }

        /// <summary>
        /// Gets a skill by its name
        /// </summary>
        /// <param name="skillName"></param>
        /// <returns></returns>
        public bool Contains(string skillName)
        {
            return m_items.ContainsKey(skillName);
        }

        /// <summary>
        /// Gets the total number of SP in this group
        /// </summary>
        public int TotalSP
        {
            get
            {
                int result = 0;
                foreach (Skill gs in m_items.Values)
                {
                    result += gs.SkillPoints;
                }
                return result;
            }
        }
    }
}

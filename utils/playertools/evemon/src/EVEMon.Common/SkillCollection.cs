using System.Collections.Generic;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a collection of skills.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class SkillCollection : ReadonlyKeyedCollection<string, Skill>
    {
        private readonly Character m_character;
        private readonly Skill[] m_itemsArray = new Skill[StaticSkills.ArrayIndicesCount];
        private readonly Dictionary<long, Skill> m_itemsByID = new Dictionary<long, Skill>();

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="character"></param>
        internal SkillCollection(Character character)
        {
            m_character = character;
            foreach (var group in character.SkillGroups)
            {
                foreach (var skill in group)
                {
                    m_items[skill.Name] = skill;
                    m_itemsByID[skill.ID] = skill;
                    m_itemsArray[skill.ArrayIndex] = skill;
                }
            }

            // Build prerequisites list
            foreach (var skill in m_itemsArray)
            {
                skill.CompleteInitialization(m_itemsArray);
            }
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
        /// Gets the skill with the provided id
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public Skill this[long id]
        {
            get 
            {
                Skill skill = null;
                m_itemsByID.TryGetValue(id, out skill);
                return skill;
            }
        }

        /// <summary>
        /// Gets the skill representing the given static skill
        /// </summary>
        /// <param name="skill"></param>
        /// <returns></returns>
        public Skill this[StaticSkill skill]
        {
            get { return GetByArrayIndex(skill.ArrayIndex); }
        }

        /// <summary>
        /// Gets the skill with the provided array index (see <see cref="StaticSkill.ArrayIndex"/>)
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public Skill GetByArrayIndex(int index)
        {
            return m_itemsArray[index];
        }
    }
}

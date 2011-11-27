using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a collection of a character's skills groups
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class SkillGroupCollection : ReadonlyKeyedCollection<string, SkillGroup>
    {
        private readonly Character m_character;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="character"></param>
        internal SkillGroupCollection(Character character)
        {
            m_character = character;
            foreach (var srcGroup in StaticSkills.AllGroups)
            {
                var group = new SkillGroup(character, srcGroup);
                m_items[group.Name] = group;
            }
        }

        /// <summary>
        /// Gets the skill group with the provided name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public SkillGroup this[string name]
        {
            get { return GetByKey(name); }
        }

    }
}

using System.Collections.Generic;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a readonly skill group definition. Characters have their own implementations, <see cref="Skillgroup"/>
    /// </summary>
    public sealed class StaticSkillGroup : ReadonlyKeyedCollection<int, StaticSkill>
    {
        private readonly long m_ID;
        private readonly string m_name;

        #region Constructors

        /// <summary>
        /// Deserialization constructor from datafiles.
        /// </summary>
        /// <param name="src"></param>
        /// <param name="skillArrayIndex"></param>
        internal StaticSkillGroup(SerializableSkillGroup src, ref int skillArrayIndex)
        {
            m_ID = src.ID;
            m_name = src.Name;
            foreach (var srcSkill in src.Skills)
            {
                m_items[srcSkill.ID] = new StaticSkill(this, srcSkill, skillArrayIndex);
                skillArrayIndex++;
            }
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets the group's identifier.
        /// </summary>
        public long ID
        {
            get { return m_ID; }
        }

        /// <summary>
        /// Gets the group's name.
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets a skill from this group by its id
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public StaticSkill this[int id]
        {
            get { return GetByKey(id); }
        }

        /// <summary>
        /// Checks whether this group contains the specified skill
        /// </summary>
        /// <param name="skillName"></param>
        /// <returns></returns>
        public bool Contains(int skillID)
        {
            return m_items.ContainsKey(skillID);
        }

        #endregion


        #region IEnumerable<StaticSkill> Members

        public IEnumerator<StaticSkill> GetEnumerator()
        {
            return m_items.Values.GetEnumerator();
        }

        #endregion

    }
}

using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a lisht and fast dictionary for skilllevels, focused on 
    /// </summary>
    internal sealed class SkillLevelSet<T> : IReadonlyCollection<T>
        where T : ISkillLevel
    {
        private readonly T[] m_items;
        private int m_count;

        /// <summary>
        /// Constructor
        /// </summary>
        public SkillLevelSet()
        {
            m_items = new T[StaticSkills.ArrayIndicesCount * 5];
        }

        /// <summary>
        /// Gets the number of items in the set
        /// </summary>
        public int Count
        {
            get { return m_count; }
        }

        /// <summary>
        /// Adds the given item in the list
        /// </summary>
        /// <param name="item"></param>
        public void Set(T item)
        {
            this[item.Skill.ArrayIndex, item.Level] = item;
        }

        #region Indexors
        /// <summary>
        /// Gets or sets the item for the given skill array index and level
        /// </summary>
        /// <param name="skillArrayIndex"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public T this[int skillArrayIndex, int level]
        {
            get
            {
                Enforce.Argument(level > 0 && level <= 5, "level", "Level mut be greater than 0 and lesser or equal than 5.");
                return m_items[skillArrayIndex * 5 + level - 1];
            }
            set
            {
                Enforce.Argument(level > 0 && level <= 5, "level", "Level mut be greater than 0 and lesser or equal than 5.");
                var oldValue = m_items[skillArrayIndex * 5 + level - 1];

                if (value.Skill == null) m_count--;
                else if (oldValue == null || oldValue.Skill == null) m_count++;
                m_items[skillArrayIndex * 5 + level - 1] = value;
            }
        }

        /// <summary>
        /// Gets or sets the item for the given skill and level
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public T this[StaticSkill skill, int level]
        {
            get { return this[skill.ArrayIndex, level]; }
            set { this[skill.ArrayIndex, level] = value; }
        }
        #endregion


        #region Contains overloads
        /// <summary>
        /// Gets true if a matching item is already contained.
        /// </summary>
        /// <param name="skillArrayindex"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public bool Contains(int skillArrayindex, int level)
        {
            var result = this[skillArrayindex, level];
            return result != null && result.Skill != null;
        }

        /// <summary>
        /// Gets true if a matching item is already contained.
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public bool Contains(StaticSkill skill, int level)
        {
            var result = this[skill.ArrayIndex, level];
            return result != null && result.Skill != null;
        }

        /// <summary>
        /// Gets true if a matching item is already contained.
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public bool Contains(Skill skill, int level)
        {
            var result = this[skill.ArrayIndex, level];
            return result != null && result.Skill != null;
        }

        /// <summary>
        /// Gets true if a matching item is already contained.
        /// </summary>
        /// <remarks>The comparison is only based on skill array index and level, the two objects may be actually different references</remarks>
        /// <param name="item"></param>
        /// <returns></returns>
        public bool Contains(ISkillLevel item)
        {
            var result = this[item.Skill.ArrayIndex, item.Level];
            return result != null && result.Skill != null;
        }
        #endregion


        #region Remove overloads
        /// <summary>
        /// Gets true if a matching item is already contained.
        /// </summary>
        /// <param name="skillArrayindex"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public void Remove(int skillArrayindex, int level)
        {
            this[skillArrayindex, level] = default(T);
        }

        /// <summary>
        /// Gets true if a matching item is already contained.
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="level"></param>
        /// <returns></returns>
        public void Remove(StaticSkill skill, int level)
        {
            this[skill.ArrayIndex, level] = default(T);
        }

        /// <summary>
        /// Remove the matching item
        /// </summary>
        /// <remarks>The comparison is only based on skill array index and level, the two objects may be actually different references</remarks>
        /// <param name="item"></param>
        /// <returns></returns>
        public void Remove(ISkillLevel item)
        {
            this[item.Skill.ArrayIndex, item.Level] = default(T);
        }
        #endregion


        #region GetLevelsOf() overloads
        /// <summary>
        /// Gets the levels of the given skill.
        /// </summary>
        /// <param name="skill"></param>
        /// <returns></returns>
        public IEnumerable<T> GetLevelsOf(Skill skill)
        {
            return GetLevelsOf(skill.ArrayIndex);
        }

        /// <summary>
        /// Gets the levels of the given skill.
        /// </summary>
        /// <param name="skill"></param>
        /// <returns></returns>
        public IEnumerable<T> GetLevelsOf(StaticSkill skill)
        {
            return GetLevelsOf(skill.ArrayIndex);
        }

        /// <summary>
        /// Gets the levels of the skill represented by the given index.
        /// </summary>
        /// <param name="skillArrayIndex"></param>
        /// <returns></returns>
        public IEnumerable<T> GetLevelsOf(int skillArrayIndex)
        {
            for (int i = 0; i < 5; i++)
            {
                T item = m_items[skillArrayIndex * 5 + i];
                if (item != null && item.Skill != null) yield return item;
            }
        }
        #endregion


        #region IEnumerable<T> Members
        private IEnumerable<T> Enumerate()
        {
            foreach (var item in m_items)
            {
                if (item != null) yield return item;
            }
        }

        public IEnumerator<T> GetEnumerator()
        {
            return Enumerate().GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return ((System.Collections.IEnumerable)Enumerate()).GetEnumerator();
        }
        #endregion
    }
}

using System.Collections.Generic;

using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents the list of all static skills
    /// </summary>
    public static class StaticSkills
    {
        private static int m_arrayIndicesCount;
        private static StaticSkill[] m_skills;
        private static readonly Dictionary<long, StaticSkill> m_skillsById = new Dictionary<long, StaticSkill>();
        private static readonly Dictionary<string, StaticSkill> m_skillsByName = new Dictionary<string, StaticSkill>();
        private static readonly Dictionary<long, StaticSkillGroup> m_allGroupsById = new Dictionary<long, StaticSkillGroup>();

        #region Public Properties

        /// <summary>
        /// Gets the total number of zero-based indices given to skills (for optimization purposes, it allows the use of arrays for computations)
        /// </summary>
        public static int ArrayIndicesCount
        {
            get { return m_arrayIndicesCount; }
        }

        /// <summary>
        /// Gets the list of groups
        /// </summary>
        public static IEnumerable<StaticSkillGroup> AllGroups
        {
            get
            {
                foreach (var group in m_allGroupsById.Values)
                {
                    yield return group;
                }
            }
        }

        /// <summary>
        /// Gets the list of groups
        /// </summary>
        public static IEnumerable<StaticSkill> AllSkills
        {
            get
            {
                foreach (var group in m_allGroupsById.Values)
                {
                    foreach (var skill in group)
                    {
                        yield return skill;
                    }
                }
            }
        }

        #endregion


        #region Public Finders
        
        /// <summary>
        /// Gets a skill by its id or its name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static StaticSkill GetSkill(this SerializableSkillPrerequisite src)
        {
            StaticSkill skill = GetSkillById(src.ID);

            if (skill == null)
                skill = GetSkillByName(src.Name);

            return skill;
        }

        /// <summary>
        /// Gets a skill by its name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static StaticSkill GetSkillByName(string name)
        {
            StaticSkill skill = null;
            m_skillsByName.TryGetValue(name, out skill);
            return skill;
        }

        /// <summary>
        /// Gets a skill by its identifier
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public static StaticSkill GetSkillById(long id)
        {
            StaticSkill skill = null;
            m_skillsById.TryGetValue(id, out skill);
            return skill;
        }

        /// <summary>
        /// Gets a skill by its array index
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public static StaticSkill GetSkillByArrayIndex(int index)
        {
            return m_skills[index];
        }

        /// <summary>
        /// Gets a group by its name
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static StaticSkillGroup GetGroupByID(int groupId)
        {
            StaticSkillGroup group = null;
            m_allGroupsById.TryGetValue(groupId, out group);
            return group;
        }

        #endregion


        #region Initializers

        /// <summary>
        /// Initialize static skills
        /// </summary>
        internal static void Load()
        {
            SkillsDatafile datafile = Util.DeserializeDatafile<SkillsDatafile>(DatafileConstants.SkillsDatafile);

            // Fetch deserialized data
            m_arrayIndicesCount = 0;
            var prereqs = new List<SerializableSkillPrerequisite[]>();
            foreach (var srcGroup in datafile.Groups)
            {
                var group = new StaticSkillGroup(srcGroup, ref m_arrayIndicesCount);
                m_allGroupsById[group.ID] = group;

                // Store skills
                foreach (var skill in group)
                {
                    m_skillsById[skill.ID] = skill;
                    m_skillsByName[skill.Name] = skill;
                }

                // Store prereqs
                foreach (var serialSkill in srcGroup.Skills)
                {
                    prereqs.Add(serialSkill.Prereqs);
                }
            }

            // Complete initialization
            m_skills = new StaticSkill[m_arrayIndicesCount];
            foreach (var ss in m_skillsById.Values)
            {
                ss.CompleteInitialization(prereqs[ss.ArrayIndex]);
                m_skills[ss.ArrayIndex] = ss;
            }
        }

        #endregion

    }
}

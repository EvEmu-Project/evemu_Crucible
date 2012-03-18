using System;
using System.Collections.Generic;
using EVEMon.Common.Data;
namespace EVEMon.Common
{

    #region ISkillLevel
    /// <summary>
    /// Represents a static skill and level tuple
    /// </summary>
    public interface ISkillLevel
    {
        int Level { get; }
        StaticSkill Skill { get; }
    }
    #endregion


    #region IStaticSkill
    public interface IStaticSkill
    {
        long ID { get; }
        int ArrayIndex { get; }
        string Name { get; }

        int Rank { get; }
        long Cost { get; }
        StaticSkillGroup Group { get; }

        IEnumerable<StaticSkillLevel> Prerequisites { get; }

        EveAttribute PrimaryAttribute { get; }
        EveAttribute SecondaryAttribute { get; }

        Skill ToCharacter(Character character);
    }
    #endregion


    #region IStaticSkillExtensions
    public static class IStaticSkillExtensions
    {
        /// <summary>
        /// Checks whether a certain skill is a prerequisite of this skill.
        /// The check is performed recursively through all prerequisites.
        /// </summary>
        /// <param name="skill">Skill to check.</param>
        /// <returns><code>true</code> if it is a prerequisite.</returns>
        public static bool HasAsPrerequisite(this IStaticSkill thisSkill, IStaticSkill skill)
        {
            int neededLevel = 0;
            return thisSkill.HasAsPrerequisite(skill, ref neededLevel, true);
        }

        /// <summary>
        /// Checks whether a certain skill is a prerequisite of this skill, and what level it needs.
        /// The check is performed recursively through all prerequisites.
        /// </summary>
        /// <param name="skill">Skill to check.</param>
        /// <param name="neededLevel">The level that is needed. Out parameter.</param>
        /// <returns><code>true</code> if it is a prerequisite, needed level in <var>neededLevel</var> out parameter.</returns>
        public static bool HasAsPrerequisite(this IStaticSkill thisSkill, IStaticSkill skill, out int neededLevel)
        {
            neededLevel = 0;
            return thisSkill.HasAsPrerequisite(skill, ref neededLevel, true);
        }

        /// <summary>
        /// Checks whether a certain skill is a prerequisite of this skill, and what level it needs.
        /// Find the highest level needed by searching entire prerequisite tree.
        /// </summary>
        /// <param name="skill">Skill to check.</param>
        /// <param name="neededLevel">The level that is needed. Out parameter.</param>
        /// <param name="recurse">Pass <code>true</code> to check recursively.</param>
        /// <returns><code>true</code> if it is a prerequisite, needed level in <var>neededLevel</var> out parameter.</returns>
        private static bool HasAsPrerequisite(this IStaticSkill thisSkill, IStaticSkill skill, ref int neededLevel, bool recurse)
        {
            var thisID = thisSkill.ID;

            foreach (var prereq in thisSkill.Prerequisites)
            {
                if (prereq.Skill == skill)
                {
                    neededLevel = Math.Max(prereq.Level, neededLevel);
                }

                if (recurse && neededLevel < 5 && prereq.Skill.ID != thisID) // check for neededLevel fixes recursuve skill bug (e.g polaris )
                {
                    prereq.Skill.HasAsPrerequisite(skill, ref neededLevel, true);
                }
            }
            return (neededLevel > 0);
        }

        /// <summary>
        /// Checks whether a certain skill is an immediate prerequisite of this skill,
        /// and the level needed
        /// </summary>
        /// <param name="skill">Skill that may be an immediate prereq</param>
        /// <param name="neededLevel">needed level of skill</param>
        /// <returns>Skill gs is an immediate prereq of this skill</returns>
        public static bool HasAsImmediatePrereq(this IStaticSkill thisSkill, IStaticSkill skill, out int neededLevel)
        {
            neededLevel = 0;
            return thisSkill.HasAsPrerequisite(skill, ref neededLevel, false);
        }
    }
    #endregion
}

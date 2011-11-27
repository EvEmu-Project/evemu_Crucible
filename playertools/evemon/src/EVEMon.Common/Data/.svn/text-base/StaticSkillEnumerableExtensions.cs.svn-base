using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Data
{
    public static class StaticSkillEnumerableExtensions
    {
        /// <summary>
        /// Rreturns an equivalent enumeration with character skills.
        /// </summary>
        /// <param name="src"></param>
        /// <param name="character"></param>
        /// <returns></returns>
        public static IEnumerable<Skill> ToCharacter(this IEnumerable<StaticSkill> src, Character character)
        {
            foreach (var item in src)
            {
                yield return character.Skills[item];
            }
        }

        /// <summary>
        /// Gets all the prerequisites. I.e, for eidetic memory, it will return <c>{ instant recall IV }</c>. 
        /// The order matches the hirerarchy but skills are not duplicated and are systematically trained to the highest required level.
        /// For example, if some skill is required to lv3 and, later, to lv4, this first time it is encountered, lv4 is returned.
        /// </summary>
        /// <remarks>Please note they may be redundancies.</remarks>
        public static IEnumerable<StaticSkillLevel> GetAllPrerequisites(this IEnumerable<StaticSkill> src)
        {
            int[] highestLevels = new int[StaticSkills.ArrayIndicesCount];
            List<StaticSkillLevel> list = new List<StaticSkillLevel>();

            // Fill the array
            foreach (var item in src)
            {
                foreach (var prereq in item.Prerequisites)
                {
                    FillPrerequisites(highestLevels, list, prereq, true);
                }
            }

            // Return the result
            foreach (var newItem in list)
            {
                if (highestLevels[newItem.Skill.ArrayIndex] != 0)
                {
                    yield return new StaticSkillLevel(newItem.Skill, highestLevels[newItem.Skill.ArrayIndex]);
                    highestLevels[newItem.Skill.ArrayIndex] = 0;
                }
            }
        }

        /// <summary>
        /// Fills the given levels array with the prerequisites and, when <c>includeRoots</c> is true, the item level itself
        /// </summary>
        /// <param name="highestLevels"></param>
        /// <param name="list"></param>
        /// <param name="item"></param>
        /// <param name="includeRoots"></param>
        internal static void FillPrerequisites(int[] highestLevels, List<StaticSkillLevel> list, StaticSkillLevel item, bool includeRoots)
        {
            // Prerequisites
            if (highestLevels[item.Skill.ArrayIndex] == 0)
            {
                foreach (var prereq in item.Skill.Prerequisites)
                {
                    // Deal with recursive skills such as Polaris
                    if (prereq.Skill != item.Skill)
                    {
                        FillPrerequisites(highestLevels, list, prereq, true);
                    }
                }
            }

            // The very level
            if (includeRoots && highestLevels[item.Skill.ArrayIndex] < item.Level) 
            {
                highestLevels[item.Skill.ArrayIndex] = item.Level;
                list.Add(item);
            }
        }
    }
}

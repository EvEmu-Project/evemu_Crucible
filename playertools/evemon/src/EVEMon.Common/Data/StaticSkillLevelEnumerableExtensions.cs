using System.Collections.Generic;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Extension methods for objects of <see cref="StaticSkillLevel">StaticSkillLevel</see> objects.
    /// </summary>
    public static class StaticSkillLevelEnumerableExtensions
    {
        
        #region Methods on Enumerations of StaticSkillLevel

        /// <summary>
        /// Returns an equivalent enumeration with character skills.
        /// </summary>
        /// <param name="src"></param>
        /// <param name="character"></param>
        /// <returns></returns>
        public static IEnumerable<SkillLevel> ToCharacter(this IEnumerable<StaticSkillLevel> src, Character character)
        {
            foreach (var item in src)
            {
                yield return new SkillLevel(character.Skills[item.Skill], item.Level);
            }
        }

        /// <summary>
        /// Gets all the dependencies, in a way matching the hirarchical order and without redudancies.
        /// I.e, for eidetic memory II, it will return <c>{ instant recall I, instant recall II, instant recall III, instant recall IV,  eidetic memory I, eidetic memory II }</c>.
        /// </summary>
        /// <param name="includeRoots">When true, the levels in this enumeration are also included.</param>
        public static IEnumerable<StaticSkillLevel> GetAllDependencies(this IEnumerable<StaticSkillLevel> src, bool includeRoots)
        {
            var set = new SkillLevelSet<StaticSkillLevel>();
            var list = new List<StaticSkillLevel>();

            // Fill the set and list
            foreach (var item in src)
            {
                FillDependencies(set, list, item, includeRoots);
            }

            // Return the results
            foreach (var item in list)
            {
                yield return item;
            }
        }

        #endregion


        #region Internal Static Methods

        /// <summary>
        /// Add the item, its previous levels and its prerequisites to the given set and list.
        /// </summary>
        /// <param name="set"></param>
        /// <param name="list"></param>
        /// <param name="item"></param>
        /// <param name="includeRoots"></param>
        internal static void FillDependencies(SkillLevelSet<StaticSkillLevel> set, List<StaticSkillLevel> list, StaticSkillLevel item, bool includeRoots)
        {
            var skill = item.Skill;

            // Add first level and prerequisites
            if (!set.Contains(skill, 1))
            {
                // Prerequisites
                foreach (var prereq in skill.Prerequisites)
                {
                    // Deal with recursive skills such as Polaris
                    if (skill != prereq.Skill)
                    {
                        FillDependencies(set, list, prereq, true);
                    }
                }

                // Include the first level
                var newItem = new StaticSkillLevel(skill, 1);
                list.Add(newItem);
                set.Set(newItem);
            }

            // Add greater levels
            int max = (includeRoots ? item.Level : item.Level - 1);
            for (int i = 2; i <= max; i++) 
            {
                if (!set.Contains(skill, i))
                {
                    var newItem = new StaticSkillLevel(skill, i);
                    list.Add(newItem);
                    set.Set(newItem);
                }
            }
        }

        #endregion

    }
}

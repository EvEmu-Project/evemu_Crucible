using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    public static class SkillLevelEnumerableExtensions
    {
        /// <summary>
        /// Returns an enumeration of the static equivalent of the items.
        /// </summary>
        /// <param name="src"></param>
        /// <returns></returns>
        public static IEnumerable<StaticSkillLevel> ToStatic(this IEnumerable<SkillLevel> src)
        {
            foreach (var item in src)
            {
                yield return new StaticSkillLevel(item);
            }
        }

        /// <summary>
        /// Gets all the dependencies, in a way matching the hirarchical order and without redudancies.
        /// I.e, for eidetic memory II, it will return <c>{ instant recall I, instant recall II, instant recall III, instant recall IV,  eidetic memory I, eidetic memory II }</c>.
        /// </summary>
        /// <param name="includeRoots">When true, the levels in this enumeration are also included.</param>
        public static IEnumerable<SkillLevel> GetAllDependencies(this IEnumerable<SkillLevel> src, bool includeRoots)
        {
            var first = src.FirstOrDefault();
            if (first.Skill == null) return EmptyEnumerable<SkillLevel>.Instance;
            return src.ToStatic().GetAllDependencies(includeRoots).ToCharacter(first.Skill.Character);
        }

        /// <summary>
        /// Gets true if all the levels are known
        /// </summary>
        /// <param name="src"></param>
        public static bool AreTrained(this IEnumerable<SkillLevel> src)
        {
            return src.All(x => x.IsKnown);
        }

        /// <summary>
        /// Checks whether those prerequisites contains the provided skill, returning the need level
        /// </summary>
        /// <param name="skill"></param>
        /// <param name="neededLevel"></param>
        /// <returns></returns>
        public static bool Contains(this IEnumerable<SkillLevel> src, Skill skill, out int neededLevel)
        {
            neededLevel = 0;
            foreach (var prereq in src)
            {
                if (prereq.Skill == skill)
                {
                    neededLevel = prereq.Level;
                    return true;
                }
            }
            return false;
        }

        #region GetTotalTrainingTime
        /// <summary>
        /// Gets the points required to train all the prerequisites
        /// </summary>
        /// <returns></returns>
        public static TimeSpan GetTotalTrainingTime(this IEnumerable<SkillLevel> src)
        {
            bool junk = false;
            return src.GetTotalTrainingTime(new Dictionary<Skill, int>(), ref junk);
        }

        /// <summary>
        /// Gets the points required to train all the prerequisites
        /// </summary>
        /// <returns></returns>
        public static TimeSpan GetTotalTrainingTime(this IEnumerable<SkillLevel> src, Dictionary<Skill, int> alreadyCountedList)
        {
            bool junk = false;
            return src.GetTotalTrainingTime(alreadyCountedList, ref junk);
        }

        /// <summary>
        /// Gets the points required to train all the prerequisites
        /// </summary>
        /// <returns></returns>
        public static TimeSpan GetTotalTrainingTime(this IEnumerable<SkillLevel> src, ref bool isCurrentlyTraining)
        {
            return src.GetTotalTrainingTime(new Dictionary<Skill, int>(), ref isCurrentlyTraining);
        }

        /// <summary>
        /// Gets the time required to train all the prerequisites
        /// </summary>
        /// <returns></returns>
        public static TimeSpan GetTotalTrainingTime(this IEnumerable<SkillLevel> src, Dictionary<Skill, int> alreadyCountedList, ref bool isCurrentlyTraining)
        {
            TimeSpan result = TimeSpan.Zero;
            foreach (var item in src)
            {
                Skill skill = item.Skill;
                isCurrentlyTraining |= skill.IsTraining;

                // Gets the number of points we're starting from
                int fromPoints = skill.SkillPoints;
                if (alreadyCountedList.ContainsKey(skill))
                {
                    fromPoints = alreadyCountedList[skill];
                }

                // Gets the number of points we're targeting
                int toPoints = skill.GetLeftPointsRequiredToLevel(item.Level);
                if (fromPoints < toPoints)
                {
                    result += skill.GetTimeSpanForPoints(toPoints - fromPoints);
                }

                // Updates the alreadyCountedList
                alreadyCountedList[skill] = Math.Max(fromPoints, toPoints);

                // Recursive
                if (fromPoints < toPoints)
                {
                    result += skill.Prerequisites.GetTotalTrainingTime(alreadyCountedList, ref isCurrentlyTraining);
                }
            }
            return result;
        }
        #endregion
    }
}

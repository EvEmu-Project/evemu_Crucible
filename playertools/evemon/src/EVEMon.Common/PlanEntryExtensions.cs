using System.Collections.Generic;

using EVEMon.Common.Data;

namespace EVEMon.Common
{
    public static class PlanEntryExtensions
    {
        /// <summary>
        /// Gets the total number of unique skills (two levels of same skill counts for one unique skill).
        /// </summary>
        /// <param name="items">List of <see cref="PlanEntry" />.</param>
        /// <returns>Count of unique skills.</returns>
        public static int GetUniqueSkillsCount(this IEnumerable<PlanEntry> items)
        {
            int count = 0;
            bool[] counted = new bool[StaticSkills.ArrayIndicesCount];

            // Scroll through entries
            foreach (var pe in items)
            {
                int index = pe.Skill.ArrayIndex;
                if (!counted[index])
                {
                    counted[index] = true;
                    count++;
                }
            }

            // Return the count
            return count;
        }

        /// <summary>
        /// Gets the number of not known skills selected (two levels of same skill counts for one unique skill).
        /// </summary>
        /// <param name="items">List of <see cref="PlanEntry" />.</param>
        /// <returns>Count of known skills.</returns>
        public static int GetNotKnownSkillsCount(this IEnumerable<PlanEntry> items)
        {
            int count = 0;
            bool[] counted = new bool[StaticSkills.ArrayIndicesCount];

            // Scroll through selection
            foreach (var pe in items)
            {
                int index = pe.Skill.ArrayIndex;
                if (!counted[index] && !pe.CharacterSkill.IsKnown && !pe.CharacterSkill.IsOwned)
                {
                    counted[index] = true;
                    count++;
                }
            }

            // Return the count
            return count;
        }

        /// <summary>
        /// Gets the total cost of the skill books, in ISK
        /// </summary>
        /// <param name="items">List of <see cref="PlanEntry" />.</param>
        /// <returns>Cumulative cost of all skill books.</returns>
        public static long GetTotalBooksCost(this IEnumerable<PlanEntry> items)
        {
            long cost = 0;
            bool[] counted = new bool[StaticSkills.ArrayIndicesCount];

            // Scroll through entries
            foreach (var pe in items)
            {
                int index = pe.Skill.ArrayIndex;
                if (!counted[index])
                {
                    counted[index] = true;
                    cost += pe.Skill.Cost;
                }
            }

            // Return the cost
            return cost;
        }

        /// <summary>
        /// Gets the cost of the not known skill books, in ISK
        /// </summary>
        /// <param name="items">List of <see cref="PlanEntry" />.</param>
        /// <returns>Cumulative cost of known skill books.</returns>
        public static long GetNotKnownSkillBooksCost(this IEnumerable<PlanEntry> items)
        {
            long cost = 0;
            bool[] counted = new bool[StaticSkills.ArrayIndicesCount];

            // Scroll through entries
            foreach (var pe in items)
            {
                int index = pe.Skill.ArrayIndex;
                if (!counted[index] && !pe.CharacterSkill.IsKnown && !pe.CharacterSkill.IsOwned)
                {
                    counted[index] = true;
                    cost += pe.Skill.Cost;
                }
            }

            // Return the cost
            return cost;
        }
    }
}

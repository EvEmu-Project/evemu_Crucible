using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// This class holds the responsibility to sort enumerations of plan entries.
    /// </summary>
    internal sealed class PlanSorter
    {
        private PlanEntrySort m_sort;
        private bool m_reverseOrder;
        private bool m_groupByPriority;
        private IEnumerable<PlanEntry> m_entries;
        private BaseCharacter m_character;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="entries"></param>
        /// <param name="sort"></param>
        /// <param name="reverseOrder"></param>
        /// <param name="groupByPriority"></param>
        internal PlanSorter(BaseCharacter character, IEnumerable<PlanEntry> entries, PlanEntrySort sort, bool reverseOrder, bool groupByPriority)
        {
            m_sort = sort;
            m_entries = entries;
            m_reverseOrder = reverseOrder;
            m_groupByPriority = groupByPriority;
            m_character = character;
        }

        /// <summary>
        /// Performs the sort.
        /// </summary>
        /// <returns></returns>
        public IEnumerable<PlanEntry> Sort()
        {
            int initialCount = m_entries.Count();

            // Apply first pass (priorities grouping)
            // We split the entries into multiple priority groups if that selection is made
            List<PlanScratchpad> groupedPlan = new List<PlanScratchpad>();
            var scratchpad = new CharacterScratchpad(m_character);

            if (m_groupByPriority)
            {
                foreach (var group in m_entries.GroupBy(x => x.Priority).OrderBy(x => x.Key))
                {
                    groupedPlan.Add(new PlanScratchpad(scratchpad, group));
                }
            }
            else
            {
                groupedPlan.Add(new PlanScratchpad(scratchpad, m_entries));
            }

            // Apply second pass (sorts)
            // We sort every group, and merge them once they're sorted
            List<PlanEntry> list = new List<PlanEntry>();

            foreach (var group in groupedPlan)
            {
                group.UpdateStatistics(scratchpad, false, false);
                group.SimpleSort(m_sort, m_reverseOrder);
                list.AddRange(group);
            }

            // Fix prerequisites order
            FixPrerequisitesOrder(list);

            // Check we didn't mess up anything
            if (initialCount != list.Count)
                throw new UnauthorizedAccessException("The sort algorithm messed up and deleted items");

            // Return
            return list;
        }

        /// <summary>
        /// Ensures the prerequisites order is correct.
        /// </summary>
        /// <param name="list"></param>
        private static void FixPrerequisitesOrder(List<PlanEntry> list)
        {
            // Gather prerequisites/postrequisites relationships and use them to connect nodes - O(n²) operation
            var dependencies = new Dictionary<PlanEntry,List<PlanEntry>>();
            foreach (var entry in list)
            {
                dependencies[entry] = new List<PlanEntry>(list.Where(x => entry.IsDependentOf(x)));
            }


            // Insert entries
            var entriesToAdd = new LinkedList<PlanEntry>(list);
            var set = new SkillLevelSet<PlanEntry>();
            list.Clear();

            while (entriesToAdd.Count != 0)
            {
                // Gets the first entry which has all its prerequisites satisfied.
                var item = entriesToAdd.First(x => dependencies[x].All(y => set[y.Skill, y.Level] != null));

                // Add it to the set and list, and remove it from the entries to add
                set[item.Skill, item.Level] = item;
                entriesToAdd.Remove(item);
                list.Add(item);
            }
        }


        #region Simple sort operators

        public static int CompareByName(PlanEntry x, PlanEntry y)
        {
            int nameDiff = String.CompareOrdinal(x.Skill.Name, y.Skill.Name);
            if (nameDiff != 0) return nameDiff;
            return x.Level - y.Level;
        }

        public static int CompareByCost(PlanEntry x, PlanEntry y)
        {
            long xCost = (x.Level == 1 && !x.CharacterSkill.IsOwned ? x.Skill.Cost : 0);
            long yCost = (y.Level == 1 && !x.CharacterSkill.IsOwned ? y.Skill.Cost : 0);
            return xCost.CompareTo(yCost);
        }

        public static int CompareByTrainingTime(PlanEntry x, PlanEntry y)
        {
            return x.TrainingTime.CompareTo(y.TrainingTime);
        }

        public static int CompareByTrainingTimeNatural(PlanEntry x, PlanEntry y)
        {
            return x.NaturalTrainingTime.CompareTo(y.NaturalTrainingTime);
        }

        public static int CompareBySPPerHour(PlanEntry x, PlanEntry y)
        {
            return x.SpPerHour - y.SpPerHour;
        }

        public static int CompareByPrimaryAttribute(PlanEntry x, PlanEntry y)
        {
            return (int)x.Skill.PrimaryAttribute - (int)y.Skill.PrimaryAttribute;
        }

        public static int CompareBySecondaryAttribute(PlanEntry x, PlanEntry y)
        {
            return (int)x.Skill.SecondaryAttribute - (int)y.Skill.SecondaryAttribute;
        }

        public static int CompareByPriority(PlanEntry x, PlanEntry y)
        {
            return x.Priority - y.Priority;
        }

        public static int CompareByPlanGroup(PlanEntry x, PlanEntry y)
        {
            return String.Compare(x.PlanGroupsDescription, y.PlanGroupsDescription);
        }

        public static int CompareByPlanType(PlanEntry x, PlanEntry y)
        {
            return (int)x.Type - (int)y.Type;
        }

        public static int CompareByNotes(PlanEntry x, PlanEntry y)
        {
            return String.Compare(x.Notes, y.Notes);
        }

        public static int CompareByTimeDifference(PlanEntry x, PlanEntry y)
        {
            var xDuration = x.TrainingTime - x.OldTrainingTime;
            var yDuration = y.TrainingTime - y.OldTrainingTime;
            return xDuration.CompareTo(yDuration);
        }

        public static int CompareByPercentCompleted(PlanEntry x, PlanEntry y)
        {
            float xRatio = x.CharacterSkill.FractionCompleted;
            float yRatio = y.CharacterSkill.FractionCompleted;
            return xRatio.CompareTo(yRatio);
        }

        public static int CompareByRank(PlanEntry x, PlanEntry y)
        {
            return x.Skill.Rank - y.Skill.Rank;
        }

        public static int CompareBySkillPointsRequired(PlanEntry x, PlanEntry y)
        {
            return x.SkillPointsRequired.CompareTo(y.SkillPointsRequired);
        }

        public static int CompareBySkillGroupDuration(PlanEntry x, PlanEntry y, IEnumerable<PlanEntry> entries, Dictionary<StaticSkillGroup, TimeSpan> skillGroupsDurations)
        {
            var xDuration = GetSkillGroupDuration(x.Skill.Group, entries, skillGroupsDurations);
            var yDuration = GetSkillGroupDuration(y.Skill.Group, entries, skillGroupsDurations);
            return xDuration.CompareTo(yDuration);
        }

        private static TimeSpan GetSkillGroupDuration(StaticSkillGroup group, IEnumerable<PlanEntry> entries, Dictionary<StaticSkillGroup, TimeSpan> skillGroupsDurations)
        {
            if (!skillGroupsDurations.ContainsKey(group))
            {
                TimeSpan time = TimeSpan.Zero;
                foreach (var entry in entries.Where(x => x.Skill.Group == group))
                {
                    time += entry.TrainingTime;
                }
                skillGroupsDurations[group] = time;
            }
            
            return skillGroupsDurations[group];
        }

        #endregion
    }
}

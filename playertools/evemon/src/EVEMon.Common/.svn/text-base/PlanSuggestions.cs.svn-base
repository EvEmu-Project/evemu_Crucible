using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Takes the responsibility of evaluating suggestions for a plan. 
    /// </summary>
    public sealed class PlanSuggestions : ReadonlyCollection<PlanEntry>
    {
        private readonly TimeSpan m_timeBenefit;
        private readonly PlanScratchpad m_plan;
        private readonly Plan m_originalPlan;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="plan">The plan to get suggestions for, it will be unchanged.</param>
        internal PlanSuggestions(Plan plan)
        {
            var character = (plan.ChosenImplantSet != null ? plan.Character.After(plan.ChosenImplantSet) : plan.Character);
            m_plan = new PlanScratchpad(character, plan);
            m_originalPlan = plan;

            var baseTime = m_plan.TotalTrainingTime;
            using (m_plan.SuspendingEvents())
            {
                // Search for new benefits, in a recursive way (adding new skills change the opportunities for new improvements)
                while (true)
                {
                    bool foundNewBenefits = false;
                    foundNewBenefits |= CheckForTimeBenefit(GetLearningEntries(EveAttribute.Intelligence));
                    foundNewBenefits |= CheckForTimeBenefit(GetLearningEntries(EveAttribute.Perception));
                    foundNewBenefits |= CheckForTimeBenefit(GetLearningEntries(EveAttribute.Willpower));
                    foundNewBenefits |= CheckForTimeBenefit(GetLearningEntries(EveAttribute.Memory));
                    foundNewBenefits |= CheckForTimeBenefit(GetLearningEntries(EveAttribute.Charisma));
                    foundNewBenefits |= CheckForTimeBenefit(GetLearningEntries());
                    if (!foundNewBenefits)
                        break;
                }
            }

            // Update the time benefit
            m_timeBenefit = baseTime - m_plan.TotalTrainingTime;
        }

        /// <summary>
        /// Gets the time benefit if the suggested skills were added.
        /// </summary>
        public TimeSpan TimeBenefit
        {
            get { return m_timeBenefit; }
        }

        /// <summary>
        /// Apply the suggestions to the plan
        /// </summary>
        public void ApplySuggestions()
        {
            m_originalPlan.TryAddSet(m_items, "Suggestions").PerformAddition(1);
            m_originalPlan.SortLearningSkills(false);
        }

        /// <summary>
        /// Check, among the sequence of skills, the number of them to add to get the best training time. 
        /// Matched skills are then added to the plan and to the suggestions list.
        /// </summary>
        /// <param name="attribute"></param>
        /// <returns></returns>
        private bool CheckForTimeBenefit(IEnumerable<PlanEntry> entries)
        {
            // Gets the skills which are neither known nor planned
            var entriesToAdd = new List<PlanEntry>();
            foreach (var entry in entries)
            {
                if (entry.CharacterSkill.Level < entry.Level && !m_plan.IsPlanned(entry.Skill, entry.Level))
                    entriesToAdd.Add(entry);
            }

            // Check which is the best level to train
            int entriesCount = 0;
            int bestEntriesCount = 0;
            var bestTime = m_plan.TotalTrainingTime;
            var testPlan = new PlanScratchpad(m_plan.Character, m_plan);

            foreach (var entry in entriesToAdd)
            {
                // Insert the skill at the best position, making sure its prerequisites are trained first.
                entriesCount++;
                testPlan.InsertAtBestPosition(entry.Skill, entry.Level);

                // Compare the new training time with the old one
                TimeSpan newTime = testPlan.TotalTrainingTime;
                if (newTime < bestTime)
                {
                    bestTime = newTime;
                    bestEntriesCount = entriesCount;
                }
            }

            // Add the suggested entries to the plan and the suggestions list
            entriesCount = 0;
            foreach(var entry in entriesToAdd)
            {
                // Stop when we reached the desired entries count
                if (entriesCount == bestEntriesCount)
                    break;

                entriesCount++;

                m_items.Add(entry);
                m_plan.InsertAtBestPosition(entry.Skill, entry.Level);
            }

            // Returns true if new benefits have been found
            return (bestEntriesCount != 0);
        }

        /// <summary>
        /// Gets the list of plan entries for learning skills for a given attribute, sorted by trainng time (1234123455).
        /// </summary>
        /// <param name="attributes"></param>
        /// <returns></returns>
        private IEnumerable<PlanEntry> GetLearningEntries(EveAttribute attributes)
        {
            StaticSkill lower = StaticSkills.GetLowerAttributeLearningSkill(attributes);
            StaticSkill upper = StaticSkills.GetUpperAttributeLearningSkill(attributes);

            yield return new PlanEntry(m_originalPlan, lower, 1);
            yield return new PlanEntry(m_originalPlan, lower, 2);
            yield return new PlanEntry(m_originalPlan, lower, 3);
            yield return new PlanEntry(m_originalPlan, lower, 4);
            yield return new PlanEntry(m_originalPlan, upper, 1);
            yield return new PlanEntry(m_originalPlan, upper, 2);
            yield return new PlanEntry(m_originalPlan, upper, 3);
            yield return new PlanEntry(m_originalPlan, upper, 4);
            yield return new PlanEntry(m_originalPlan, lower, 5);
            yield return new PlanEntry(m_originalPlan, upper, 5);
        }

        /// <summary>
        /// Gets the list of plan entries for the learning skills.
        /// </summary>
        /// <param name="attributes"></param>
        /// <returns></returns>
        private IEnumerable<PlanEntry> GetLearningEntries()
        {
            StaticSkill skill = StaticSkills.LearningSkill;

            yield return new PlanEntry(m_originalPlan, skill, 1);
            yield return new PlanEntry(m_originalPlan, skill, 2);
            yield return new PlanEntry(m_originalPlan, skill, 3);
            yield return new PlanEntry(m_originalPlan, skill, 4);
            yield return new PlanEntry(m_originalPlan, skill, 5);
        }
    }
}

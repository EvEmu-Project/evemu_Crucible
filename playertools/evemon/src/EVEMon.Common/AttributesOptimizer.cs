using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Text;
using EVEMon.Common;

namespace EVEMon.Common
{
    public static class AttributesOptimizer
    {
        #region Remapping

        /// <summary>
        /// Represents the result of a remapping
        /// </summary>
        public sealed class RemappingResult
        {
            private readonly List<ISkillLevel> m_skills = new List<ISkillLevel>();
            private readonly CharacterScratchpad m_baseScratchpad;
            private readonly RemappingPoint m_point;
            private readonly TimeSpan m_startTime;

            private CharacterScratchpad m_bestScratchpad;
            private TimeSpan m_baseDuration;
            private TimeSpan m_bestDuration;

            /// <summary>
            /// Constructor without any remapping point associated 
            /// </summary>
            /// <param name="baseScratchpad"></param>
            public RemappingResult(CharacterScratchpad baseScratchpad)
            {
                m_baseScratchpad = baseScratchpad;
                m_startTime = m_baseScratchpad.TrainingTime;
            }

            /// <summary>
            /// Constructor for a result bound to a remapping point
            /// </summary>
            /// <param name="point">Associated remapping point, may be null.</param>
            /// <param name="baseScratchpad"></param>
            public RemappingResult(RemappingPoint point, CharacterScratchpad baseScratchpad)
                : this(baseScratchpad)
            {
                m_point = point;
            }

            /// <summary>
            /// Constructor for a manually edited result from a base result.
            /// </summary>
            /// <param name="point">Associated remapping point, may be null.</param>
            /// <param name="baseScratchpad"></param>
            /// <param name="bestScratchpad"></param>
            public RemappingResult(RemappingResult result, CharacterScratchpad bestScratchpad)
                : this(result.Point, result.BaseScratchpad)
            {
                m_skills.AddRange(result.m_skills);
                m_bestScratchpad = bestScratchpad;
            }

            /// <summary>
            /// Gets the optimized plan
            /// </summary>
            public List<ISkillLevel> Skills
            {
                get { return m_skills; }
            }

            /// <summary>
            /// Gets the remapping point associated with that remapping. May be null if a remapping was automatically added at the beginning of the training
            /// </summary>
            public RemappingPoint Point
            {
                get { return m_point; }
            }

            /// <summary>
            /// Gets the best scratchpad after the remapping
            /// </summary>
            public CharacterScratchpad BaseScratchpad
            {
                get { return m_baseScratchpad; }
            }

            /// <summary>
            /// Gets the best scratchpad after the remapping
            /// </summary>
            public CharacterScratchpad BestScratchpad
            {
                get { return m_bestScratchpad; }
            }

            /// <summary>
            /// Gets the training duration with the best remapping
            /// </summary>
            public TimeSpan BestDuration
            {
                get { return m_bestDuration; }
            }

            /// <summary>
            /// Gets the base training duration before the remapping
            /// </summary>
            public TimeSpan BaseDuration
            {
                get { return m_baseDuration; }
            }

            /// <summary>
            /// Gets the time when this remapping was done
            /// </summary>
            public TimeSpan StartTime
            {
                get { return m_startTime; }
            }

            /// <summary>
            /// Computes an optimized scratchpad, then call <see cref="Update"/>.
            /// </summary>
            /// <param name="maxDuration">The max duration to take into account for optimization.</param>
            /// <returns></returns>
            public void Optimize(TimeSpan maxDuration)
            {
                m_bestScratchpad = AttributesOptimizer.Optimize(m_skills, m_baseScratchpad, maxDuration);
                Update();
            }

            /// <summary>
            /// Updates the times and, when any, the associated remapping point.
            /// </summary>
            /// <returns></returns>
            public void Update()
            {
                // Optimize
                m_baseDuration = m_baseScratchpad.After(m_skills).TrainingTime - m_startTime;
                m_bestDuration = m_bestScratchpad.After(m_skills).TrainingTime - m_startTime;

                // Update the underlying remapping point
                if (m_point != null)
                    m_point.SetBaseAttributes(m_bestScratchpad, m_baseScratchpad);
            }

            /// <summary>
            /// Gets a string representation of this object.
            /// </summary>
            /// <param name="character"></param>
            /// <returns></returns>
            public string ToString(Character character)
            {
                StringBuilder builder = new StringBuilder().
                    Append("i").Append(m_bestScratchpad.Intelligence.Base.ToString()).
                    Append(" p").Append(m_bestScratchpad.Perception.Base.ToString()).
                    Append(" c").Append(m_bestScratchpad.Charisma.Base.ToString()).
                    Append(" w").Append(m_bestScratchpad.Willpower.Base.ToString()).
                    Append(" m").Append(m_bestScratchpad.Memory.Base.ToString());

                return builder.ToString();
            }
        }

        #endregion


        #region Computations

        /// <summary>
        /// Compute the best possible attributes to fulfill the given trainings array
        /// </summary>
        /// <param name="skills"></param>
        /// <param name="baseScratchpad"></param>
        /// <param name="maxDuration"></param>
        /// <returns></returns>
        private static CharacterScratchpad Optimize<T>(IEnumerable<T> skills, CharacterScratchpad baseScratchpad, TimeSpan maxDuration)
            where T : ISkillLevel
        {
            CharacterScratchpad bestScratchpad = new CharacterScratchpad(baseScratchpad);
            CharacterScratchpad tempScratchpad = new CharacterScratchpad(baseScratchpad);
            TimeSpan baseTime = baseScratchpad.TrainingTime;
            TimeSpan bestTime = TimeSpan.MaxValue;
            int bestSkillCount = 0;

            // Now, we have the points to spend, let's perform all the
            // combinations (less than 11^4 = 14,641)
            for (int per = 0; per <= EveConstants.MaxRemappablePointsPerAttribute; per++)
            {
                // WIL
                int maxWillpower = EveConstants.SpareAttributePointsOnRemap - per;
                for (int will = 0; will <= maxWillpower && will <= EveConstants.MaxRemappablePointsPerAttribute; will++)
                {
                    // INT
                    int maxIntelligence = maxWillpower - will;
                    for (int intell = 0; intell <= maxIntelligence && intell <= EveConstants.MaxRemappablePointsPerAttribute; intell++)
                    {
                        // MEM
                        int maxMemory = maxIntelligence - intell;
                        for (int mem = 0; mem <= maxMemory && mem <= EveConstants.MaxRemappablePointsPerAttribute; mem++)
                        {
                            // CHA
                            int cha = maxMemory - mem;

                            // Reject invalid combinations
                            if (cha <= EveConstants.MaxRemappablePointsPerAttribute)
                            {
                                // Resets the scratchpad
                                tempScratchpad.Reset();

                                // Set new attributes
                                tempScratchpad.Memory.Base = mem + EveConstants.CharacterBaseAttributePoints;
                                tempScratchpad.Charisma.Base = cha + EveConstants.CharacterBaseAttributePoints;
                                tempScratchpad.Willpower.Base = will + EveConstants.CharacterBaseAttributePoints;
                                tempScratchpad.Perception.Base = per + EveConstants.CharacterBaseAttributePoints;
                                tempScratchpad.Intelligence.Base = intell + EveConstants.CharacterBaseAttributePoints;

                                // Train skills
                                int tempSkillCount = 0;
                                foreach (var skill in skills)
                                {
                                    tempSkillCount++;
                                    tempScratchpad.Train(skill);

                                    // Did it go over max duration ?
                                    if (tempScratchpad.TrainingTime - baseTime > maxDuration) break;

                                    // Did it go over the best time so far without training more skills ?
                                    if (tempSkillCount <= bestSkillCount && tempScratchpad.TrainingTime > bestTime) break;
                                }

                                // Did it manage to train more skills before the max duration, 
                                // or did it train the same number of skills in a lesser time ?
                                if (tempSkillCount > bestSkillCount || (tempSkillCount == bestSkillCount && tempScratchpad.TrainingTime < bestTime))
                                {
                                    bestScratchpad.Reset();
                                    bestScratchpad.Memory.Base = tempScratchpad.Memory.Base;
                                    bestScratchpad.Charisma.Base = tempScratchpad.Charisma.Base;
                                    bestScratchpad.Willpower.Base = tempScratchpad.Willpower.Base;
                                    bestScratchpad.Perception.Base = tempScratchpad.Perception.Base;
                                    bestScratchpad.Intelligence.Base = tempScratchpad.Intelligence.Base;
                                    bestTime = tempScratchpad.TrainingTime;
                                    bestSkillCount = tempSkillCount;
                                }
                            }
                        }
                    }
                }
            }

            // Return the best scratchpad found
            return bestScratchpad;
        }


        /// <summary>
        /// Generate a trainings array from a plan
        /// </summary>
        /// <param name="plan"></param>
        /// <param name="bestDuration"></param>
        /// <returns></returns>
        public static List<RemappingResult> OptimizeFromPlanAndRemappingPoints(BasePlan plan)
        {
            var results = GetResultsFromRemappingPoints(plan);
            foreach (var result in results)
            {
                result.Optimize(TimeSpan.MaxValue);
            }
            return results;
        }

        /// <summary>
        /// Gets the list of remapping results from a plan.
        /// </summary>
        /// <param name="plan"></param>
        /// <returns></returns>
        public static List<RemappingResult> GetResultsFromRemappingPoints(BasePlan plan)
        {
            var scratchpad = new CharacterScratchpad(plan.Character.After(plan.ChosenImplantSet));
            var remappingList = new List<RemappingResult>();
            var list = new List<ISkillLevel>();
            RemappingResult remapping = null;

            // Scroll through the entries and split it into remappings
            foreach (var entry in plan)
            {
                // Ends the current remapping and start a new one
                if (entry.Remapping != null)
                {
                    // Creates a new remapping
                    remapping = new RemappingResult(entry.Remapping, scratchpad.Clone());
                    remappingList.Add(remapping);
                    list = remapping.Skills;
                }

                // Add this skill to the training list
                scratchpad.Train(entry);
                list.Add(entry);
            }

            // Return
            return remappingList;
        }

        /// <summary>
        /// Compute the best remapping for the first year of this plan.
        /// </summary>
        /// <param name="plan"></param>
        /// <returns></returns>
        public static RemappingResult OptimizeFromFirstYearOfPlan(BasePlan plan)
        {
            var remapping = new RemappingResult(new CharacterScratchpad(plan.Character.After(plan.ChosenImplantSet)));

            // Scroll through the entries and split it into remappings
            foreach (var entry in plan)
            {
                remapping.Skills.Add(entry);
            }

            // Compute
            remapping.Optimize(TimeSpan.FromDays(365.0));
            return remapping;
        }

        /// <summary>
        /// Generate a trainings array from the skills already know by a character.
        /// </summary>
        /// <param name="character">The character.</param>
        /// <param name="plan">The plan.</param>
        /// <returns></returns>
        public static RemappingResult OptimizeFromCharacter(Character character, BasePlan plan)
        {
            // Create a character without any skill
            var scratchpad = new CharacterScratchpad(character.After(plan.ChosenImplantSet));
            scratchpad.ClearSkills();

            // Create a new plan
            var newPlan = new Plan(scratchpad);

            // Add all trained skill levels that the character has trained so far
            foreach (var skill in character.Skills)
            {
                newPlan.PlanTo(skill, skill.Level);
            }

            // Create a new remapping
            var remapping = new RemappingResult(scratchpad);
            
            // Add those skills to the remapping
            foreach (var entry in newPlan)
            {
                remapping.Skills.Add(entry);
            }

            // Optimize
            remapping.Optimize(TimeSpan.MaxValue);
            return remapping;
        }

        #endregion
    }
}

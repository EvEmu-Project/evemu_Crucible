using System;
using System.Linq;
using System.Text;
using System.Threading;
using System.Collections.Generic;
using System.Collections.ObjectModel;

using EVEMon.Common.Data;
using EVEMon.Common.Attributes;
using EVEMon.Common.Controls;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a character's plan
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class Plan : BasePlan
    {
        #region PlanOperation
        /// <summary>
        /// This class is used to add entries. It enumerates the prerequisites to add, their lowest prioties, etc.
        /// </summary>
        private sealed class PlanOperation : IPlanOperation
        {
            private readonly Plan m_plan;
            private readonly PlanOperations m_type;

            // Addition
            private readonly int m_highestPriorityForAddition;
            private readonly List<ISkillLevel> m_skillsToAdd = new List<ISkillLevel>();
            private readonly List<PlanEntry> m_allEntriesToAdd = new List<PlanEntry>();

            // Suppression
            private readonly List<ISkillLevel> m_skillsToRemove = new List<ISkillLevel>();
            private readonly List<PlanEntry> m_allEntriesToRemove = new List<PlanEntry>();
            private readonly List<PlanEntry> m_removablePrerequisites = new List<PlanEntry>();


            /// <summary>
            /// Constructor for an empty operation.
            /// </summary>
            /// <param name="plan"></param>
            public PlanOperation(Plan plan)
            {
                m_plan = plan;
            }

            /// <summary>
            /// Constructor for entries addition.
            /// </summary>
            /// <param name="plan"></param>
            /// <param name="entries"></param>
            /// <param name="lowestPrerequisitesPriority"></param>
            public PlanOperation(Plan plan, IEnumerable<ISkillLevel> skillsToAdd, IEnumerable<PlanEntry> allEntriesToAdd, int lowestPrerequisitesPriority)
            {
                m_plan = plan;
                m_type = (skillsToAdd.IsEmpty() ? PlanOperations.None : PlanOperations.Addition);

                m_skillsToAdd.AddRange(skillsToAdd);
                m_allEntriesToAdd.AddRange(allEntriesToAdd);
                m_highestPriorityForAddition = lowestPrerequisitesPriority;
            }

            /// <summary>
            /// Constructor for entries suppression.
            /// </summary>
            /// <param name="plan"></param>
            /// <param name="entries"></param>
            /// <param name="highestPriority"></param>
            public PlanOperation(Plan plan, IEnumerable<ISkillLevel> skillsToRemove, IEnumerable<PlanEntry> allEntriesToRemove, IEnumerable<PlanEntry> removablePrerequisites)
            {
                m_plan = plan;
                m_type = (skillsToRemove.IsEmpty() ? PlanOperations.None : PlanOperations.Suppression);

                m_skillsToRemove.AddRange(skillsToRemove);
                m_allEntriesToRemove.AddRange(allEntriesToRemove);
                m_removablePrerequisites.AddRange(removablePrerequisites);
            }

            /// <summary>
            /// Gets the type of operation to perform
            /// </summary>
            public PlanOperations Type
            {
                get { return m_type; }
            }

            /// <summary>
            /// Gets the plan affected by this operation.
            /// </summary>
            public Plan Plan 
            {
                get { return m_plan; } 
            }

            /// <summary>
            /// Gets the skill levels the user originally wanted to add.
            /// </summary>
            public ReadOnlyCollection<ISkillLevel> SkillsToAdd
            {
                get { return m_skillsToAdd.AsReadOnly(); }
            }

            /// <summary>
            /// Gets all the entries to add when an addition is performed, including the prerequisites.
            /// </summary>
            public ReadOnlyCollection<PlanEntry> AllEntriesToAdd
            {
                get { return m_allEntriesToAdd.AsReadOnly(); }
            }

            /// <summary>
            /// Gets the skill levels the user originally wanted to remove.
            /// </summary>
            public ReadOnlyCollection<ISkillLevel> SkillsToRemove
            {
                get { return m_skillsToRemove.AsReadOnly(); }
            }

            /// <summary>
            /// Gets all the entries to remove when a suppression is performed, including the dependencies.
            /// </summary>
            public ReadOnlyCollection<PlanEntry> AllEntriesToRemove
            {
                get { return m_allEntriesToRemove.AsReadOnly(); }
            }

            /// <summary>
            /// Gets the entries that can be optionally removed when a suppression is performed.
            /// </summary>
            public ReadOnlyCollection<PlanEntry> RemovablePrerequisites
            {
                get { return m_removablePrerequisites.AsReadOnly(); }
            }

            /// <summary>
            /// Gets the highest possible priority (lowest possible number) for new entries when an addition is performed. 
            /// This limit is due to the prerequisites, since they cannot have a lower priority than the entries to add.
            /// </summary>
            public int HighestPriorityForAddition
            {
                get { return m_highestPriorityForAddition; }
            }

            /// <summary>
            /// Performs the operation in the simplest possible way, using default priority for insertions and not removing useless prerequisites for 
            /// suppressions (but still removing dependent entries !).
            /// </summary>
            public void Perform()
            {
                switch (m_type)
                {
                    case PlanOperations.Suppression:
                        PerformSuppression(false);
                        break;
                    case PlanOperations.Addition:
                        PerformAddition(PlanEntry.DefaultPriority);
                        break;
                    default:
                        return;
                }
            }

            /// <summary>
            /// Suppress the entries.
            /// </summary>
            /// <param name="removePrerequisites">When true, also remove the prerequisites that are not used anymore.</param>
            public void PerformSuppression(bool removePrerequisites)
            {
                // Checks this operation is an addition
                if (m_type == PlanOperations.Addition)
                    throw new InvalidOperationException("The represented operation is an addition.");

                // No entries ? Quit
                if (m_skillsToRemove.Count == 0)
                    return;


                using (m_plan.SuspendingEvents())
                {
                    // Remove the entries
                    foreach (PlanEntry entry in m_allEntriesToRemove)
                    {
                        var existingEntry = m_plan.GetEntry(entry.Skill, entry.Level);
                        if (existingEntry != null)
                            m_plan.RemoveCore(m_plan.IndexOf(existingEntry));
                    }

                    // Also remove the prerequisites if the caller requested it.
                    if (removePrerequisites)
                    {
                        foreach (PlanEntry entry in m_removablePrerequisites)
                        {
                            var existingEntry = m_plan.GetEntry(entry.Skill, entry.Level);
                            if (existingEntry != null)
                                m_plan.RemoveCore(m_plan.IndexOf(existingEntry));
                        }
                    }
                }
            }

            /// <summary>
            /// Adds the entries.
            /// </summary>
            /// <param name="priority">The desired priority for the new entries, it is automatically adjusted to match the <see cref="HighestPriorityForAddition"/> property.</param>
            public void PerformAddition(int priority)
            {
                // Checks this operation is an addition
                if (m_type == PlanOperations.Suppression)
                    throw new InvalidOperationException("The represented operation is a suppression.");

                // No entries ? Quit
                if (m_allEntriesToAdd.Count == 0)
                    return;

                // Fixes priority
                priority = Math.Max(priority, m_highestPriorityForAddition);

                using (m_plan.SuspendingEvents())
                {
                    foreach (PlanEntry entry in m_allEntriesToAdd)
                    {
                        var existingEntry = m_plan.GetEntry(entry.Skill, entry.Level);

                        // Are we updating an existing entry ? Then just change the note
                        if (existingEntry != null)
                        {
                            // If existing entry's notes is null, we replace it 
                            if (String.IsNullOrEmpty(existingEntry.Notes))
                            {
                                existingEntry.Notes = entry.Notes;
                            }
                            // Else, we concatenate the notes
                            else if (entry.Notes.Contains(entry.Notes))
                            {
                                existingEntry.Notes += (", " + entry.Notes);
                            }

                            // Update the priority
                            if (existingEntry.Priority > priority)
                                existingEntry.Priority = priority;
                        }
                        else
                        {
                            entry.Priority = priority;
                            m_plan.AddCore(entry.Clone(m_plan));
                        }
                    }
                }
            }
        }
        #endregion

        private string m_name;
        private bool m_isConnected;
        private int m_changedNotificationSuppressions;
        private PlanChange m_change;
        private PlanSorting m_sortingPreferences;
        private InvalidPlanEntry[] m_invalidEntries;

        #region Construction, importation, exportation
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="character"></param>
        public Plan(BaseCharacter character)
            : base(character)
        {
            m_sortingPreferences = new PlanSorting();
            m_invalidEntries = new InvalidPlanEntry[0];
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="character"></param>
        /// <param name="serial"></param>
        internal Plan(BaseCharacter character, SerializablePlan serial)
            : this(character)
        {
            Import(serial);
        }

        /// <summary>
        /// Imports data from a serialization object
        /// </summary>
        /// <param name="serial"></param>
        public void Import(SerializablePlan serial)
        {
            // Update name
            m_name = serial.Name;
            m_sortingPreferences = serial.SortingPreferences.Clone();

            // Update entries
            List<PlanEntry> entries = new List<PlanEntry>();
            List<InvalidPlanEntry> invalidEntries = new List<InvalidPlanEntry>();
            foreach (var serialEntry in serial.Entries)
            {
                PlanEntry entry = new PlanEntry(this, serialEntry);

                if (entry.Skill != null)
                {
                    entries.Add(entry);
                }
                // There are buggy entries in the plan
                else 
                {
                    var invalidEntry = new InvalidPlanEntry()
                    {
                        SkillName = serialEntry.SkillName,
                        PlannedLevel = serialEntry.Level
                    };

                    invalidEntries.Add(invalidEntry);
                }
            }

            RebuildPlanFrom(entries);
            FixPrerequisites();

            foreach (var serialInvalidEntry in serial.InvalidEntries)
            {
                var invalidEntry = new InvalidPlanEntry()
                {
                    SkillName = serialInvalidEntry.SkillName,
                    PlannedLevel = serialInvalidEntry.PlannedLevel,
                    Acknowledged = serialInvalidEntry.Acknowledged
                };

                invalidEntries.Add(invalidEntry);
            }

            m_invalidEntries = invalidEntries.ToArray();

            // Notify name change
            if (m_isConnected)
                EveClient.OnPlanNameChanged(this);
        }

        /// <summary>
        /// Generates a serialization object
        /// </summary>
        /// <returns></returns>
        public SerializablePlan Export()
        {
            // Create serialization object
            var character = (Character)m_character;
            var serial = new SerializablePlan { Name = m_name, Owner = character.Guid, SortingPreferences = m_sortingPreferences.Clone() };

            // Add entries
            foreach (var entry in m_items)
            {
                var serialEntry = new SerializablePlanEntry
                {
                    ID = entry.Skill.ID,
                    SkillName = entry.Skill.Name,
                    Level = entry.Level,
                    Type = entry.Type,
                    Notes = entry.Notes, 
                    Priority = entry.Priority
                };

                // Add groups
                foreach (var group in entry.PlanGroups)
                {
                    serialEntry.PlanGroups.Add(group);
                }

                // Remapping point
                if (entry.Remapping != null)
                    serialEntry.Remapping = entry.Remapping.Export();

                serial.Entries.Add(serialEntry);
            }

            foreach (var entry in m_invalidEntries)
            {
                var serialEntry = new SerializableInvalidPlanEntry
                {
                    SkillName = entry.SkillName,
                    PlannedLevel = entry.PlannedLevel,
                    Acknowledged = entry.Acknowledged
                };

                serial.InvalidEntries.Add(serialEntry);
            }

            return serial;
        }
        #endregion


        /// <summary>
        /// Gets or sets true if the plan is connected to a character and will send notifications. When false, it's just a computing helper.
        /// </summary>
        internal bool IsConnected
        {
            get { return m_isConnected; }
            set { m_isConnected = true; }
        }

        /// <summary>
        /// Gets or sets the plan's name
        /// </summary>
        public string Name
        {
            get { return m_name; }
            set 
            { 
                m_name = value;
                if (m_isConnected)
                    EveClient.OnPlanNameChanged(this);
            }
        }

        /// <summary>
        /// List of invalid entries in the plan
        /// </summary>
        public IEnumerable<InvalidPlanEntry> InvalidEntries
        {
            get
            {
                return m_invalidEntries.Where(x => !x.Acknowledged);
            }
        }

        /// <summary>
        /// Does the plan contain one or more invalid entries
        /// </summary>
        public bool ContainsInvalidEntries
        {
            get
            {
                return m_invalidEntries.Where(x => !x.Acknowledged).Count() != 0;
            }
        }

        /// <summary>
        /// Gets sorting preferences for this plan. Those are only preferences, it does not change the plan.
        /// </summary>
        public PlanSorting SortingPreferences
        {
            get { return m_sortingPreferences; }
        }

        #region Event firing and suppression
        /// <summary>
        /// Returns an <see cref="IDisposable"/> object which suspends events notification and will resume them once disposed.
        /// </summary>
        /// <remarks>Use the returned object in a <c>using</c> block to ensure the disposal of the object even when exceptions are thrown.</remarks>
        /// <returns></returns>
        public override IDisposable SuspendingEvents()
        {
            Interlocked.Increment(ref m_changedNotificationSuppressions);

            return new DisposableWithCallback(() =>
                {
                    if (Interlocked.Decrement(ref m_changedNotificationSuppressions) == 0 && m_change != PlanChange.None)
                        OnChanged(m_change);
                });
        }

        /// <summary>
        /// Notify changes happened in the entries
        /// </summary>
        internal override void OnChanged(PlanChange change)
        {
            // Updates and notifications have been suspended
            if (m_changedNotificationSuppressions > 0)
            {
                m_change |= change;
                return;
            }

            // Changes are about to be fired
            change |= m_change;
            m_change = PlanChange.None;

            // Add missing prerequisites
            if ((change & PlanChange.Prerequisites) != PlanChange.None)
                FixPrerequisites();

            // Notify changes
            if ((change & PlanChange.Notification) != PlanChange.None && m_isConnected)
                EveClient.OnPlanChanged(this);

        }
        #endregion


        #region Insertion and removal
        /// <summary>
        /// Set the planned level to the given one, lowering it if it's higher than the targetted one. 
        /// When the skill is not planned already, the prerequisites are automatically added.
        /// </summary>
        /// <param name="skill">The skill we want to plan</param>
        /// <param name="level">The level we want to train to</param>
        public void PlanTo(StaticSkill skill, int level)
        {
            PlanTo(skill, level, PlanEntry.DefaultPriority, skill.Name);
        }

        /// <summary>
        /// Set the planned level to the given one, lowering it if it's higher than the targetted one. 
        /// When the skill is not planned already, the prerequisites are automatically added.
        /// Note this method won't remove entries other entries depend of.
        /// </summary>
        /// <param name="skill">The skill we want to plan</param>
        /// <param name="level">The level we want to train to</param>
        /// <param name="noteForNewEntries">The reason we want to train this skill</param>
        public void PlanTo(StaticSkill skill, int level, int priority, string noteForNewEntries)
        {
            int plannedLevel = GetPlannedLevel(skill);
            if (level == plannedLevel)
                return;

            using (SuspendingEvents())
            {
                // We may either have to add or remove entries. First, we assume we have to add ones
                if (level > plannedLevel)
                {
                    var skillsToAdd = new List<StaticSkillLevel>();
                    skillsToAdd.Add(new StaticSkillLevel(skill, level));

                    // None added ? Then return
                    var operation = TryAddSet(skillsToAdd, noteForNewEntries);
                    operation.PerformAddition(priority);
                }
                else
                {
                    level = Math.Max(level, GetMinimumLevel(skill));

                    // If we reach this point, then we have to remove entries
                    for (int i = 5; i > level; i--)
                    {
                        PlanEntry entry = GetEntry(skill, i);
                        if (entry == null)
                            continue;

                        RemoveCore(m_items.IndexOf(entry));
                    }
                }
            }
        }

        /// <summary>
        /// Returns an operation to set the planned level to the given one, lowering it if it's higher than the targetted one. 
        /// The returned object allows an extended control, to automatically remove dependencies and prerequisites. 
        /// </summary>
        /// <param name="skill">The skill we want to plan</param>
        /// <param name="level">The level we want to train to</param>
        /// <returns></returns>
        public IPlanOperation TryPlanTo(Skill skill, int level)
        {
            return TryPlanTo(skill, level, skill.Name);
        }

        /// <summary>
        /// Returns an operation to set the planned level to the given one, lowering it if it's higher than the targetted one. 
        /// The returned object allows an extended control, to automatically remove dependencies and prerequisites. 
        /// </summary>
        /// <param name="skill">The skill we want to plan</param>
        /// <param name="level">The level we want to train to</param>
        /// <param name="noteForNewEntries">The reason we want to train this skill</param>
        /// <returns></returns>
        public IPlanOperation TryPlanTo(Skill skill, int level, string noteForNewEntries)
        {
            int plannedLevel = GetPlannedLevel(skill);
            if (level == plannedLevel)
                return new PlanOperation(this);

            // Addition
            if (level > plannedLevel)
            {
                // Get skill levels to add
                var skillsToAdd = new List<StaticSkillLevel>();
                for (int i = plannedLevel + 1; i <= level; i++)
                {
                    skillsToAdd.Add(new StaticSkillLevel(skill, level));
                }

                // Create the operation
                return TryAddSet(skillsToAdd, noteForNewEntries);
            }
            // Suppression
            else
            {
                // Get skill levels to remove
                var skillsToRemove = new List<StaticSkillLevel>();
                for (int i = plannedLevel; i > level && i > skill.Level; i--)
                {
                    skillsToRemove.Add(new StaticSkillLevel(skill, i));
                }

                return TryRemoveSet(skillsToRemove);
            }
        }

        /// <summary>
        /// Adds a set of skills to this plan
        /// </summary>
        /// <param name="skillsToAdd">The skill levels to add.</param>
        /// <param name="note">The note for the new entries.</param>
        /// <returns>An object allowing to perform and control the addition.</returns>
        public IPlanOperation TryAddSet<T>(IEnumerable<T> skillsToAdd, string note)
            where T : ISkillLevel
        {
            int lowestPrereqPriority;
            var allEntriesToAdd = GetAllEntriesToAdd(skillsToAdd, note, out lowestPrereqPriority);

            return new PlanOperation(this, skillsToAdd.Cast<ISkillLevel>(), allEntriesToAdd, lowestPrereqPriority);

        }

        /// <summary>
        /// Removes a set of skill levels from this plan.
        /// </summary>
        /// <param name="skillsToRemove">The skill levels to remove.</param>
        /// <returns>An object allowing to perform and control the removal.</returns>
        public IPlanOperation TryRemoveSet<T>(IEnumerable<T> skillsToRemove)
            where T : ISkillLevel
        {
            var allEntriesToRemove = GetAllEntriesToRemove(skillsToRemove);

            // Creates a plan where the entries and their dependencies have been removed
            var freePlan = new PlanScratchpad(m_character);
            freePlan.RebuildPlanFrom(m_items);
            foreach (var entry in allEntriesToRemove)
            {
                freePlan.Remove(entry);
            }

            // Gather removables prerequisites now useless
            var removablePrerequisites = new List<PlanEntry>();
            foreach (var entryToRemove in allEntriesToRemove)
            {
                foreach (var prereq in m_items)
                {
                    // Skip if this skill was not dependent of the prereq.
                    if (!entryToRemove.IsDependentOf(prereq))
                        continue;

                    // Skip if still required
                    if (freePlan.GetMinimumLevel(prereq.Skill) != 0)
                        continue;

                    // Add the entry if its type is Prerequisite
                    var prereqEntry = freePlan.GetEntry(prereq.Skill, prereq.Level);
                    if (prereqEntry == null)
                        continue;

                    // Adds it as a removable prerequisite
                    if (prereqEntry.Type == PlanEntryType.Prerequisite)
                    {
                        removablePrerequisites.Add(prereqEntry);
                        freePlan.Remove(prereqEntry);
                    }
                }
            }

            // Create the operation
            return new PlanOperation(this, skillsToRemove.Cast<ISkillLevel>(), allEntriesToRemove, removablePrerequisites);
        }
        #endregion


        #region Certificates
        /// <summary>
        /// Adds the provided certificate's prerequisites to the plan
        /// </summary>
        /// <param name="certificate"></param>
        /// <param name="confirmationCallback">A callback to request the user a confirmation</param>
        public IPlanOperation TryPlanTo(StaticCertificate certificate)
        {
            return TryAddSet(certificate.AllTopPrerequisiteSkills, certificate.ToString());
        }
        #endregion


        #region Priorities changes
        /// <summary>
        /// Try to set the priority of the given entries and cancel if a conflict arises.
        /// </summary>
        /// <param name="entries">The list of entries to change priority of</param>
        /// <param name="priority">The new priority to set</param>
        /// <returns>True when successful, false when a conflict arised.</returns>
        public bool TrySetPriority(PlanScratchpad m_displayPlan, IEnumerable<PlanEntry> entries, int priority)
        {
            // Change priorities and make a backup
            Queue<int> oldPriorities = new Queue<int>();
            foreach (var entry in entries)
            {
                oldPriorities.Enqueue(entry.Priority);
                entry.Priority = priority;
            }
            
            // We are rebuilding the plan with the new priorities in order to check them
            RebuildPlanFrom(m_displayPlan, true);

            // Check priorities
            if (FixPrioritiesOrder(false, false))
            {
                // Priorities are OK we save them and return
                OnChanged(PlanChange.Notification);
                return true;
            }

            // Failure, restore the priorities
            foreach (var entry in entries)
            {
                entry.Priority = oldPriorities.Dequeue();
            }

            // We are rebuilding the plan from the old priorities
            RebuildPlanFrom(m_displayPlan, true);
            
            return false;
        }
        #endregion


        /// <summary>
        /// Creates a clone for another character
        /// </summary>
        /// <param name="character"></param>
        /// <returns></returns>
        public Plan Clone(BaseCharacter character)
        {
            Plan plan = new Plan(character);
            plan.m_name = m_name;
            plan.RebuildPlanFrom(this);
            return plan;
        }

        /// <summary>
        /// Creates a clone
        /// </summary>
        /// <returns></returns>
        public Plan Clone()
        {
            return Clone(m_character);
        }

        /// <summary>
        /// Marks all Invalid Entries in the plan as Acknowledged
        /// </summary>
        public void AcknoledgeInvalidEntries()
        {
            foreach (var entry in m_invalidEntries)
            {
                entry.Acknowledged = true;
            }
        }

        /// <summary>
        /// Removes all Invalid Entries that have yet to be Acknowledged
        /// </summary>
        public void ClearInvalidEntries()
        {
            m_invalidEntries = m_invalidEntries.Where(x => x.Acknowledged == true).ToArray();
        }

        /// <summary>
        /// Merges the characters skills with the plan entries
        /// </summary>
        public SerializableCharacterSkill Merge(SerializableCharacterSkill skill)
        {
            foreach (PlanEntry entry in m_items)
            {
                if (entry.Skill.ID == skill.ID)
                {
                    skill.Level = entry.Level;
                    skill.Skillpoints = entry.Skill.GetPointsRequiredForLevel(entry.Level);
                }
            }
            return skill;
        }
    }
}
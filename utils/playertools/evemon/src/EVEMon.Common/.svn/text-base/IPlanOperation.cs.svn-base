using System;
using System.Collections.ObjectModel;
namespace EVEMon.Common
{
    public interface IPlanOperation
    {
        /// <summary>
        /// Gets the plan affected by this operation.
        /// </summary>
        Plan Plan { get; }
        /// <summary>
        /// Gets the type of operation to perform
        /// </summary>
        PlanOperations Type { get; }
        /// <summary>
        /// Gets the highest possible priority (lowest possible number) for new entries when an addition is performed. 
        /// This limit is due to the prerequisites, since they cannot have a lower priority than the entries to add.
        /// </summary>
        int HighestPriorityForAddition { get; }

        /// <summary>
        /// Gets all the entries to add when an addition is performed, including the prerequisites.
        /// </summary>
        ReadOnlyCollection<PlanEntry> AllEntriesToAdd { get; }
        /// <summary>
        /// Gets the skill levels the user originally wanted to add.
        /// </summary>
        ReadOnlyCollection<ISkillLevel> SkillsToAdd { get; }

        /// <summary>
        /// Gets all the entries to remove when a suppression is performed, including the dependencies.
        /// </summary>
        ReadOnlyCollection<PlanEntry> AllEntriesToRemove { get; }
        /// <summary>
        /// Gets the entries that can be optionally removed when a suppression is performed.
        /// </summary>
        ReadOnlyCollection<PlanEntry> RemovablePrerequisites { get; }
        /// <summary>
        /// Gets the skill levels the user originally wanted to remove.
        /// </summary>
        ReadOnlyCollection<ISkillLevel> SkillsToRemove { get; }

        /// <summary>
        /// Performs the operation in the simplest possible way, using default priority for insertions and not removing useless prerequisites for 
        /// suppressions (but still removing dependent entries !).
        /// </summary>
        void Perform();

        /// <summary>
        /// Adds the entries.
        /// </summary>
        /// <param name="priority">The priority of the new entries.</param>
        void PerformAddition(int priority);

        /// <summary>
        /// Suppress the entries.
        /// </summary>
        /// <param name="removePrerequisites">When true, also remove the prerequisites that are not used anymore.</param>
        void PerformSuppression(bool removePrerequisites);

    }
}

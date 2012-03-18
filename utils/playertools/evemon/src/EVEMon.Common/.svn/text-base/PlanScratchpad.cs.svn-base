using System;
using System.Collections.Generic;
using System.Linq;
using EVEMon.Common.Attributes;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a plan made for computation, without the automatic priorities fixing and prerequisites checks. 
    /// It therefore provides different methods from <see cref="Plan"/> for insertions and removals, to allow usage of this class as a regular collection.
    /// It also does not fire events.
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class PlanScratchpad : BasePlan
    {
        #region Disposable
        /// <summary>
        /// A stub disposable object.
        /// </summary>
        private sealed class Disposable : IDisposable
        {
            public void Dispose()
            {
            }
        }
        #endregion

        private static readonly Disposable m_disposable = new Disposable();

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="character"></param>
        public PlanScratchpad(BaseCharacter character)
            : base(character)
        {
        }

        /// <summary>
        /// Constructor from an enumeration.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="entries"></param>
        public PlanScratchpad(BaseCharacter character, IEnumerable<PlanEntry> entries)
            : this(character)
        {
            AddRange(entries);
        }

        #region Event firing and suppression
        /// <summary>
        /// Returns an <see cref="IDisposable"/> object which suspends events notification and will resume them once disposed.
        /// </summary>
        /// <remarks>Use the returned object in a <c>using</c> block to ensure the disposal of the object even when exceptions are thrown.</remarks>
        /// <returns></returns>
        public override IDisposable SuspendingEvents()
        {
            return m_disposable;
        }

        /// <summary>
        /// Notify changes happened in the entries
        /// </summary>
        internal override void OnChanged(PlanChange change)
        {
        }
        #endregion


        #region Insertions and removals
        /// <summary>
        /// Adds an item.
        /// </summary>
        /// <param name="entry"></param>
        public void Add(PlanEntry entry)
        {
            AddCore(entry);
        }

        /// <summary>
        /// Adds the provided items.
        /// </summary>
        /// <param name="entry"></param>
        public void AddRange(IEnumerable<PlanEntry> entries)
        {
            foreach(var entry in entries) AddCore(entry);
        }

        /// <summary>
        /// Removes an item.
        /// </summary>
        /// <param name="entry"></param>
        public void Remove(PlanEntry entry)
        {
            int index = IndexOf(entry.Skill, entry.Level);
            if (index != -1) RemoveCore(index);
        }
        #endregion


        #region Simple sort
        /// <summary>
        /// Performs a simple ordering by the given sort criteria, based on the latest statistics.
        /// </summary>
        /// <param name="sort"></param>
        /// <param name="reverseOrder"></param>
        internal void SimpleSort(PlanEntrySort sort, bool reverseOrder)
        {
            // Apply simple sort operators
            switch (sort)
            {
                case PlanEntrySort.Name:
                    m_items.StableSort(PlanSorter.CompareByName);
                    break;
                case PlanEntrySort.Cost:
                    m_items.StableSort(PlanSorter.CompareByCost);
                    break;
                case PlanEntrySort.PrimaryAttribute:
                    m_items.StableSort(PlanSorter.CompareByPrimaryAttribute);
                    break;
                case PlanEntrySort.SecondaryAttribute:
                    m_items.StableSort(PlanSorter.CompareBySecondaryAttribute);
                    break;
                case PlanEntrySort.Priority:
                    m_items.StableSort(PlanSorter.CompareByPriority);
                    break;
                case PlanEntrySort.PlanGroup:
                    m_items.StableSort(PlanSorter.CompareByPlanGroup);
                    break;
                case PlanEntrySort.PercentCompleted:
                    m_items.StableSort(PlanSorter.CompareByPercentCompleted);
                    break;
                case PlanEntrySort.Rank:
                    m_items.StableSort(PlanSorter.CompareByRank);
                    break;
                case PlanEntrySort.Notes:
                    m_items.StableSort(PlanSorter.CompareByNotes);
                    break;
                case PlanEntrySort.PlanType:
                    m_items.StableSort(PlanSorter.CompareByPlanType);
                    break;
                case PlanEntrySort.TimeDifference:
                    m_items.StableSort(PlanSorter.CompareByTimeDifference);
                    break;
                case PlanEntrySort.TrainingTime:
                    m_items.StableSort(PlanSorter.CompareByTrainingTime);
                    break;
                case PlanEntrySort.TrainingTimeNatural:
                    m_items.StableSort(PlanSorter.CompareByTrainingTimeNatural);
                    break;
                case PlanEntrySort.SkillGroupDuration:
                    var skillGroupsDurations = new Dictionary<StaticSkillGroup, TimeSpan>();
                    m_items.StableSort((x, y) => PlanSorter.CompareBySkillGroupDuration(x, y, m_items, skillGroupsDurations));
                    break;
                case PlanEntrySort.SPPerHour:
                    m_items.StableSort(PlanSorter.CompareBySPPerHour);
                    break;
                case PlanEntrySort.SkillPointsRequired:
                    m_items.StableSort(PlanSorter.CompareBySkillPointsRequired);
                    break;
                default:
                    break;
            }

            // Reverse order
            if (reverseOrder) m_items.Reverse();
        }
        #endregion
    }
}

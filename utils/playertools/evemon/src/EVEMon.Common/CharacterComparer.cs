using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a customizable characters comparer
    /// </summary>
    public struct CharacterComparer : IComparer<Character>
    {
        private CharacterSortCriteria m_criteria;
        private SortOrder m_order;

        #region Constructors
        /// <summary>
        /// Constructor for an ascending sort along the given criteria.
        /// </summary>
        public CharacterComparer(CharacterSortCriteria criteria)
        {
            m_criteria = criteria;
            m_order = SortOrder.Ascending;
        }

        /// <summary>
        /// Constructor with custom parameters.
        /// </summary>
        public CharacterComparer(CharacterSortCriteria criteria, SortOrder order)
        {
            m_criteria = criteria;
            m_order = order;
        }
        
        /// <summary>
        /// Constructor from a tray popup setting
        /// </summary>
        /// <param name="criteria"></param>
        public CharacterComparer(TrayPopupSort criteria)
        {
            switch (criteria)
            {
                case TrayPopupSort.NameASC:
                    m_criteria = CharacterSortCriteria.Name;
                    m_order = SortOrder.Ascending;
                    break;
                case TrayPopupSort.NameDESC:
                    m_criteria = CharacterSortCriteria.Name;
                    m_order = SortOrder.Descending;
                    break;
                case TrayPopupSort.TrainingCompletionTimeASC:
                    m_criteria = CharacterSortCriteria.TrainingCompletion;
                    m_order = SortOrder.Ascending;
                    break;
                case TrayPopupSort.TrainingCompletionTimeDESC:
                    m_criteria = CharacterSortCriteria.TrainingCompletion;
                    m_order = SortOrder.Descending;
                    break;
                default:
                    throw new NotImplementedException();
            }
        }
        #endregion


        /// <summary>
        /// Gets or sets the sort order
        /// </summary>
        public SortOrder Order
        {
            get { return m_order; }
            set { m_order = value; }
        }

        /// <summary>
        /// Gets or sets the sort criteria
        /// </summary>
        public CharacterSortCriteria Criteria
        {
            get { return m_criteria; }
            set { m_criteria = value; }
        }

        /// <summary>
        /// Performs the comparison
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns></returns>
        public int Compare(Character x, Character y)
        {
            // Exchange items when descending sort
            if (m_order == SortOrder.Descending)
            {
                var temp = x;
                x = y;
                y = temp;
            }

            // Deal with the criteria
            switch (m_criteria)
            {
                case CharacterSortCriteria.TrainingCompletion:
                    return CompareByCompletionTime(x, y);

                case CharacterSortCriteria.Name:
                    return CompareByName(x, y);

                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Compares the two characters by their training completion time or, when not in training their names
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        public static int CompareByCompletionTime(Character x, Character y)
        {
            // Get their training skills
            var skillX = x.CurrentlyTrainingSkill;
            var skillY = y.CurrentlyTrainingSkill;
            if (skillX == null && skillY == null) return String.Compare(x.Name, y.Name);
            if (skillX == null) return -1;
            if (skillY == null) return +1;

            // Compare end time
            return DateTime.Compare(skillX.EndTime, skillY.EndTime);
        }

        /// <summary>
        /// Compare the given characters by their names
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        public static int CompareByName(Character x, Character y)
        {
            return String.Compare(x.Name, y.Name);
        }
    }

    public enum SortOrder 
    { 
        Ascending = 0, 
        Descending = 1 
    };

    public enum CharacterSortCriteria
    {
        /// <summary>
        /// Characters are sorted by their names
        /// </summary>
        Name = 0,
        /// <summary>
        /// Characters are sorted by their training completion time or, when not in training, their names.
        /// </summary>
        TrainingCompletion = 1,
    };
}

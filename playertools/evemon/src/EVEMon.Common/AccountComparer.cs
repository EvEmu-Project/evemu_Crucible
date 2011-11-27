using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common
{
    public sealed class AccountComparer : IComparer<Account>
    {
        private AccountSortCriteria m_criteria;
        private SortOrder m_order;

        #region Constructors
        /// <summary>
        /// Constructor for an ascending sort along the given criteria.
        /// </summary>
        public AccountComparer(AccountSortCriteria criteria)
        {
            m_criteria = criteria;
            m_order = SortOrder.Ascending;
        }

        /// <summary>
        /// Constructor with custom parameters.
        /// </summary>
        public AccountComparer(AccountSortCriteria criteria, SortOrder order)
        {
            m_criteria = criteria;
            m_order = order;
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
        public AccountSortCriteria Criteria
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
        public int Compare(Account x, Account y)
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
                case AccountSortCriteria.TrainingCompletion:
                    return CompareByCompletionTime(x, y);

                case AccountSortCriteria.ID:
                    return CompareByID(x, y);

                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Compares the two characters by their training completion time or, when not in training their names
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        public static int CompareByCompletionTime(Account x, Account y)
        {
            // Get CCP characters
            CCPCharacter cx = x.TrainingCharacter;
            CCPCharacter cy = y.TrainingCharacter;
            if (cx == null && cy == null) return CompareByID(x, y);
            if (cx == null) return -1;
            if (cy == null) return +1;

            // Get their training skills
            var skillX = cx.CurrentlyTrainingSkill;
            var skillY = cy.CurrentlyTrainingSkill;
            if (skillX == null && skillY == null) return CompareByID(x, y);
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
        public static int CompareByID(Account x, Account y)
        {
            if (x.UserID > y.UserID) return +1;
            if (x.UserID < y.UserID) return -1;
            return 0;
        }
    }



    public enum AccountSortCriteria
    {
        /// <summary>
        /// Accounts are sorted by their ids
        /// </summary>
        ID = 0,
        /// <summary>
        /// Accounts are sorted by their training completion time or, when not in training, their ids.
        /// </summary>
        TrainingCompletion = 1,
    };

}

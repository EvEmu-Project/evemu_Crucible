using System.Collections.Generic;

using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Performs a comparison between two <see cref="EveNotification"/> types.
    /// </summary>
    public sealed class EveNotificationsComparer : Comparer<EveNotification>
    {
        private EveNotificationsColumn m_column;
        private bool m_isAscending;

        /// <summary>
        /// Initializes a new instance of the <see cref="EveNotificationsComparer"/> class.
        /// </summary>
        /// <param name="column">The industry job column.</param>
        /// <param name="isAscending">Is ascending flag.</param>
        public EveNotificationsComparer(EveNotificationsColumn column, bool isAscending)
        {
            m_column = column;
            m_isAscending = isAscending;
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="EveNotification" /> type and returns a value
        /// indicating whether one object is less than, equal to, or greater than the other.
        /// </summary>
        /// <param name="x">The first object to compare.</param>
        /// <param name="y">The second object to compare.</param>
        /// <returns>
        /// Less than zero
        /// <paramref name="x"/> is less than <paramref name="y"/>.
        /// Zero
        /// <paramref name="x"/> equals <paramref name="y"/>.
        /// Greater than zero
        /// <paramref name="x"/> is greater than <paramref name="y"/>.
        /// </returns>
        public override int Compare(EveNotification x, EveNotification y)
        {
            if (m_isAscending)
                return CompareCore(x, y);

            return -CompareCore(x, y);
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="EveNotification"/> type and returns a value
        /// indicating whether one object is less than, equal to, or greater than the other.
        /// </summary>
        /// <param name="x">The first object to compare.</param>
        /// <param name="y">The second object to compare.</param>
        /// <returns>
        /// Less than zero
        /// <paramref name="x"/> is less than <paramref name="y"/>.
        /// Zero
        /// <paramref name="x"/> equals <paramref name="y"/>.
        /// Greater than zero
        /// <paramref name="x"/> is greater than <paramref name="y"/>.
        /// </returns>
        private int CompareCore(EveNotification x, EveNotification y)
        {
            switch (m_column)
            {
                case EveNotificationsColumn.SenderName:
                    return x.Sender.CompareTo(y.Sender);

                case EveNotificationsColumn.Type:
                    return x.Type.CompareTo(y.Type);

                case EveNotificationsColumn.SentDate:
                    return x.SentDate.CompareTo(y.SentDate);

                default:
                    return 0;
            }
        }
    }
}

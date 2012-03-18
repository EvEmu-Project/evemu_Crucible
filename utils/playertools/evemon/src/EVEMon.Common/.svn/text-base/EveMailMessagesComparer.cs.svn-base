using System.Collections.Generic;

using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    /// <summary>
    /// Performs a comparison between two <see cref="EveMailMessage"/> types.
    /// </summary>
    public sealed class EveMailMessagesComparer : Comparer<EveMailMessage>
    {
        private EveMailMessagesColumn m_column;
        private bool m_isAscending;

        /// <summary>
        /// Initializes a new instance of the <see cref="EveMailMessagesComparer"/> class.
        /// </summary>
        /// <param name="column">The industry job column.</param>
        /// <param name="isAscending">Is ascending flag.</param>
        public EveMailMessagesComparer(EveMailMessagesColumn column, bool isAscending)
        {
            m_column = column;
            m_isAscending = isAscending;
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="EveMailMessage" /> type and returns a value
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
        public override int Compare(EveMailMessage x, EveMailMessage y)
        {
            if (m_isAscending)
                return CompareCore(x, y);

            return -CompareCore(x, y);
        }

        /// <summary>
        /// Performs a comparison of two objects of the <see cref="EveMailMessage" /> type and returns a value
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
        private int CompareCore(EveMailMessage x, EveMailMessage y)
        {
            switch (m_column)
            {
                case EveMailMessagesColumn.SenderName:
                    return x.Sender.CompareTo(y.Sender);

                case EveMailMessagesColumn.Title:
                    return x.Title.CompareTo(y.Title);

                case EveMailMessagesColumn.SentDate:
                    return x.SentDate.CompareTo(y.SentDate);

                case EveMailMessagesColumn.ToCharacters:
                    return x.ToCharacters[0].CompareTo(y.ToCharacters[0]);

                case EveMailMessagesColumn.ToCorpOrAlliance:
                    return x.ToCorpOrAlliance.CompareTo(y.ToCorpOrAlliance);

                case EveMailMessagesColumn.ToMailingList:
                    return x.ToMailingLists[0].CompareTo(y.ToMailingLists[0]);
                
                default:
                    return 0;
            }
        }
    }
}

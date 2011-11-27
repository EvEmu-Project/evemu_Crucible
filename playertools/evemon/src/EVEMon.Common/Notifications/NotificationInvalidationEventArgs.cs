using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Notifications
{
    /// <summary>
    /// A class for the arguments of notifications invalidation.
    /// </summary>
    public sealed class NotificationInvalidationEventArgs : EventArgs
    {
        private long m_key;

        /// <summary>
        /// Constructor with a key identifying a sender/category pair.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="category"></param>
        public NotificationInvalidationEventArgs(Object sender, NotificationCategory category)
        {
            m_key = Notification.GetKey(sender, category);
        }

        /// <summary>
        /// Constructor with a key gotten from a notification.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="category"></param>
        public NotificationInvalidationEventArgs(Notification notification)
        {
            m_key = notification.InvalidationKey;
        }

        /// <summary>
        /// Gets a key identifying the notifications to invalidate.
        /// </summary>
        public long Key
        {
            get { return m_key; }
        }
    }
}

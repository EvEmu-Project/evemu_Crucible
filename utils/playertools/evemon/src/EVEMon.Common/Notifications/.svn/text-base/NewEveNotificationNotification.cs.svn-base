using System;

namespace EVEMon.Common.Notifications
{
    public sealed class NewEveNotificationNotification : Notification
    {
        public NewEveNotificationNotification(Object sender, int newNotifications)
            : base(NotificationCategory.NewEveNotification, sender)
        {
            NewNotificationsCount = newNotifications;
            UpdateDescription();
        }

        /// <summary>
        /// Gets or sets the new notifications count.
        /// </summary>
        /// <value>The new notifications count.</value>
        public int NewNotificationsCount
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets true if the notification has details.
        /// </summary>
        public override bool HasDetails
        {
            get { return false; }
        }

        /// <summary>
        /// Adds the number of new mail messages from the given notification to this notification.
        /// </summary>
        /// <param name="other"></param>
        public override void Append(Notification other)
        {
            var notifications = ((NewEveNotificationNotification)other).NewNotificationsCount;
            NewNotificationsCount += notifications;
            UpdateDescription();
        }

        /// <summary>
        /// Updates the description.
        /// </summary>
        private void UpdateDescription()
        {
            m_description = String.Format(CultureConstants.DefaultCulture, "{0} new EVE notification{1}.",
                                            NewNotificationsCount, (NewNotificationsCount > 1 ? "s" : String.Empty));
        }
    }
}

using System;

namespace EVEMon.Common.Notifications
{
    public sealed class NewEveMailMessageNotification : Notification
    {
        public NewEveMailMessageNotification(Object sender, int newMessages)
            : base(NotificationCategory.NewEveMailMessage, sender)
        {
            NewMailMessagesCount = newMessages;
            UpdateDescription();
        }

        /// <summary>
        /// Gets or sets the new mail messages count.
        /// </summary>
        /// <value>The new mail messages count.</value>
        public int NewMailMessagesCount
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
            var mailMessages = ((NewEveMailMessageNotification)other).NewMailMessagesCount;
            NewMailMessagesCount += mailMessages;
            UpdateDescription();
        }

        /// <summary>
        /// Updates the description.
        /// </summary>
        private void UpdateDescription()
        {
            m_description = String.Format(CultureConstants.DefaultCulture, "{0} new EVE mail message{1}.",
                                            NewMailMessagesCount, (NewMailMessagesCount > 1 ? "s" : String.Empty));
        }
    }
}

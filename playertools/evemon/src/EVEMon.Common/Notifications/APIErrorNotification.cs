using System;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common.Notifications
{
    public sealed class APIErrorNotification : Notification
    {
        private readonly IAPIResult m_result;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="category"></param>
        /// <param name="sender"></param>
        public APIErrorNotification(Object sender, IAPIResult result)
            : base(NotificationCategory.QueryingError, sender)
        {
            m_result = result;
        }

        /// <summary>
        /// Gets the associated API result.
        /// </summary>
        public IAPIResult Result
        {
            get { return m_result; }
        }

        /// <summary>
        /// Gets true if the notification has details.
        /// </summary>
        public override bool HasDetails
        {
            get { return true; }
        }
    }
}

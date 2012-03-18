using System;
using System.Collections.Generic;

namespace EVEMon.Common.Notifications
{
    public sealed class MarketOrdersNotification : Notification
    {
        private readonly List<MarketOrder> m_orders;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="category"></param>
        /// <param name="sender"></param>
        public MarketOrdersNotification(Object sender, IEnumerable<MarketOrder> orders)
            : base(NotificationCategory.MarketOrdersEnding, sender)
        {
            m_orders = new List<MarketOrder>(orders);
            UpdateDescription();
        }

        /// <summary>
        /// Gets the associated API result.
        /// </summary>
        public IEnumerable<MarketOrder> Orders
        {
            get 
            {
                foreach (var order in m_orders)
                {
                    yield return order;
                }
            }
        }

        /// <summary>
        /// Gets true if the notification has details.
        /// </summary>
        public override bool HasDetails
        {
            get { return true; }
        }

        /// <summary>
        /// Enqueue the orders from the given notification at the end of this notification.
        /// </summary>
        /// <param name="other"></param>
        public override void Append(Notification other)
        {
            var orders = ((MarketOrdersNotification)other).m_orders;
            m_orders.AddRange(orders);
            UpdateDescription();
        }

        /// <summary>
        /// Updates the description.
        /// </summary>
        private void UpdateDescription()
        {
            m_description = String.Format(CultureConstants.DefaultCulture, "{0} market order{1} expired or fulfilled.", m_orders.Count, (m_orders.Count > 1 ? "s" : String.Empty));
        }
    }
}

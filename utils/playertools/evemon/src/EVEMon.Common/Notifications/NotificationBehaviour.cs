using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Notifications
{
    /// <summary>
    /// How the notification interacts with the ones which share the same <see cref="NotificationBase.InvalidationKey"/>.
    /// </summary>
    public enum NotificationBehaviour
    {
        /// <summary>
        /// All the notifications with the same invalidation key will cohabit together as distinct notifications.
        /// </summary>
        Cohabitate,
        /// <summary>
        /// Replaces all the previous notifications with the same invalidation key.
        /// </summary>
        Overwrite,
        /// <summary>
        /// All the notifications with this invalidation key will be merged with this one, through their details merging.
        /// </summary>
        Merge
    }
}

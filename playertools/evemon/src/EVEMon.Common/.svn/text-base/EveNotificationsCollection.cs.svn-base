using System;
using System.Collections.Generic;
using System.Linq;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    public sealed class EveNotificationsCollection : ReadonlyCollection<EveNotification>
    {
        private readonly CCPCharacter m_ccpCharacter;

        #region Constructor

        /// <summary>
        /// Internal constructor.
        /// </summary>
        internal EveNotificationsCollection(CCPCharacter ccpCharacter)
        {
            m_ccpCharacter = ccpCharacter;

        }

        #endregion


        #region Properties

        /// <summary>
        /// Gets or sets the number of new notifications.
        /// </summary>
        /// <value>The new notifications.</value>
        public int NewNotifications { get; set; }

        #endregion


        #region Importation & Exportation

        /// <summary>
        /// Imports the eve notifications IDs from a serializable object.
        /// </summary>
        /// <param name="eveMailMessagesIDs">The eve mail messages IDs.</param>
        internal void Import(string eveNotificationsIDs)
        {
            if (String.IsNullOrEmpty(eveNotificationsIDs))
                return;

            List<string> ids = eveNotificationsIDs.Split(',').ToList();
            foreach (var id in ids)
            {
                long ID = long.Parse(id);
                m_items.Add(new EveNotification(m_ccpCharacter,
                                                new SerializableNotificationsListItem()
                                                {
                                                    NotificationID = ID
                                                }));
            }
        }

        /// <summary>
        /// Imports an enumeration of API objects.
        /// </summary>
        /// <param name="src">The enumeration of serializable notifications from the API.</param>
        internal void Import(List<SerializableNotificationsListItem> src)
        {
            NewNotifications = 0;

            List<EveNotification> newNotifications = new List<EveNotification>();

            // Import the notifications from the API
            foreach (var srcEVENotification in src)
            {
                // If it's a new notification increase the counter
                var notification = m_items.FirstOrDefault(x => x.NotificationID == srcEVENotification.NotificationID);
                if (notification == null)
                    NewNotifications++;

                newNotifications.Add(new EveNotification(m_ccpCharacter, srcEVENotification));
            }

            m_items.Clear();
            m_items.AddRange(newNotifications);
        }

        /// <summary>
        /// Exports the eve notifications IDs to a serializable object.
        /// </summary>
        /// <returns></returns>
        internal string Export()
        {
            List<string> serial = new List<string>();

            foreach (var notification in m_items)
            {
                serial.Add(notification.NotificationID.ToString());
            }

            return string.Join(",", serial);
        }

        #endregion
    }
}

using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' eve notifications. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPINotifications
    {
        [XmlArray("notifications")]
        [XmlArrayItem("notification")]
        public List<SerializableNotificationsListItem> Notifications
        {
            get;
            set;
        }
    }
}

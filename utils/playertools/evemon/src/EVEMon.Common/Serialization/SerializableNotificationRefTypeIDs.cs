using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization
{
    [XmlRoot("NotificationRefTypeIDs")]
    public sealed class SerializableNotificationRefTypeIDs
    {
        public SerializableNotificationRefTypeIDs()
        {
            Types = new List<SerializableNotificationRefTypeIDsListItem>();
        }

        [XmlArray("refTypes")]
        [XmlArrayItem("refType")]
        public List<SerializableNotificationRefTypeIDsListItem> Types
        {
            get;
            set;
        }
    }
}

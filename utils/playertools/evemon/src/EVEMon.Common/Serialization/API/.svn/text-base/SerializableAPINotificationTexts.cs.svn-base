using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    public sealed class SerializableAPINotificationTexts
    {
        [XmlArray("notifications")]
        [XmlArrayItem("notification")]
        public List<SerializableNotificationTextsListItem> Texts
        {
            get;
            set;
        }

        [XmlElement("missingIDs")]
        public string MissingMessageIDs
        {
            get;
            set;
        }
    }
}

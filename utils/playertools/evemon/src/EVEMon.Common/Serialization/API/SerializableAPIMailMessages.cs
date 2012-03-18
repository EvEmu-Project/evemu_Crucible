using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' eve mail messages headers. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIMailMessages
    {
        [XmlArray("messages")]
        [XmlArrayItem("message")]
        public List<SerializableMailMessagesListItem> Messages
        {
            get;
            set;
        }
    }
}

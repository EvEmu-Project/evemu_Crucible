using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' eve mail messages bodies. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIMailBodies
    {
        [XmlArray("messages")]
        [XmlArrayItem("message")]
        public List<SerializableMailBodiesListItem> Bodies
        {
            get;
            set;
        }

        [XmlElement("missingMessageIDs")]
        public string MissingMessageIDs
        {
            get;
            set;
        }
    }
}

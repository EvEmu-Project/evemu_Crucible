using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of the conquerable stations list. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIConquerableStationList
    {
        [XmlArray("outposts")]
        [XmlArrayItem("outpost")]
        public List<SerializableOutpost> Outposts
        {
            get;
            set;
        }
    }
}
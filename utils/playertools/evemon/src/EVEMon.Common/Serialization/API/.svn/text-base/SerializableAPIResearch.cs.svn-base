using System.Xml.Serialization;
using System.Collections.Generic;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' research points. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIResearch
    {
        [XmlArray("research")]
        [XmlArrayItem("points")]
        public List<SerializableResearchListItem> ResearchPoints
        {
            get;
            set;
        }
    }
}

using System.Collections.Generic;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a characters' industry jobs. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIIndustryJobs
    {
        [XmlArray("jobs")]
        [XmlArrayItem("job")]
        public List<SerializableJobListItem> Jobs
        {
            get;
            set;
        }
    }
}

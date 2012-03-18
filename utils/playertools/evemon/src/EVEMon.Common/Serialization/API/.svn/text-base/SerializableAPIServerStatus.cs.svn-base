using System;
using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.API
{
    /// <summary>
    /// Represents a serializable version of a server status. Used for querying CCP.
    /// </summary>
    public sealed class SerializableAPIServerStatus
    {
        [XmlElement("serverOpen")]
        public string CCPOpen
        {
            get;
            set;
        }

        [XmlIgnore]
        public bool Open
        {
            get { return Boolean.Parse(CCPOpen.ToLower(CultureConstants.DefaultCulture)); }
            set { CCPOpen = (value ? "True" : "False"); }
        }

        [XmlElement("onlinePlayers")]
        public int Players
        {
            get;
            set;
        }

    }
}
